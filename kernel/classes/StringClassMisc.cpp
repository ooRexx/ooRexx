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
/* REXX Kernel                                                  okbmisc.c     */
/*                                                                            */
/* Miscellaneous REXX string methods                                          */
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

PCHAR Memcpbrk(
  PCHAR    String,                     /* search string                     */
  PCHAR    Set,                        /* reference set                     */
  size_t   Length )                    /* size of string                    */
/*********************************************************************/
/*  Function:  Find first occurence of set nonmember in block        */
/*********************************************************************/
{
  PCHAR    Retval;                     /* returned value                    */

  Retval = NULL;                       /* nothing found yet                 */
  while (Length--) {                   /* search through string             */
                                       /* find a match in ref set?          */
    if (*String == '\0' || !strchr(Set, *String)) {
      Retval = String;                 /* copy position                     */
      break;                           /* quit the loop                     */
    }
    String++;                          /* step the pointer                  */
  }
  return Retval;                       /* return matched position           */
}

INT ValSet(
  PCHAR    String,                     /* string to validate                */
  size_t    Length,                    /* string length                     */
  PCHAR     Set,                       /* character set                     */
  INT       Modulus,                   /* smallest group size               */
  size_t   *PackedSize )               /* total packed size                 */
/*********************************************************************/
/*  Function:               Validate blocks in string                */
/*                                                                   */
/*                          A string is considered valid if consists */
/*                          of zero or more characters belonging to  */
/*                          the null-terminated C string set in      */
/*                          groups of size modulus.  The first group */
/*                          may have fewer than modulus characters.  */
/*                          The groups are optionally separated by   */
/*                          one or more blanks.                      */
/*********************************************************************/
{
  UCHAR    c;                          /* current character                 */
  size_t   Count;                      /* # set members found               */
  PUCHAR   Current;                    /* current location                  */
  INT      SpaceFound;                 /* space found yet?                  */
  size_t   Residue;                    /* if space_found, # set members     */
  INT      rc;                         /* return code                       */

  rc = FALSE;                          /* default to failure                */
  if (*String != ' ' && *String != '\t') {    /* if no leading blank               */
    SpaceFound = 0;                    /* set initial space flag            */
    Count = 0;                         /* start count with zero             */
    Current = (PUCHAR)String;          /* point to start                    */

    rc = TRUE;                         /* default to good now               */
    for (; Length; Length--) {         /* process entire string             */
      c = *Current++;                  /* get char and step pointer         */
                                       /* if c in set                       */
      if (c != '\0' && strchr(Set, c) != NULL)
        Count++;                       /* bump count                        */
      else {
        if (c == ' ' || c == '\t') {   /* if c blank                        */
          if (!SpaceFound) {           /* if 1st blank                      */
                                       /* save position                     */
            Residue = (Count % Modulus);
            SpaceFound = 1;            /* we have the first space           */
          }
                                       /* else if bad position              */
          else if (Residue != (Count % Modulus)) {
            rc = FALSE;                /* this is an error                  */
            break;                     /* report error                      */
          }
        }
        else {
          rc = FALSE;                  /* this is an error                  */
          break;                       /* report error                      */
        }
      }
    }
    if (rc) {                          /* still good?                       */
      if (c == ' ' || c == '\t')       /* if trailing blank                 */
        rc = FALSE;                    /* report error                      */
      else if (SpaceFound && (Count % Modulus) != Residue)
        rc = FALSE;                    /* grouping problem                  */
      else
        *PackedSize = Count;           /* return count of chars             */
    }
  }
  return rc;                           /* return success/failure            */
}

int RexxString::isSymbol()
/*********************************************************************/
/*                                                                   */
/*   Function:         determines valid rexx symbols and returns     */
/*                     a type indicator for valid symbols that can   */
/*                     be passed on to the dictionary routines.      */
/*                                                                   */
/*********************************************************************/
{
  PUCHAR     Scan;                     /* string scan pointer               */
  size_t     Compound;                 /* count of periods                  */
  size_t     i;                        /* loop counter                      */
  PUCHAR     Linend;                   /* end of line                       */
  INT        Type;                     /* return type                       */

                                       /* name too long                     */
                                       /* or too short                      */
  if (this->length > MAX_SYMBOL_LENGTH || this->length == 0)
    return STRING_BAD_VARIABLE;        /* set a bad type                    */

                                       /* step to end                       */
  Linend = (PUCHAR)this->stringData + this->length;

  Compound = 0;                        /* set compound name is no           */
  Scan = (PUCHAR)this->stringData;     /* save start position               */
                                       /* while still part of symbol        */
  while (Scan < Linend && lookup[*Scan]) {

    if (*Scan == '.')                  /* a dot found..                     */
      Compound++;                      /* indicate a compound var           */

    Scan++;                            /* step the pointer                  */
  }                                    /* len of symbol                     */
                                       /* now check for exponent            */
  if (((Scan + 1) < Linend) &&
      (*Scan == '-' || *Scan == '+') &&
      (isdigit(this->stringData[0]) || *Scan == '.') &&
      (toupper(*(Scan - 1)) == 'E')) {
    Scan++;                            /* step to next                      */

    while (Scan < Linend) {            /* while more characters             */
      if (!isdigit(*Scan))             /* if not a digit                    */
        return STRING_BAD_VARIABLE;    /* this isn't valid                  */
      Scan++;                          /* step to next char                 */
    }
  }
  if (Scan < Linend)                   /* use the entire string?            */
    return STRING_BAD_VARIABLE;        /* no, can't be good                 */
                                       /* now determine symbol type         */
                                       /* possible number?                  */
  if (this->stringData[0] == '.' || isdigit(this->stringData[0])) {

                                       /* only a period?                    */
    if (Compound == 1 && this->length == 1)
      Type = STRING_LITERAL_DOT;       /* yes, set the token type           */
    else if (Compound > 1)             /* too many periods?                 */
      Type = STRING_LITERAL;           /* yes, just a literal token         */
    else {                             /* check for a real number           */
      Type = STRING_NUMERIC;           /* assume numeric for now            */
      Scan = (PUCHAR)this->stringData; /* point to symbol                   */
                                       /* scan symbol, validating           */
      for (i = this->length ; i; i-- ) {
        if (!isdigit(*Scan) &&         /* if not a digit and                */
            *Scan != '.')              /* and not a period...               */
          break;                       /* finished                          */
        Scan++;                        /* step to next character            */
      }
      if (i > 1 &&                     /* if tripped over an 'E'            */
          toupper(*Scan) == 'E') {     /* could be exponential              */
        Scan++;                        /* step past E                       */
        i--;                           /* count the character               */
                                       /* +/- case already validated        */
        if (*Scan != '+' && *Scan != '-') {
          for(; i; i--) {              /* scan rest of symbol               */
            if (!isdigit(*Scan)) {     /* if not a digit...                 */
              Type = STRING_LITERAL;   /* not a number                      */
              break;
            }
            Scan++;                    /* step to next character            */
          }
        }
      }
      else if (i)                      /* literal if stuff left             */
        Type = STRING_LITERAL;         /* yes, just a literal token         */
    }
  }

  else if (!Compound) {                /* not a compound so...              */
    Type = STRING_NAME;                /* set the token type                */
  }
                                       /* is it a stem?                     */
  else if (Compound == 1 && *(Scan - 1) == '.')
    Type = STRING_STEM;                /* yes, set the token type           */
  else
    Type = STRING_COMPOUND_NAME;       /* otherwise just plain              */
                                       /* compound                          */
  return Type;                         /* return the type info              */
}

RexxObject *DataType(
     RexxString *String,               /* string value                      */
     UCHAR       Option )              /* requested option                  */
/*********************************************************************/
/* Function:  Perform primitive datatype validation                  */
/*********************************************************************/
{
  size_t      Len;                     /* validated string length           */
  RexxObject *Answer;                  /* validation result                 */
  RexxObject *Temp;                    /* temporary value                   */
  PCHAR       Scanp;                   /* string data pointer               */
  size_t      Count;                   /* hex nibble count                  */
  INT         Type;                    /* validated symbol type             */
  RexxNumberString *TempNum;

  Len = String->length;                /* get validated string len          */
  Option = toupper(Option);            /* get the first character           */

                                       /* assume failure on checking        */
  Answer = (RexxObject *)TheFalseObject;
                                       /* get a scan pointer                */
  Scanp = (PCHAR)String->stringData;

  switch (Option) {                    /* based on type to confirm          */

    case DATATYPE_ALPHANUMERIC:        /* Alphanumeric                      */
                                       /* all in the set?                   */
      if (Len != 0 && !Memcpbrk(Scanp, ALPHANUM, Len))
                                       /* this is a good string             */
        Answer = (RexxObject *)TheTrueObject;
      break;

    case DATATYPE_BINARY:              /* Binary string                     */
                                       /* validate the string               */
      if (Len == 0 || ValSet(Scanp, Len, (PCHAR)BINARI, 4, &Count))
                                       /* this is a good string             */
        Answer = (RexxObject *)TheTrueObject;
      break;

    case DATATYPE_MIXED_DBCS:          /* Mixed SBCS/DBCS string            */
                                       /* pass on to the DBCS function      */
      Answer = String->DBCSdatatype(DATATYPE_MIXED_DBCS);
      break;

    case DATATYPE_PURE_DBCS:           /* Pure DBCS string                  */
                                       /* pass on to the DBCS function      */
      Answer = String->DBCSdatatype(DATATYPE_PURE_DBCS);
      break;

    case DATATYPE_LOWERCASE:           /* Lowercase                         */
      if (Len != 0 && !Memcpbrk(Scanp, LOWER_ALPHA, Len))
                                       /* this is a good string             */
        Answer = (RexxObject *)TheTrueObject;
      break;

    case DATATYPE_UPPERCASE:           /* Uppercase                         */
      if (Len != 0 && !Memcpbrk(Scanp, UPPER_ALPHA, Len))
                                       /* this is a good string             */
        Answer = (RexxObject *)TheTrueObject;
      break;

    case DATATYPE_MIXEDCASE:           /* Mixed case                        */
      if (Len != 0 && !Memcpbrk(Scanp, MIXED_ALPHA, Len))
                                       /* this is a good string             */
        Answer = (RexxObject *)TheTrueObject;
      break;

    case DATATYPE_WHOLE_NUMBER:        /* Whole number                      */
                                       /* validate as a number              */
      TempNum = String->numberString();
      if (TempNum != OREF_NULL) {      /* valid number?                     */
                                       /* force rounding to current digits  */
        TempNum = (RexxNumberString *)TempNum->plus(IntegerZero);
                                       /* check for integer then            */
        Answer = TempNum->isInteger();
      }
      break;

    case DATATYPE_NUMBER:              /* Number                            */
                                       /* validate as a number              */
      Temp = (RexxObject *)String->numberString();
      if (Temp != OREF_NULL)           /* valid number?                     */
                                       /* got a good one                    */
        Answer = (RexxObject *)TheTrueObject;
      break;

    case DATATYPE_9DIGITS:             /* NUMERIC DIGITS 9 number           */
                                       /* good long number                  */
      if (String->longValue(DEFAULT_DIGITS) != NO_LONG)
                                       /* say it's good                     */
        Answer = (RexxObject *)TheTrueObject;
      break;

    case DATATYPE_HEX:                 /* heXadecimal                       */
                                       /* validate the string               */
      if (Len == 0 || ValSet(Scanp, Len, (PCHAR)HEX_CHAR_STR, 2, &Count))
                                       /* valid hexadecimal                 */
        Answer = (RexxObject *)TheTrueObject;
      break;

    case DATATYPE_SYMBOL:              /* Symbol                            */
                                       /* validate the symbol               */
      if (String->isSymbol() != STRING_BAD_VARIABLE)
                                       /* is a valid symbol                 */
        Answer = (RexxObject *)TheTrueObject;
      break;

    case DATATYPE_VARIABLE:            /* Variable                          */

                                       /* validate the symbol               */
      Type = String->isSymbol();
                                       /* a valid variable type?            */
      if (Type == STRING_NAME ||
          Type == STRING_STEM ||
          Type == STRING_COMPOUND_NAME)
                                       /* is a valid symbol                 */
        Answer = (RexxObject *)TheTrueObject;
      break;

      case DATATYPE_LOGICAL:           // Test for a valid logical.
          if (Len != 1 || (*Scanp != '1' && *Scanp != '0'))
          {
              Answer = TheFalseObject;
          }
          else
          {
              Answer = TheTrueObject;
          }

          break;

    default  :                         /* unsupported option                */
      report_exception2(Error_Incorrect_method_option, new_cstring("ABCDLMNOSUVWX9"), new_string((PCHAR)&Option,1));
  }
  return Answer;                       /* return validation answer          */
}

RexxInteger *RexxString::abbrev(
    RexxString *info,                  /* target compared value             */
    RexxInteger *length)               /* minimum length                    */
/******************************************************************************/
/*  Function:  ABBREV string method                                           */
/******************************************************************************/
{
  size_t   Len1;                       /* length of string1                 */
  size_t   Len2;                       /* length of string1                 */
  size_t   ChkLen;                     /* required check length             */
  INT      rc;                         /* compare result                    */

  if (DBCS_MODE)                       /* need to use DBCS?                 */
                                       /* do the DBCS version               */
    return this->DBCSabbrev(info, length);

  info = get_string(info, ARG_ONE);    /* process the information string    */
  Len2 = info->length;                 /* get the length also               */
                                       /* get the optional check length     */
                                       /* get the optional check length     */
  ChkLen = optional_length(length, Len2, ARG_TWO);
  Len1 = this->length;                 /* get this length                   */

  if (ChkLen == 0 && Len2 == 0)        /* if null string match              */
    rc = 1;                            /* then we have an abbrev            */
  else if (Len1 == 0L ||               /* len 1 is zero,                    */
      (Len2 < ChkLen) ||               /* or second is too short            */
      (Len1 < Len2))                   /* or second is too long             */
    rc = 0;                            /* not an abbreviation               */

  else                                 /* now we have to check it           */
                                       /* do the comparison                 */
    rc = !(memcmp((PCHAR)this->stringData, info->stringData, Len2));
                                       /* return proper string value        */
  return (rc) ? IntegerOne : IntegerZero;
}


RexxInteger *RexxString::caselessAbbrev(RexxString *info, RexxInteger *length)
{
    // the info must be a string value
    info = get_string(info, ARG_ONE);
    stringsize_t len2 = info->getLength();
    // the check length is optional, and defaults to the length of info.
    stringsize_t chkLen = optional_length(length, len2, ARG_TWO);

    stringsize_t len1 = this->getLength();

    // if a null string match is allowed, this is true
    if (chkLen == 0 && len2 == 0)
    {
        return TheTrueObject;
    }

    // if the info is a null string, no match is possible
    // if the target string is shorter than the check length, also no match
    // if the info string is shorter than this string, not a match.
    if (len1 == 0 || (len2 < chkLen) || (len1 < len2))
    {
        return TheFalseObject;
    }
    /* do the comparison                 */
    return(CaselessCompare((PUCHAR)this->getStringData(), (PUCHAR)info->getStringData(), len2) == 0) ? TheTrueObject : TheFalseObject;
}


RexxInteger *RexxString::compare(
    RexxString *string2,               /* other string to compare against   */
    RexxString *pad)                   /* optional padding character        */
/******************************************************************************/
/*  Function:  String class COMPARE method/function.                          */
/******************************************************************************/
{
  UCHAR    PadChar;                    /* pad character                     */
  size_t   MisMatch;                   /* mismatch location                 */
  RexxInteger *Retval;                 /* returned result                   */
  PCHAR    String1;                    /* string 1 pointer                  */
  PCHAR    String2;                    /* string 2 pointer                  */
  size_t   Lead;                       /* leading length                    */
  size_t   Remainder;                  /* trailing length                   */
  size_t   i;                          /* loop index                        */
  size_t   Length1;                    /* first string length               */
  size_t   Length2;                    /* second string length              */

  if (DBCS_MODE)                       /* need to use DBCS?                 */
                                       /* do the DBCS version               */
    return this->DBCScompare(string2, pad);

  Length1 = this->length;              /* get this strings length           */
                                       /* validate the compare string       */
  string2 = get_string(string2, ARG_ONE);
  Length2 = string2->length;           /* get the length also               */
  PadChar = get_pad(pad, ' ', ARG_TWO);/* get the pad character             */
  if (Length1 > Length2) {             /* first longer?                     */
    String1 = (PCHAR)this->stringData;     /* make arg 1 first string           */
                                       /* arg 2 is second string            */
    String2 = string2->stringData;
    Lead = Length2;                    /* get shorter length                */
    Remainder = Length1 - Lead;        /* get trailing size                 */
  }
  else {                               /* make arg 2 first string           */
    String1 = string2->stringData;
    String2 = (PCHAR)this->stringData; /* arg 1 is second string            */
    Lead = Length1;                    /* get shorter length                */
    Remainder = Length2 - Lead;        /* get trailing size                 */
  }
  MisMatch = 0;                        /* assume they are equal             */
  i = 0;                               /* set the start                     */
  while (i < Lead) {                   /* if have leading compare           */
    if (String1[i] != String2[i]) {    /* not the same?                     */
      MisMatch = i + 1;                /* save position, origin one         */
      break;                           /* exit the loop                     */
    }
    i++;                               /* step the index                    */
  }
  if (!MisMatch && Remainder) {        /* need to handle padding?           */
    String1 += Lead;                   /* step to remainder                 */
    for (i = 0; i < Remainder; i++) {  /* scan the remainder                */
      if (String1[i] != PadChar) {     /* pad mismatch?                     */
        MisMatch = Lead + i + 1;       /* get mismatch position             */
        break;                         /* finished                          */
      }
    }
  }
  if (!MisMatch)
    Retval = IntegerZero;              /* this is zero                      */
  else
    Retval = new_integer(MisMatch);    /* make an integer return value      */
  return Retval;                       /* return result string              */
}


/**
 * Caseless version of the compare() method.
 *
 * @param string2 The string to compare to.
 * @param pad     The padding character used for length mismatches.
 *
 * @return 0 if the two strings are equal (with padding applied), otherwise
 *         it returns the mismatch position.
 */
RexxInteger *RexxString::caselessCompare(RexxString *other, RexxString *pad)
{
    stringsize_t length1 = this->getLength(); /* get this strings length           */
                                         /* validate the compare string       */
    other = get_string(other, ARG_ONE);
    stringsize_t length2 = other->getLength();       /* get the length also               */
    // we uppercase the pad character now since this is caseless
    stringchar_t padChar = toupper(get_pad(pad, ' ', ARG_TWO));

    stringchar_t *string1;
    stringchar_t *string2;
    stringsize_t lead;
    stringsize_t remainder;

    // is the first longer?
    if (length1 > length2)
    {
        string1 = (stringchar_t *)this->getStringData();   /* make arg 1 first string           */
                                           /* arg 2 is second string            */
        string2 = (stringchar_t *)other->getStringData();
        lead = length2;                    /* get shorter length                */
        remainder = length1 - lead;        /* get trailing size                 */
    }
    else
    {
        string1 = (stringchar_t *)other->getStringData();    /* make arg 2 first string           */
        string2 = (stringchar_t *)this->getStringData();     /* arg 1 is second string            */
        lead = length1;                      /* get shorter length                */
        remainder = length2 - lead;          /* get trailing size                 */
    }
    stringsize_t i = 0;                      /* set the start                     */
    // compare the leading parts
    for (i = 0; i < lead; i++)
    {
        // have a mismatch?
        if (toupper(string1[i]) != toupper(string2[i]))
        {
            return new_integer(i+1);           // return the mismatch position
        }
    }
    string1 += lead;              // step to the remainder and scan
    for (i = 0; i < remainder; i++)
    {
        // mismatch on the pad?
        if (toupper(string1[i]) != padChar)
        {
            // this is the mismatch position, return it
            return new_integer(lead + i + 1);
        }
    }
    return IntegerZero;    // no mismatch, return the failure indicator
}


RexxString *RexxString::copies(RexxInteger *copies)
/******************************************************************************/
/* Function:  String class COPIES method/function                             */
/******************************************************************************/
{
  size_t   Count;                      /* copies count                      */
  RexxString *Retval;                  /* return value                      */
  size_t   Len;                        /* copy string length                */
  PCHAR    Temp;                       /* copy location                     */

  if (DBCS_SELF)                       /* need to use DBCS?                 */
    ValidDBCS(this);                   /* validate the DBCS string          */
  required_arg(copies, ONE);           /* the count is required             */
                                       /* get the copies count              */
  Count = copies->requiredNonNegative(ARG_ONE);
  Len = this->length;                  /* get argument length               */

  if (Count == 0 ||                    /* no copies requested?              */
      Len == 0 )                       /* or copying a null string          */
    Retval = OREF_NULLSTRING;          /* just a null string                */
  else {                               /* get storage size                  */
                                       /* allocate storage needed           */
                                       /* allocate storage needed           */
    Retval = (RexxString *)raw_string(Len * Count);

    if (Len == 1) {                    /* if only 1 char long               */
                                       /* just do this with memset          */
      memset(Retval->stringData, this->stringData[0], Count);
    }
                                       /* if any copies                     */
    else {
                                       /* point to the string               */
      Temp = Retval->stringData;
      while (Count--) {                /* copy 2 thru n copies              */
                                       /* copy the string                   */
        memcpy(Temp, (PCHAR)this->stringData, Len);
        Temp += Len;
      }
    }
                                       /* done building the string          */
    Retval->generateHash();
  }
  return Retval;                       /* return copied string              */
}

RexxObject *RexxString::dataType(RexxString *ptype)
/******************************************************************************/
/* Function:  String class DATATYPE method/function                           */
/******************************************************************************/
{
 UINT type;

 if (ptype != OREF_NULL) {             /* see if type was specified?        */
                                       /* yes, specified, get 1st char      */
  type = option_character(ptype, 0, ARG_ONE);
                                       /* and call datatype routine to      */
  return DataType(this, type);         /* determine if its type specified.  */
 }
                                       /* type not specified, see if its a  */
                                       /* valid number                      */
 return (DataType(this, 'N') == TheTrueObject
         ? new_string("NUM",3)         /* if so we return NUM               */
         : new_string("CHAR",4));      /* otherwise we return CHAR          */
}


RexxInteger *RexxString::lastPosRexx(RexxString  *needle, RexxInteger *start)
{
    // if DBCS mode is turned on...pass it on.
    if (DBCS_MODE)
    {
        return this->DBCSlastPos(needle, start);
    }
    needle = REQUIRED_STRING(needle, ARG_ONE);
    // find out where to start the search. The default is at the very end.
    size_t startPos = optional_position(start, getLength(), ARG_TWO);
    // now perform the actual search.
    return new_integer(lastPos(needle, startPos));
}


RexxInteger *RexxString::caselessLastPosRexx(RexxString  *needle, RexxInteger *start)
{
    // validate that this is a good string argument
    needle = REQUIRED_STRING(needle, ARG_ONE);
    // find out where to start the search. The default is at the very end.
    size_t startPos = optional_position(start, getLength(), ARG_TWO);
    // now perform the actual search.
    return new_integer(caselessLastPos(needle, startPos));
}



/**
 * Primitive implementation of a lastpos search.
 *
 * @param needle The search needle.
 * @param start  The starting position (origin 1).
 *
 * @return Returns the last match position, searching back from the start
 *         position.  The starting position is the right-most character
 *         of the past possible match (as if the string was truncated
 *         at start).
 */
size_t RexxString::lastPos(RexxString  *needle, size_t start)
{
    size_t haystackLen = this->getLength();          /* get the haystack length           */
    size_t needleLen = needle->getLength();          /* and get the length too            */

    // no match possible if either string is null
    if (needleLen == 0 || haystackLen == 0)
    {
        return 0;
    }
    else
    {
        // get the start position for the search.
        haystackLen = min(start, haystackLen);
                                         /* do the search                     */
        PUCHAR matchLocation = lastPos((PUCHAR)needle->getStringData(), needleLen, (PUCHAR )this->getStringData(), haystackLen);
        if (matchLocation == NULL)
        {
            return 0;
        }
        else
        {
            return matchLocation - (PUCHAR)this->getStringData() + 1;
        }
    }
}


/**
 * Primitive implementation of a caseless lastpos search.
 *
 * @param needle The search needle.
 * @param start  The starting position (origin 1).
 *
 * @return Returns the last match position, searching back from the start
 *         position.  The starting position is the right-most character
 *         of the past possible match (as if the string was truncated
 *         at start).
 */
size_t RexxString::caselessLastPos(RexxString *needle, size_t start)
{
    size_t haystackLen = this->getLength();          /* get the haystack length           */
    size_t needleLen = needle->getLength();          /* and get the length too            */

    // no match possible if either string is null
    if (needleLen == 0 || haystackLen == 0)
    {
        return 0;
    }
    else
    {
        // get the start position for the search.
        haystackLen = min(start, haystackLen);
                                         /* do the search                     */
        PUCHAR matchLocation = caselessLastPos((PUCHAR)needle->getStringData(), needleLen, (PUCHAR )this->getStringData(), haystackLen);
        if (matchLocation == NULL)
        {
            return 0;
        }
        else
        {
            return matchLocation - (PUCHAR)this->getStringData() + 1;
        }
    }
}


/**
 * Absolutely most primitive version of a lastpos search.  This
 * version searches directly in a buffer rather than a Rexx
 * String.
 *
 * @param needle    Pointer to the needle string.
 * @param needleLen Length of the needle string.
 * @param haystack  The pointer to the haystack string.
 * @param haystackLen
 *                  The length of the haystack string.
 *
 * @return A pointer to the match location or NULL if there is no match.
 */
PUCHAR RexxString::lastPos(PUCHAR needle, size_t needleLen, PUCHAR  haystack, size_t haystackLen)
{
    // if the needle's longer than the haystack, no chance of a match
    if (needleLen > haystackLen)
    {
        return NULL;
    }
    // set the search startpoing point relative to the end of the search string
    haystack = haystack + haystackLen - needleLen;
    // this is the possible number of compares we might need to perform
    size_t count = haystackLen - needleLen + 1;
    // now scan backward
    while (count > 0)
    {
        // got a match at this position, return it directly
        if (memcmp(haystack, needle, needleLen) == 0)
        {
            return haystack;
        }
        // decrement count and position
        count--;
        haystack--;
    }
    return NULL;   // nothing to see here folks, move along
}


/**
 * Absolutely most primitive version of a caseless lastpos
 * search. This version searches directly in a buffer rather
 * than a Rexx String.
 *
 * @param needle    Pointer to the needle string.
 * @param needleLen Length of the needle string.
 * @param haystack  The pointer to the haystack string.
 * @param haystackLen
 *                  The length of the haystack string.
 *
 * @return A pointer to the match location or NULL if there is no match.
 */
PUCHAR RexxString::caselessLastPos(PUCHAR needle, size_t needleLen, PUCHAR  haystack, size_t haystackLen)
{
    // if the needle's longer than the haystack, no chance of a match
    if (needleLen > haystackLen)
    {
        return NULL;
    }
    // set the search startpoing point relative to the end of the search string
    haystack = haystack + haystackLen - needleLen;
    // this is the possible number of compares we might need to perform
    size_t count = haystackLen - needleLen + 1;
    // now scan backward
    while (count > 0)
    {
        // got a match at this position, return it directly
        if (CaselessCompare(haystack, needle, needleLen) == 0)
        {
            return haystack;
        }
        // decrement count and position
        count--;
        haystack--;
    }
    return NULL;   // nothing to see here folks, move along
}

size_t RexxString::countStr(RexxString *needle)
/******************************************************************************/
/* Function:  Count occurrences of one string in another.                     */
/******************************************************************************/
{
  size_t count;                        /* count of strings                  */
  size_t match;                        /* last match position               */
  size_t needlelength ;                /* length of the needle              */

  count = 0;                           /* no matches yet                    */
                                       /* get the length of the needle      */
  needlelength = needle->length;
                                       /* get the first match position      */
  match = this->pos(needle, 0);
  while (match != 0) {                 /* while we're getting matches       */
    count = count + 1;                 /* count this match                  */
                                       /* do the next search                */
    match = this->pos(needle, match + needlelength - 1);
  }
  return count;                        /* return the match count            */
}

RexxInteger *RexxString::countStrRexx(RexxString *needle)
/******************************************************************************/
/* Function:  Count occurrences of one string in another.                     */
/******************************************************************************/
{
  size_t count;                        /* count of strings                  */

                                       /* force needle to a string          */
  needle = REQUIRED_STRING(needle, ARG_ONE);
  count = this->countStr(needle);      /* do the counting                   */
  return new_integer(count);           /* return the count as an object     */
}

size_t RexxString::caselessCountStr(RexxString *needle)
/******************************************************************************/
/* Function:  Count occurrences of one string in another.                     */
/******************************************************************************/
{
  size_t count;                        /* count of strings                  */
  size_t match;                        /* last match position               */
  size_t needlelength ;                /* length of the needle              */

  count = 0;                           /* no matches yet                    */
                                       /* get the length of the needle      */
  needlelength = needle->length;
                                       /* get the first match position      */
  match = this->caselessPos(needle, 0);
  while (match != 0) {                 /* while we're getting matches       */
    count = count + 1;                 /* count this match                  */
                                       /* do the next search                */
    match = this->caselessPos(needle, match + needlelength - 1);
  }
  return count;                        /* return the match count            */
}

RexxInteger *RexxString::caselessCountStrRexx(RexxString *needle)
/******************************************************************************/
/* Function:  Count occurrences of one string in another.                     */
/******************************************************************************/
{
  size_t count;                        /* count of strings                  */

                                       /* force needle to a string          */
  needle = REQUIRED_STRING(needle, ARG_ONE);
  count = this->caselessCountStr(needle); /* do the counting                   */
  return new_integer(count);              /* return the count as an object     */
}

RexxString *RexxString::changeStr(RexxString *needle, RexxString *newNeedle, RexxInteger *countArg)
/******************************************************************************/
/* Function:  Change strings into another string.                             */
/******************************************************************************/
{
  size_t start;                        /* converted start position          */
  size_t match;                        /* last match position               */
  size_t needleLength;                 /* length of the needle              */
  size_t newLength;                    /* length of the replacement string  */
  size_t matches;                      /* number of replacements            */
  size_t copyLength;                   /* length to copy                    */
  PCHAR source;                        /* point to the string source        */
  PCHAR copy;                          /* current copy position             */
  PCHAR newPtr;                        /* pointer to replacement data       */
  RexxString *result;                  /* returned result string            */
  size_t i;

                                       /* force needle to a string          */
  needle = REQUIRED_STRING(needle, ARG_ONE);
                                       /* newneedle must be a string two    */
  newNeedle = REQUIRED_STRING(newNeedle, ARG_TWO);

  // we'll only change up to a specified count.  If not there, we do everything.
  size_t count = optionalPositive(countArg, MAX_WHOLE_NUMBER, ARG_THREE);
  matches = this->countStr(needle);    /* find the number of replacements   */
  if (matches > count)                 // the matches are bounded by the count
  {
      matches = count;
  }
  needleLength = needle->length;       /* get the length of the needle      */
  newLength = newNeedle->length;       /* and the replacement length        */
                                       /* get a proper sized string         */
  result = (RexxString *)raw_string(this->length - (matches * needleLength) + (matches * newLength));
  copy = result->stringData;           /* point to the copy location        */
  source = this->stringData;           /* and out own data                  */
                                       /* and the string to replace         */
  newPtr = newNeedle->stringData;
  start = 0;                           /* set a zero starting point         */
  for (i = 0; i < matches; i++) {      /* until we hit count or run out     */
    match = this->pos(needle, start);  /* look for the next occurrence      */
    if (match == 0)                    /* not found?                        */
      break;                           /* get out of here                   */
    copyLength = (match - 1) - start;  /* get the next length to copy       */
    if (copyLength != 0) {             /* something to copy?                */
                                       /* add on the next string section    */
      memcpy(copy, source + start, copyLength);
      copy += copyLength;              /* step over the copied part         */
    }
    if (newLength != 0) {              /* something to replace with?        */
      memcpy(copy, newPtr, newLength); /* copy over the new segment         */
      copy += newLength;               /* and step it over also             */
    }
    start = match + needleLength - 1;  /* step to the next position         */
  }
  if (start < this->length)            /* some remainder left?              */
                                       /* add it on                         */
    memcpy(copy, source + start, this->length - start);
  result->generateHash();              /* now finishe off this string       */
  return result;                       /* finished                          */
}

RexxString *RexxString::caselessChangeStr(RexxString *needle, RexxString *newNeedle, RexxInteger *countArg)
/******************************************************************************/
/* Function:  Change strings into another string.                             */
/******************************************************************************/
{
  size_t start;                        /* converted start position          */
  size_t match;                        /* last match position               */
  size_t needleLength;                 /* length of the needle              */
  size_t newLength;                    /* length of the replacement string  */
  size_t matches;                      /* number of replacements            */
  size_t copyLength;                   /* length to copy                    */
  PCHAR source;                        /* point to the string source        */
  PCHAR copy;                          /* current copy position             */
  PCHAR newPtr;                        /* pointer to replacement data       */
  RexxString *result;                  /* returned result string            */
  size_t i;

                                       /* force needle to a string          */
  needle = REQUIRED_STRING(needle, ARG_ONE);
                                       /* newneedle must be a string two    */
  newNeedle = REQUIRED_STRING(newNeedle, ARG_TWO);
  // we'll only change up to a specified count.  If not there, we do everything.
  size_t count = optionalPositive(countArg, MAX_WHOLE_NUMBER, ARG_THREE);

  matches = this->caselessCountStr(needle);    /* find the number of replacements   */
  if (matches > count)                 // the matches are bounded by the count
  {
      matches = count;
  }
  needleLength = needle->length;       /* get the length of the needle      */
  newLength = newNeedle->length;       /* and the replacement length        */
                                       /* get a proper sized string         */
  result = (RexxString *)raw_string(this->length - (matches * needleLength) + (matches * newLength));
  copy = result->stringData;           /* point to the copy location        */
  source = this->stringData;           /* and out own data                  */
                                       /* and the string to replace         */
  newPtr = newNeedle->stringData;
  start = 0;                           /* set a zero starting point         */
  for (i = 0; i < matches; i++) {      /* until we hit count or run out     */
    match = this->caselessPos(needle, start);  /* look for the next occurrence      */
    if (match == 0)                    /* not found?                        */
      break;                           /* get out of here                   */
    copyLength = (match - 1) - start;  /* get the next length to copy       */
    if (copyLength != 0) {             /* something to copy?                */
                                       /* add on the next string section    */
      memcpy(copy, source + start, copyLength);
      copy += copyLength;              /* step over the copied part         */
    }
    if (newLength != 0) {              /* something to replace with?        */
      memcpy(copy, newPtr, newLength); /* copy over the new segment         */
      copy += newLength;               /* and step it over also             */
    }
    start = match + needleLength - 1;  /* step to the next position         */
  }
  if (start < this->length)            /* some remainder left?              */
                                       /* add it on                         */
    memcpy(copy, source + start, this->length - start);
  result->generateHash();              /* now finishe off this string       */
  return result;                       /* finished                          */
}


RexxInteger *RexxString::posRexx(
    RexxString  *needle,               /* search string                     */
    RexxInteger *pstart)               /* starting position                 */
/******************************************************************************/
/* Function:  String class POS method/function                                */
/******************************************************************************/
{
  size_t start;                        /* converted start position          */

                                       /* force needle to a string          */
  needle = REQUIRED_STRING(needle, ARG_ONE);
                                       /* get the starting position         */
  start = optional_position(pstart, 1, ARG_TWO);
                                       /* pass on to the primitive function */
                                       /* and return as an integer object   */
  return new_integer(this->pos(needle, start - 1));
}


RexxInteger *RexxString::caselessPosRexx(
    RexxString  *needle,               /* search string                     */
    RexxInteger *pstart)               /* starting position                 */
/******************************************************************************/
/* Function:  String class POS method/function                                */
/******************************************************************************/
{
  size_t start;                        /* converted start position          */

                                       /* force needle to a string          */
  needle = REQUIRED_STRING(needle, ARG_ONE);
                                       /* get the starting position         */
  start = optional_position(pstart, 1, ARG_TWO);
                                       /* pass on to the primitive function */
                                       /* and return as an integer object   */
  return new_integer(this->caselessPos(needle, start - 1));
}


size_t RexxString::pos(RexxString *needle, size_t start)
{
    // DBCS mode is handled elsewhere
    if (DBCS_MODE)
    {
        return this->DBCSpos(needle, start);
    }

    // get the two working lengths
    size_t haystack_length = getLength();
    size_t needle_length = needle->getLength();

    // ok, there are a few quick checks we can perform.  If the needle is
    // bigger than the haystack, or the needle is a null string or
    // our haystack length after adjusting to the starting position
    // zero, then we can quickly return zero.
    if (needle_length > haystack_length + start || needle_length == 0 || start + needle_length > haystack_length)
    {
        return 0;
    }

    // address the string value
    PUCHAR haypointer = (PUCHAR)getStringData() + start;
    PUCHAR needlepointer = (PUCHAR)needle->getStringData();
    size_t location = start + 1;         // this is the match location as an index
    // calculate the number of probes we can make in this string
    size_t count = (haystack_length - start) - needle_length + 1;

    // now scan
    while (count--)
    {
                                           /* get a hit?                        */
        if (memcmp((PCHAR)haypointer, (PCHAR)needlepointer, needle_length) == 0)
        {
            return location;
        }
        // step our pointers accordingly
        location++;
        haypointer++;
    }
    return 0;  // we got nothing...
}


size_t RexxString::caselessPos(RexxString *needle, size_t start)
{
    // DBCS mode is handled elsewhere
    if (DBCS_MODE)
    {
        return this->DBCScaselessPos(needle, start);
    }

    // get the two working lengths
    size_t haystack_length = getLength();
    size_t needle_length = needle->getLength();

    // ok, there are a few quick checks we can perform.  If the needle is
    // bigger than the haystack, or the needle is a null string or
    // our haystack length after adjusting to the starting position
    // zero, then we can quickly return zero.
    if (needle_length > haystack_length + start || needle_length == 0 || start + needle_length > haystack_length)
    {
        return 0;
    }

    // address the string value
    PUCHAR haypointer = (PUCHAR)getStringData() + start;
    PUCHAR needlepointer = (PUCHAR)needle->getStringData();
    size_t location = start + 1;         // this is the match location as an index
    // calculate the number of probes we can make in this string
    size_t count = (haystack_length - start) - needle_length + 1;

    // now scan
    while (count--)
    {
                                           /* get a hit?                        */
        if (CaselessCompare(haypointer, needlepointer, needle_length) == 0)
        {
            return location;
        }
        // step our pointers accordingly
        location++;
        haypointer++;
    }
    return 0;  // we got nothing...
}

size_t MemPos(
  PUCHAR  String,                      /* search string                     */
  size_t Length,                       /* string length                     */
  UCHAR  Char )                        /* target character                  */
/*********************************************************************/
/*  Function:  offset of first occurrence of char in string          */
/*********************************************************************/
{
  PUCHAR  Scan;                        /* scan location                     */
  size_t Position;                     /* matched position                  */

  Position = -1;                       /* default to no match               */
                                       /* while in the string               */
  for (Scan = String; Length; Length--) {
    if (*Scan == Char) {               /* find a match?                     */
      Position = Scan - String;        /* return difference                 */
      break;                           /* quit the loop                     */
    }
    Scan++;                            /* step the position                 */
  }
  return Position;                     /* return match position             */
}

RexxString *RexxString::translate(
    RexxString *tableo,                /* output table                      */
    RexxString *tablei,                /* input table                       */
    RexxString *pad)                   /* pad character                     */
/******************************************************************************/
/*  Function:  String class TRANSLATE method/function                         */
/******************************************************************************/
{
  RexxString *Retval;                  /* return value                      */
  PUCHAR    OutTable;                  /* output table                      */
  size_t    OutTableLength;            /* length of output table            */
  PUCHAR    InTable;                   /* input table                       */
  PUCHAR    ScanPtr;                   /* scanning pointer                  */
  size_t    ScanLength;                /* scanning length                   */
  size_t    InTableLength;             /* length of input table             */
  UCHAR     PadChar;                   /* pad character                     */
  UCHAR     ch;                        /* current character                 */
  size_t    Position;                  /* table position                    */

  if (DBCS_MODE)                       /* need to use DBCS?                 */
                                       /* do the DBCS version               */
    return (RexxString *)this->DBCStranslate(tableo, tablei, pad);


                                       /* just a simple uppercase?          */
 if (tableo == OREF_NULL && tablei == OREF_NULL && pad == OREF_NULL)
   return this->upper();               /* return the uppercase version      */
                                       /* validate the tables               */
                                       /* validate the tables               */
 tableo = optional_string(tableo, OREF_NULLSTRING, ARG_ONE);
 OutTableLength = tableo->length;      /* get the table length              */
                                       /* input table too                   */
 tablei = optional_string(tablei, OREF_NULLSTRING, ARG_TWO);
 InTableLength = tablei->length;       /* get the table length              */
 InTable = (PUCHAR)tablei->stringData; /* point at the input table          */
 OutTable = (PUCHAR)tableo->stringData;/* and the output table              */
                                       /* get the pad character             */
  PadChar = get_pad(pad, ' ', ARG_THREE);
                                       /* allocate space for answer         */
                                       /* and copy the string               */
 Retval = new_string((PCHAR)this->stringData, this->length);
 ScanPtr = (PUCHAR)Retval->stringData; /* point to data                     */
 ScanLength = this->length;            /* get the length too                */

 while (ScanLength--) {                /* spin thru input                   */
   ch = (UCHAR)*ScanPtr;               /* get a character                   */

   if (tablei != OREF_NULLSTRING)      /* input table specified?            */
                                       /* search for the character          */
     Position = MemPos(InTable, InTableLength, ch);
   else
     Position = (size_t)ch;            /* position is the character value   */
   if (Position != -1) {               /* found in the table?               */
     if (Position < OutTableLength)    /* in the output table?              */
                                       /* convert the character             */
       *ScanPtr = *(OutTable + Position);
     else
       *ScanPtr = PadChar;             /* else use the pad character        */
   }
   ScanPtr++;                          /* step the pointer                  */
 }
 Retval->generateHash();               /* generate the new hash value       */
 return Retval;                        /* return translated string          */
}

RexxInteger *RexxString::verify(
    RexxString  *ref,                  /* compare reference string          */
    RexxString  *option,               /* Match/NoMatch option              */
    RexxInteger *start)                /* optional starg position           */
/******************************************************************************/
/*  Function:  String class VERIFY function                                   */
/******************************************************************************/
{
  size_t    StartPos;                  /* start position                    */
  size_t    StringLen;                 /* length of string                  */
  size_t    Position;                  /* returned position                 */
  size_t    ReferenceLen;              /* length of reference set           */
  size_t    Temp;                      /* temporary scan length             */
  RexxInteger *Retval;                 /* return value                      */
  CHAR      Option;                    /* verify option                     */
  PCHAR     Reference;                 /* reference pointer                 */
  PCHAR     Current;                   /* current scan position             */
  CHAR      ch;                        /* scan character                    */
  BOOL      Match;                     /* found a match                     */

  if (DBCS_MODE)                       /* need to use DBCS?                 */
                                       /* do the DBCS version               */
    return this->DBCSverify(ref, option, start);

  ref = get_string(ref, ARG_ONE);      /* get the reference string          */
  ReferenceLen = ref->length;          /* get a length also                 */
                                       /* get the option, default 'Nomatch' */
  Option = option_character(option, VERIFY_NOMATCH, ARG_TWO);
  if (Option != VERIFY_MATCH &&        /* options are 'Match' and           */
      Option != VERIFY_NOMATCH)        /* 'NoMatch'                         */
                                       /* not that either, then its an error*/
    report_exception2(Error_Incorrect_method_option, new_string("MN", 2), option);

                                       /* get starting position             */
  StartPos = optional_position(start, 1, ARG_THREE);
  StringLen = this->length;            /* get the string length             */
  if (StartPos > StringLen)            /* beyond end of string?             */
    Retval = IntegerZero;              /* couldn't find it                  */
  else {
                                       /* point at start position           */
    Current = (PCHAR)this->stringData + StartPos - 1;
    StringLen -= (StartPos - 1);       /* reduce the length                 */
    Position = 0;                      /* haven't found it yet              */

    if (!ReferenceLen) {               /* if verifying a nullstring         */
      if (Option == VERIFY_MATCH)      /* can't match at all                */
        Retval = IntegerZero;          /* so return zero                    */
      else
        Retval = new_integer(StartPos);/* non-match at start position       */
    }
    else {                             /* need to really search             */
      while (StringLen--) {            /* while input left                  */
        ch = *Current++;               /* get next char                     */
                                       /* get reference string              */
        Reference = ref->stringData;
        Temp = ReferenceLen;           /* copy the reference length         */
        Match = FALSE;                 /* no match yet                      */

        while (Temp--) {               /* spin thru reference               */
          if (ch == *Reference++) {    /* in reference ?                    */
            Match = TRUE;              /* had a match                       */
            break;                     /* quit the loop                     */
          }
        }
                                       /* have needed matching?             */
        if ((Match && Option == VERIFY_MATCH) ||
            (!Match && Option == VERIFY_NOMATCH)) {
                                       /* calculate the position            */
          Position = Current - (PCHAR)this->stringData;
          break;                       /* done searching                    */
        }
      }
                                       /* format the position               */
      Retval = Position ? new_integer(Position) : IntegerZero;
    }
  }
  return Retval;                       /* return formatted number           */
}


/**
 * Test if regions within two strings match.
 *
 * @param start_  The starting compare position within the target string.  This
 *                must be within the bounds of the string.
 * @param other   The other compare string.
 * @param offset_ The starting offset of the compare string.  This must be
 *                within the string bounds.  The default start postion is 1.
 * @param len_    The length of the compare substring.  The length and the
 *                offset must specify a valid substring of other.  If not
 *                specified, this defaults to the substring from the
 *                offset to the end of the string.
 *
 * @return True if the two regions match, false for any mismatch.
 */
RexxInteger *RexxString::match(RexxInteger *start_, RexxString *other, RexxInteger *offset_, RexxInteger *len_)
{
    stringsize_t start = positionArgument(start_, ARG_ONE);
    // the start position must be within the string bounds
    if (start > getLength())
    {
        reportException(Error_Incorrect_method_position, start);
    }
    other = stringArgument(other, ARG_TWO);

    stringsize_t offset = optionalPositionArgument(offset_, 1, ARG_THREE);

    if (offset > other->getLength())
    {
        reportException(Error_Incorrect_method_position, offset);
    }

    stringsize_t len = optionalLengthArgument(len_, other->getLength() - offset + 1, ARG_FOUR);

    if ((offset + len - 1) > other->getLength())
    {
        reportException(Error_Incorrect_method_length, len);
    }

    return primitiveMatch(start, other, offset, len) ? TheTrueObject : TheFalseObject;
}


/**
 * Test if regions within two strings match.
 *
 * @param start_  The starting compare position within the target string.  This
 *                must be within the bounds of the string.
 * @param other   The other compare string.
 * @param offset_ The starting offset of the compare string.  This must be
 *                within the string bounds.  The default start postion is 1.
 * @param len_    The length of the compare substring.  The length and the
 *                offset must specify a valid substring of other.  If not
 *                specified, this defaults to the substring from the
 *                offset to the end of the string.
 *
 * @return True if the two regions match, false for any mismatch.
 */
RexxInteger *RexxString::caselessMatch(RexxInteger *start_, RexxString *other, RexxInteger *offset_, RexxInteger *len_)
{
    stringsize_t start = positionArgument(start_, ARG_ONE);
    // the start position must be within the string bounds
    if (start > getLength())
    {
        reportException(Error_Incorrect_method_position, start);
    }
    other = stringArgument(other, ARG_TWO);

    stringsize_t offset = optionalPositionArgument(offset_, 1, ARG_THREE);

    if (offset > other->getLength())
    {
        reportException(Error_Incorrect_method_position, offset);
    }

    stringsize_t len = optionalLengthArgument(len_, other->getLength() - offset + 1, ARG_FOUR);

    if ((offset + len - 1) > other->getLength())
    {
        reportException(Error_Incorrect_method_length, len);
    }

    return primitiveCaselessMatch(start, other, offset, len) ? TheTrueObject : TheFalseObject;
}


/**
 * Perform a compare of regions of two string objects.  Returns
 * true if the two regions match, returns false for mismatches.
 *
 * @param start  The starting offset within the target string.
 * @param other  The source string for the compare.
 * @param offset The offset of the substring of the other string to use.
 * @param len    The length of the substring to compare.
 *
 * @return True if the regions match, false otherwise.
 */
bool RexxString::primitiveMatch(stringsize_t start, RexxString *other, stringsize_t offset, stringsize_t len)
{
    start--;      // make the starting point origin zero
    offset--;

    // if the match is not possible in the target string, just return false now.
    if ((start + len) > getLength())
    {
        return false;
    }

    return memcmp(getStringData() + start, other->getStringData() + offset, len) == 0;
}


/**
 * Perform a caselesee compare of regions of two string objects.
 * Returns true if the two regions match, returns false for
 * mismatches.
 *
 * @param start  The starting offset within the target string.
 * @param other  The source string for the compare.
 * @param offset The offset of the substring of the other string to use.
 * @param len    The length of the substring to compare.
 *
 * @return True if the regions match, false otherwise.
 */
bool RexxString::primitiveCaselessMatch(stringsize_t start, RexxString *other, stringsize_t offset, stringsize_t len)
{
    start--;      // make the starting point origin zero
    offset--;

    // if the match is not possible in the target string, just return false now.
    if ((start + len) > getLength())
    {
        return false;
    }

    return CaselessCompare((PUCHAR)(getStringData() + start), (PUCHAR)(other->getStringData() + offset), len) == 0;
}


/**
 * Compare a single character at a give position against
 * a set of characters to see if any of the characters is
 * a match.
 *
 * @param position_ The character position
 * @param matchSet  The set to compare against.
 *
 * @return true if the character at the give position is any of the characters,
 *         false if none of them match.
 */
RexxInteger *RexxString::matchChar(RexxInteger *position_, RexxString *matchSet)
{
    stringsize_t position = positionArgument(position_, ARG_ONE);
    // the start position must be within the string bounds
    if (position > getLength())
    {
        reportException(Error_Incorrect_method_position, position);
    }
    matchSet = stringArgument(matchSet, ARG_TWO);

    stringsize_t setLength = matchSet->getLength();
    stringchar_t matchChar = getChar(position - 1);

    // iterate through the match set looking for a match
    for (stringsize_t i = 0; i < setLength; i++)
    {
        if (matchChar == (stringchar_t)matchSet->getChar(i))
        {
            return TheTrueObject;
        }
    }
    return TheFalseObject;
}


/**
 * Compare a single character at a give position against
 * a set of characters to see if any of the characters is
 * a match.
 *
 * @param position_ The character position
 * @param matchSet  The set to compare against.
 *
 * @return true if the character at the give position is any of the characters,
 *         false if none of them match.
 */
RexxInteger *RexxString::caselessMatchChar(RexxInteger *position_, RexxString *matchSet)
{
    stringsize_t position = positionArgument(position_, ARG_ONE);
    // the start position must be within the string bounds
    if (position > getLength())
    {
        reportException(Error_Incorrect_method_position, position);
    }
    matchSet = stringArgument(matchSet, ARG_TWO);

    stringsize_t setLength = matchSet->getLength();
    stringchar_t matchChar = getChar(position - 1);

    // iterate through the match set looking for a match, using a
    // caseless compare
    for (stringsize_t i = 0; i < setLength; i++)
    {
        if (toupper(matchChar) == toupper(matchSet->getChar(i)))
        {
            return TheTrueObject;
        }
    }
    return TheFalseObject;
}


/**
 * Do a sorting comparison of two strings.
 *
 * @param other  The other compare string.
 * @param start_ The starting compare position within the target string.
 * @param len_   The length of the compare substring.
 *
 * @return True if the two regions match, false for any mismatch.
 */
RexxInteger *RexxString::compareToRexx(RexxString *other, RexxInteger *start_, RexxInteger *len_)
{
    other = stringArgument(other, ARG_ONE);

    stringsize_t start = optionalPositionArgument(start_, 1, ARG_TWO);
    stringsize_t len = optionalLengthArgument(len_, max(getLength(), other->getLength()) - start + 1, ARG_THREE);

    return primitiveCompareTo(other, start, len);
}


/**
 * Perform a compare of regions of two string objects.  Returns
 * -1, 0, 1 based on the relative ordering of the two strings.
 *
 * @param other  The source string for the compare.
 * @param start  The starting offset within the target string.
 * @param len    The length of the substring to compare.
 *
 * @return -1 if the target string is less than, 0 if the two strings are
 *         equal, 1 if the target string is the greater.
 */
RexxInteger *RexxString::primitiveCompareTo(RexxString *other, stringsize_t start, stringsize_t len)
{
    stringsize_t myLength = getLength();
    stringsize_t otherLength = other->getLength();

    // if doing the compare outside of the string length, we're less than the other string
    // unless the start is
    if (start > myLength)
    {
        return start > otherLength ? IntegerZero : IntegerMinusOne;
    }
    // if beyond the other length, they we're the larger
    if (start > otherLength)
    {
        return IntegerOne;
    }

    start--;      // make the starting point origin zero

    myLength = min(len, myLength - start);
    otherLength = min(len, otherLength - start);

    len = min(myLength, otherLength);

    wholenumber_t result = memcmp(getStringData() + start, other->getStringData() + start, len);

    // if they compare equal, then they are only
    if (result == 0)
    {
        if (myLength == otherLength)
        {
            return IntegerZero;
        }
        else if (myLength > otherLength)
        {
            return IntegerOne;
        }
        else
        {
            return IntegerMinusOne;
        }
    }
    else if (result > 0)
    {
        return IntegerOne;
    }
    else
    {
        return IntegerMinusOne;
    }
}




/**
 * Do a sorting comparison of two strings.
 *
 * @param other  The other compare string.
 * @param start_ The starting compare position within the target string.
 * @param len_   The length of the compare substring.
 *
 * @return True if the two regions match, false for any mismatch.
 */
RexxInteger *RexxString::caselessCompareToRexx(RexxString *other, RexxInteger *start_, RexxInteger *len_)
{
    other = stringArgument(other, ARG_ONE);

    stringsize_t start = optionalPositionArgument(start_, 1, ARG_TWO);
    stringsize_t len = optionalLengthArgument(len_, max(getLength(), other->getLength()) - start + 1, ARG_THREE);

    return primitiveCaselessCompareTo(other, start, len);
}




/**
 * Perform a compare of regions of two string objects.  Returns
 * -1, 0, 1 based on the relative ordering of the two strings.
 *
 * @param other  The source string for the compare.
 * @param start  The starting offset within the target string.
 * @param len    The length of the substring to compare.
 *
 * @return -1 if the target string is less than, 0 if the two strings are
 *         equal, 1 if the target string is the greater.
 */
RexxInteger *RexxString::primitiveCaselessCompareTo(RexxString *other, stringsize_t start, stringsize_t len)
{
    stringsize_t myLength = getLength();
    stringsize_t otherLength = other->getLength();

    // if doing the compare outside of the string length, we're less than the other string
    // unless the start is
    if (start > myLength)
    {
        return start > otherLength ? IntegerZero : IntegerMinusOne;
    }
    // if beyond the other length, they we're the larger
    if (start > otherLength)
    {
        return IntegerOne;
    }

    start--;      // make the starting point origin zero

    myLength = min(len, myLength - start);
    otherLength = min(len, otherLength - start);

    len = min(myLength, otherLength);

    wholenumber_t result = CaselessCompare((PUCHAR)getStringData() + start, (PUCHAR)other->getStringData() + start, len);

    // if they compare equal, then they are only
    if (result == 0)
    {
        if (myLength == otherLength)
        {
            return IntegerZero;
        }
        else if (myLength > otherLength)
        {
            return IntegerOne;
        }
        else
        {
            return IntegerMinusOne;
        }
    }
    else if (result > 0)
    {
        return IntegerOne;
    }
    else
    {
        return IntegerMinusOne;
    }
}
