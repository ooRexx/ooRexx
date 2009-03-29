/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                                                */
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
#include "SourceFile.hpp"
#include "ActivityManager.hpp"
#include "StringUtil.hpp"

int RexxString::isSymbol()
/*********************************************************************/
/*                                                                   */
/*   Function:         determines valid rexx symbols and returns     */
/*                     a type indicator for valid symbols that can   */
/*                     be passed on to the dictionary routines.      */
/*                                                                   */
/*********************************************************************/
{
    const char *Scan;                    /* string scan pointer               */
    size_t     Compound;                 /* count of periods                  */
    size_t     i;                        /* loop counter                      */
    const char *Linend;                  /* end of line                       */
    int        Type;                     /* return type                       */

                                         /* name too long                     */
                                         /* or too short                      */
    if (this->getLength() > (size_t)MAX_SYMBOL_LENGTH || this->getLength() == 0)
    {
        return STRING_BAD_VARIABLE;        /* set a bad type                    */
    }

                                           /* step to end                       */
    Linend = this->getStringData() + this->getLength();

    Compound = 0;                        /* set compound name is no           */
    Scan = this->getStringData();        /* save start position               */
                                         /* while still part of symbol        */
    while (Scan < Linend && RexxSource::isSymbolCharacter(*Scan))
    {

        if (*Scan == '.')                  /* a dot found..                     */
        {
            Compound++;                      /* indicate a compound var           */
        }

        Scan++;                            /* step the pointer                  */
    }                                    /* len of symbol                     */
    /* now check for exponent            */
    if (((Scan + 1) < Linend) &&
        (*Scan == '-' || *Scan == '+') &&
        (isdigit(this->getChar(0)) || *Scan == '.') &&
        (toupper(*(Scan - 1)) == 'E'))
    {
        Scan++;                            /* step to next                      */

        while (Scan < Linend)
        {            /* while more characters             */
            if (!isdigit(*Scan))             /* if not a digit                    */
            {
                return STRING_BAD_VARIABLE;    /* this isn't valid                  */
            }
            Scan++;                          /* step to next char                 */
        }
    }
    if (Scan < Linend)                   /* use the entire string?            */
    {
        return STRING_BAD_VARIABLE;        /* no, can't be good                 */
    }
                                           /* now determine symbol type         */
                                           /* possible number?                  */
    if (this->getChar(0) == '.' || isdigit(this->getChar(0)))
    {

        /* only a period?                    */
        if (Compound == 1 && this->getLength() == 1)
        {
            Type = STRING_LITERAL_DOT;       /* yes, set the token type           */
        }
        else if (Compound > 1)             /* too many periods?                 */
        {
            Type = STRING_LITERAL;           /* yes, just a literal token         */
        }
        else
        {                             /* check for a real number           */
            Type = STRING_NUMERIC;           /* assume numeric for now            */
            Scan = this->getStringData();    /* point to symbol                   */
                                             /* scan symbol, validating           */
            for (i = this->getLength() ; i; i-- )
            {
                if (!isdigit(*Scan) &&         /* if not a digit and                */
                    *Scan != '.')              /* and not a period...               */
                {
                    break;                       /* finished                          */
                }
                Scan++;                        /* step to next character            */
            }
            if (i > 1 &&                     /* if tripped over an 'E'            */
                toupper(*Scan) == 'E')
            {     /* could be exponential              */
                Scan++;                        /* step past E                       */
                i--;                           /* count the character               */
                                               /* +/- case already validated        */
                if (*Scan != '+' && *Scan != '-')
                {
                    for (; i; i--)
                    {              /* scan rest of symbol               */
                        if (!isdigit(*Scan))
                        {     /* if not a digit...                 */
                            Type = STRING_LITERAL;   /* not a number                      */
                            break;
                        }
                        Scan++;                    /* step to next character            */
                    }
                }
            }
            else if (i)                      /* literal if stuff left             */
            {
                Type = STRING_LITERAL;         /* yes, just a literal token         */
            }
        }
    }

    else if (!Compound)
    {                /* not a compound so...              */
        Type = STRING_NAME;                /* set the token type                */
    }
    /* is it a stem?                     */
    else if (Compound == 1 && *(Scan - 1) == '.')
    {
        Type = STRING_STEM;                /* yes, set the token type           */
    }
    else
    {
        Type = STRING_COMPOUND_NAME;       /* otherwise just plain              */
    }
                                           /* compound                          */
    return Type;                         /* return the type info              */
}

RexxInteger *RexxString::abbrev(
    RexxString *info,                  /* target compared value             */
    RexxInteger *_length)              /* minimum length                    */
/******************************************************************************/
/*  Function:  ABBREV string method                                           */
/******************************************************************************/
{
    size_t   Len1;                       /* length of string1                 */
    size_t   Len2;                       /* length of string1                 */
    size_t   ChkLen;                     /* required check length             */
    int      rc;                         /* compare result                    */

    info = stringArgument(info, ARG_ONE);    /* process the information string    */
    Len2 = info->getLength();                 /* get the length also               */
    /* get the optional check length     */
    /* get the optional check length     */
    ChkLen = optionalLengthArgument(_length, Len2, ARG_TWO);
    Len1 = this->getLength();                 /* get this length                   */

    if (ChkLen == 0 && Len2 == 0)        /* if null string match              */
    {
        rc = 1;                            /* then we have an abbrev            */
    }
    else if (Len1 == 0L ||               /* len 1 is zero,                    */
             (Len2 < ChkLen) ||               /* or second is too short            */
             (Len1 < Len2))                   /* or second is too long             */
    {
        rc = 0;                            /* not an abbreviation               */
    }

    else                                 /* now we have to check it           */
    {
        /* do the comparison                 */
        rc = !(memcmp(this->getStringData(), info->getStringData(), Len2));
    }
    /* return proper string value        */
    return(rc) ? IntegerOne : IntegerZero;
}


RexxInteger *RexxString::caselessAbbrev(RexxString *info, RexxInteger *_length)
{
    // the info must be a string value
    info = stringArgument(info, ARG_ONE);
    stringsize_t len2 = info->getLength();
    // the check length is optional, and defaults to the length of info.
    stringsize_t chkLen = optionalLengthArgument(_length, len2, ARG_TWO);

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
    return(StringUtil::caselessCompare(this->getStringData(), info->getStringData(), len2) == 0) ? TheTrueObject : TheFalseObject;
}


RexxInteger *RexxString::compare(
    RexxString *string2,               /* other string to compare against   */
    RexxString *pad)                   /* optional padding character        */
/******************************************************************************/
/*  Function:  String class COMPARE method/function.                          */
/******************************************************************************/
{
    char     PadChar;                    /* pad character                     */
    size_t   MisMatch;                   /* mismatch location                 */
    RexxInteger *Retval;                 /* returned result                   */
    const char *String1;                 /* string 1 pointer                  */
    const char *String2;                 /* string 2 pointer                  */
    size_t   Lead;                       /* leading length                    */
    size_t   Remainder;                  /* trailing length                   */
    size_t   i;                          /* loop index                        */
    size_t   Length1;                    /* first string length               */
    size_t   Length2;                    /* second string length              */

    Length1 = this->getLength();              /* get this strings length           */
    /* validate the compare string       */
    string2 = stringArgument(string2, ARG_ONE);
    Length2 = string2->getLength();           /* get the length also               */
    PadChar = optionalPadArgument(pad, ' ', ARG_TWO);/* get the pad character             */
    if (Length1 > Length2)
    {             /* first longer?                     */
        String1 = this->getStringData();   /* make arg 1 first string           */
                                           /* arg 2 is second string            */
        String2 = string2->getStringData();
        Lead = Length2;                    /* get shorter length                */
        Remainder = Length1 - Lead;        /* get trailing size                 */
    }
    else
    {                               /* make arg 2 first string           */
        String1 = string2->getStringData();
        String2 = this->getStringData();   /* arg 1 is second string            */
        Lead = Length1;                    /* get shorter length                */
        Remainder = Length2 - Lead;        /* get trailing size                 */
    }
    MisMatch = 0;                        /* assume they are equal             */
    i = 0;                               /* set the start                     */
    while (i < Lead)
    {                   /* if have leading compare           */
        if (String1[i] != String2[i])
        {    /* not the same?                     */
            MisMatch = i + 1;                /* save position, origin one         */
            break;                           /* exit the loop                     */
        }
        i++;                               /* step the index                    */
    }
    if (!MisMatch && Remainder)
    {        /* need to handle padding?           */
        String1 += Lead;                   /* step to remainder                 */
        for (i = 0; i < Remainder; i++)
        {  /* scan the remainder                */
            if (String1[i] != PadChar)
            {     /* pad mismatch?                     */
                MisMatch = Lead + i + 1;       /* get mismatch position             */
                break;                         /* finished                          */
            }
        }
    }
    if (!MisMatch)
    {
        Retval = IntegerZero;              /* this is zero                      */
    }
    else
    {
        Retval = new_integer(MisMatch);    /* make an integer return value      */
    }
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
    other = stringArgument(other, ARG_ONE);
    stringsize_t length2 = other->getLength();       /* get the length also               */
    // we uppercase the pad character now since this is caseless
    char padChar = toupper(optionalPadArgument(pad, ' ', ARG_TWO));

    const char *string1;
    const char *string2;
    stringsize_t lead;
    stringsize_t _remainder;

    // is the first longer?
    if (length1 > length2)
    {
        string1 = this->getStringData();   /* make arg 1 first string           */
                                           /* arg 2 is second string            */
        string2 = other->getStringData();
        lead = length2;                    /* get shorter length                */
        _remainder = length1 - lead;        /* get trailing size                 */
    }
    else
    {
        string1 = other->getStringData();    /* make arg 2 first string           */
        string2 = this->getStringData();     /* arg 1 is second string            */
        lead = length1;                      /* get shorter length                */
        _remainder = length2 - lead;          /* get trailing size                 */
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
    for (i = 0; i < _remainder; i++)
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


RexxString *RexxString::copies(RexxInteger *_copies)
/******************************************************************************/
/* Function:  String class COPIES method/function                             */
/******************************************************************************/
{
    size_t   Count;                      /* copies count                      */
    RexxString *Retval;                  /* return value                      */
    size_t   Len;                        /* copy string length                */
    char    *Temp;                       /* copy location                     */

    requiredArgument(_copies, ARG_ONE);           /* the count is required             */
    /* get the copies count              */
    Count = _copies->requiredNonNegative(ARG_ONE);
    Len = this->getLength();                  /* get argument length               */

    if (Count == 0 ||                    /* no copies requested?              */
        Len == 0 )                       /* or copying a null string          */
    {
        Retval = OREF_NULLSTRING;          /* just a null string                */
    }
    else
    {                               /* get storage size                  */
                                    /* allocate storage needed           */
                                    /* allocate storage needed           */
        Retval = (RexxString *)raw_string(Len * Count);

        if (Len == 1)
        {                    /* if only 1 char long               */
                             /* just do this with memset          */
            memset(Retval->getWritableData(), this->getChar(0), Count);
        }
        /* if any copies                     */
        else
        {
            /* point to the string               */
            Temp = Retval->getWritableData();
            while (Count--)
            {                /* copy 2 thru n copies              */
                             /* copy the string                   */
                memcpy(Temp, this->getStringData(), Len);
                Temp += Len;
            }
        }
    }
    return Retval;                       /* return copied string              */
}

RexxObject *RexxString::dataType(RexxString *pType)
/******************************************************************************/
/* Function:  String class DATATYPE method/function                           */
/******************************************************************************/
{
    if (pType != OREF_NULL)
    {             /* see if type was specified?        */
                  /* yes, specified, get 1st char      */
        int type = optionalOptionArgument(pType, 0, ARG_ONE);
        /* and call datatype routine to      */
        return StringUtil::dataType(this, type);    /* determine if its type specified.  */
    }
    /* type not specified, see if its a  */
    /* valid number                      */
    return(StringUtil::dataType(this, 'N') == TheTrueObject
           ? new_string("NUM",3)         /* if so we return NUM               */
           : new_string("CHAR",4));      /* otherwise we return CHAR          */
}


/**
 * Do a lastpos() search for a string.
 *
 * @param needle The search needle.
 * @param _start The starting position.
 *
 * @return the offset of the match position (origin 1).  Returns 0
 *         if no match was found.
 */
RexxInteger *RexxString::lastPosRexx(RexxString  *needle, RexxInteger *_start, RexxInteger *_range)
{
    return StringUtil::lastPosRexx(getStringData(), getLength(), needle, _start, _range);
}


/**
 * Rexx exported version of the caselessLastPos() method.
 *
 * @param needle The search needle.
 * @param _start The starting position.
 *
 * @return The match position.  0 means not found.
 */
RexxInteger *RexxString::caselessLastPosRexx(RexxString  *needle, RexxInteger *_start, RexxInteger *_range)
{
    // validate that this is a good string argument
    needle = stringArgument(needle, ARG_ONE);
    // find out where to start the search. The default is at the very end.
    size_t startPos = optionalPositionArgument(_start, getLength(), ARG_TWO);
    size_t range = optionalLengthArgument(_range, getLength(), ARG_THREE);
    // now perform the actual search.
    return new_integer(StringUtil::caselessLastPos(getStringData(), getLength(), needle, startPos, range));
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
size_t RexxString::lastPos(RexxString  *needle, size_t _start)
{
    return StringUtil::lastPos(getStringData(), getLength(), needle, _start, getLength());
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
size_t RexxString::caselessLastPos(RexxString *needle, size_t _start)
{
    return StringUtil::caselessLastPos(getStringData(), getLength(), needle, _start, getLength());
}

RexxInteger *RexxString::countStrRexx(RexxString *needle)
/******************************************************************************/
/* Function:  Count occurrences of one string in another.                     */
/******************************************************************************/
{
    /* force needle to a string          */
    needle = stringArgument(needle, ARG_ONE);
    // delegate the counting to the string util
    return new_integer(StringUtil::countStr(getStringData(), getLength(), needle));
}


RexxInteger *RexxString::caselessCountStrRexx(RexxString *needle)
/******************************************************************************/
/* Function:  Count occurrences of one string in another.                     */
/******************************************************************************/
{
    /* force needle to a string          */
    needle = stringArgument(needle, ARG_ONE);
    // delegate the counting to the string util
    return new_integer(StringUtil::caselessCountStr(getStringData(), getLength(), needle));
}

RexxString *RexxString::changeStr(RexxString *needle, RexxString *newNeedle, RexxInteger *countArg)
/******************************************************************************/
/* Function:  Change strings into another string.                             */
/******************************************************************************/
{
    size_t _start;                       /* converted start position          */
    size_t matchPos;                     /* last match position               */
    size_t needleLength;                 /* length of the needle              */
    size_t newLength;                    /* length of the replacement string  */
    size_t matches;                      /* number of replacements            */
    size_t copyLength;                   /* length to copy                    */
    const char *source;                  /* point to the string source        */
    char *copyPtr;                       /* current copy position             */
    const char *newPtr;                  /* pointer to replacement data       */
    RexxString *result;                  /* returned result string            */
    size_t i;

    /* force needle to a string          */
    needle = stringArgument(needle, ARG_ONE);
    /* newneedle must be a string two    */
    newNeedle = stringArgument(newNeedle, ARG_TWO);

    // we'll only change up to a specified count.  If not there, we do everything.
    size_t count = optionalPositive(countArg, Numerics::MAX_WHOLENUMBER, ARG_THREE);
    matches = StringUtil::countStr(getStringData(), getLength(), needle);    /* find the number of replacements   */
    if (matches > count)                 // the matches are bounded by the count
    {
        matches = count;
    }
    needleLength = needle->getLength();  /* get the length of the needle      */
    newLength = newNeedle->getLength();  /* and the replacement length        */
                                         /* get a proper sized string         */
    result = (RexxString *)raw_string(this->getLength() - (matches * needleLength) + (matches * newLength));
    copyPtr = result->getWritableData(); /* point to the copy location        */
    source = this->getStringData();      /* and out own data                  */
                                         /* and the string to replace         */
    newPtr = newNeedle->getStringData();
    _start = 0;                          /* set a zero starting point         */
    for (i = 0; i < matches; i++)
    {      /* until we hit count or run out     */
        matchPos = pos(needle, _start);  /* look for the next occurrence      */
        if (matchPos == 0)               /* not found?                        */
        {
            break;                           /* get out of here                   */
        }
        copyLength = (matchPos - 1) - _start;  /* get the next length to copy       */
        if (copyLength != 0)
        {             /* something to copy?                */
                      /* add on the next string section    */
            memcpy(copyPtr, source + _start, copyLength);
            copyPtr += copyLength;           /* step over the copied part         */
        }
        if (newLength != 0)
        {              /* something to replace with?        */
            memcpy(copyPtr, newPtr, newLength); /* copy over the new segment         */
            copyPtr += newLength;            /* and step it over also             */
        }
        _start = matchPos + needleLength - 1;  /* step to the next position         */
    }
    if (_start < this->getLength())      /* some remainder left?              */
    {
        /* add it on                         */
        memcpy(copyPtr, source + _start, this->getLength() - _start);
    }
    return result;                       /* finished                          */
}

RexxString *RexxString::caselessChangeStr(RexxString *needle, RexxString *newNeedle, RexxInteger *countArg)
/******************************************************************************/
/* Function:  Change strings into another string.                             */
/******************************************************************************/
{
    size_t _start;                       /* converted start position          */
    size_t matchPos;                     /* last match position               */
    size_t needleLength;                 /* length of the needle              */
    size_t newLength;                    /* length of the replacement string  */
    size_t matches;                      /* number of replacements            */
    size_t copyLength;                   /* length to copy                    */
    const char *source;                  /* point to the string source        */
    char * copyPtr;                      /* current copy position             */
    const char *newPtr;                  /* pointer to replacement data       */
    RexxString *result;                  /* returned result string            */
    size_t i;

    /* force needle to a string          */
    needle = stringArgument(needle, ARG_ONE);
    /* newneedle must be a string two    */
    newNeedle = stringArgument(newNeedle, ARG_TWO);
    // we'll only change up to a specified count.  If not there, we do everything.
    size_t count = optionalPositive(countArg, Numerics::MAX_WHOLENUMBER, ARG_THREE);

    matches = StringUtil::caselessCountStr(getStringData(), getLength(), needle);    /* find the number of replacements   */
    if (matches > count)                 // the matches are bounded by the count
    {
        matches = count;
    }
    needleLength = needle->getLength();       /* get the length of the needle      */
    newLength = newNeedle->getLength();       /* and the replacement length        */
    /* get a proper sized string         */
    result = (RexxString *)raw_string(this->getLength() - (matches * needleLength) + (matches * newLength));
    copyPtr = result->getWritableData();    /* point to the copy location        */
    source = this->getStringData();      /* and out own data                  */
                                         /* and the string to replace         */
    newPtr = newNeedle->getStringData();
    _start = 0;                           /* set a zero starting point         */
    for (i = 0; i < matches; i++)
    {      /* until we hit count or run out     */
        matchPos = this->caselessPos(needle, _start);  /* look for the next occurrence      */
        if (matchPos == 0)                    /* not found?                        */
        {
            break;                           /* get out of here                   */
        }
        copyLength = (matchPos - 1) - _start;  /* get the next length to copy       */
        if (copyLength != 0)
        {             /* something to copy?                */
                      /* add on the next string section    */
            memcpy(copyPtr, source + _start, copyLength);
            copyPtr += copyLength;              /* step over the copied part         */
        }
        if (newLength != 0)
        {              /* something to replace with?        */
            memcpy(copyPtr, newPtr, newLength); /* copy over the new segment         */
            copyPtr += newLength;               /* and step it over also             */
        }
        _start = matchPos + needleLength - 1;  /* step to the next position         */
    }
    if (_start < this->getLength())            /* some remainder left?              */
    {
        /* add it on                         */
        memcpy(copyPtr, source + _start, this->getLength() - _start);
    }
    return result;                       /* finished                          */
}


RexxInteger *RexxString::posRexx(RexxString  *needle, RexxInteger *pstart, RexxInteger *range)
/******************************************************************************/
/* Function:  String class POS method/function                                */
/******************************************************************************/
{
    return StringUtil::posRexx(getStringData(), getLength(), needle, pstart, range);
}


/**
 * Do a caseless search for one string in another.
 *
 * @param needle The search string.
 * @param pstart The starting position for the search.
 * @param range  A maximum range for the search.
 *
 * @return The index of any match position, or 0 if not found.
 */
RexxInteger *RexxString::caselessPosRexx(RexxString *needle, RexxInteger *pstart, RexxInteger *range)
{
    /* force needle to a string          */
    needle = stringArgument(needle, ARG_ONE);
    /* get the starting position         */
    size_t _start = optionalPositionArgument(pstart, 1, ARG_TWO);
    size_t _range = optionalLengthArgument(range, getLength() - _start + 1, ARG_THREE);
    /* pass on to the primitive function */
    /* and return as an integer object   */
    return new_integer(StringUtil::caselessPos(getStringData(), getLength(), needle , _start - 1, _range));
}


/**
 * Do a primitive level pos() search on a string.
 *
 * @param needle The search needle.
 * @param _start The starting position (origin 0)
 *
 * @return The match position (origin 1).  Returns 0 for no match.
 */
size_t RexxString::pos(RexxString *needle, size_t _start)
{
    return StringUtil::pos(getStringData(), getLength(), needle, _start, getLength());
}


/**
 * Do a primitive level pos() search on a string.
 *
 * @param needle The search needle.
 * @param _start The starting position (origin 0)
 *
 * @return The match position (origin 1).  Returns 0 for no match.
 */
size_t RexxString::caselessPos(RexxString *needle, size_t _start)
{
    return StringUtil::caselessPos(getStringData(), getLength(), needle, _start, getLength());
}


RexxString *RexxString::translate(
    RexxString *tableo,                /* output table                      */
    RexxString *tablei,                /* input table                       */
    RexxString *pad,                   /* pad character                     */
    RexxInteger *_start,               // start position to translate
    RexxInteger *_range)               // length to translate
/******************************************************************************/
/*  Function:  String class TRANSLATE method/function                         */
/******************************************************************************/
{
    RexxString *Retval;                  /* return value                      */
    const char *OutTable;                /* output table                      */
    size_t    OutTableLength;            /* length of output table            */
    const char *InTable;                 /* input table                       */
    char       *ScanPtr;                 /* scanning pointer                  */
    size_t    ScanLength;                /* scanning length                   */
    size_t    InTableLength;             /* length of input table             */
    char      PadChar;                   /* pad character                     */
    char      ch;                        /* current character                 */
    size_t    Position;                  /* table position                    */

                                         /* just a simple uppercase?          */
    if (tableo == OREF_NULL && tablei == OREF_NULL && pad == OREF_NULL)
    {
        return this->upperRexx(_start, _range);   /* return the uppercase version      */
    }
                                            /* validate the tables               */
                                            /* validate the tables               */
    tableo = optionalStringArgument(tableo, OREF_NULLSTRING, ARG_ONE);
    OutTableLength = tableo->getLength();      /* get the table length              */
    /* input table too                   */
    tablei = optionalStringArgument(tablei, OREF_NULLSTRING, ARG_TWO);
    InTableLength = tablei->getLength();       /* get the table length              */
    InTable = tablei->getStringData();    /* point at the input table          */
    OutTable = tableo->getStringData();   /* and the output table              */
                                          /* get the pad character             */
    PadChar = optionalPadArgument(pad, ' ', ARG_THREE);
    size_t startPos = optionalPositionArgument(_start, 1, ARG_FOUR);
    size_t range = optionalLengthArgument(_range, getLength() - startPos + 1, ARG_FOUR);

    // if nothing to translate, we can return now
    if (startPos > getLength() || range == 0)
    {
        return this;
    }
    // cap the real range
    range = Numerics::minVal(range, getLength() - startPos + 1);

    /* allocate space for answer         */
    /* and copy the string               */
    Retval = new_string(this->getStringData(), this->getLength());
    ScanPtr = Retval->getWritableData() + startPos - 1;  /* point to data                     */
    ScanLength = range;                        /* get the length too                */

    while (ScanLength--)
    {                /* spin thru input                   */
        ch = *ScanPtr;                      /* get a character                   */

        if (tablei != OREF_NULLSTRING)      /* input table specified?            */
        {
            /* search for the character          */
            Position = StringUtil::memPos(InTable, InTableLength, ch);
        }
        else
        {
            Position = ((size_t)ch) & 0xFF;     /* position is the character value   */
        }
        if (Position != (size_t)(-1))
        {     /* found in the table?               */
            if (Position < OutTableLength)    /* in the output table?              */
            {
                /* convert the character             */
                *ScanPtr = *(OutTable + Position);
            }
            else
            {
                *ScanPtr = PadChar;             /* else use the pad character        */
            }
        }
        ScanPtr++;                          /* step the pointer                  */
    }
    return Retval;                        /* return translated string          */
}


RexxInteger *RexxString::verify(
    RexxString  *ref,                  /* compare reference string          */
    RexxString  *option,               /* Match/NoMatch option              */
    RexxInteger *_start,               /* optional starg position           */
    RexxInteger *range)                // length to search
/******************************************************************************/
/*  Function:  String class VERIFY function                                   */
/******************************************************************************/
{
    return StringUtil::verify(getStringData(), getLength(), ref, option, _start, range);
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
    stringsize_t _start = positionArgument(start_, ARG_ONE);
    // the start position must be within the string bounds
    if (_start > getLength())
    {
        reportException(Error_Incorrect_method_position, start_);
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

    return primitiveMatch(_start, other, offset, len) ? TheTrueObject : TheFalseObject;
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
    stringsize_t _start = positionArgument(start_, ARG_ONE);
    // the start position must be within the string bounds
    if (_start > getLength())
    {
        reportException(Error_Incorrect_method_position, start_);
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

    return primitiveCaselessMatch(_start, other, offset, len) ? TheTrueObject : TheFalseObject;
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
bool RexxString::primitiveMatch(stringsize_t _start, RexxString *other, stringsize_t offset, stringsize_t len)
{
    _start--;      // make the starting point origin zero
    offset--;

    // if the match is not possible in the target string, just return false now.
    if ((_start + len) > getLength())
    {
        return false;
    }

    return memcmp(getStringData() + _start, other->getStringData() + offset, len) == 0;
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
bool RexxString::primitiveCaselessMatch(stringsize_t _start, RexxString *other, stringsize_t offset, stringsize_t len)
{
    _start--;      // make the starting point origin zero
    offset--;

    // if the match is not possible in the target string, just return false now.
    if ((_start + len) > getLength())
    {
        return false;
    }

    return StringUtil::caselessCompare(getStringData() + _start, other->getStringData() + offset, len) == 0;
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

    stringsize_t _setLength = matchSet->getLength();
    char         _matchChar = getChar(position - 1);

    // iterate through the match set looking for a match
    for (stringsize_t i = 0; i < _setLength; i++)
    {
        if (_matchChar == matchSet->getChar(i))
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

    stringsize_t _setLength = matchSet->getLength();
    char         _matchChar = getChar(position - 1);
    _matchChar = toupper(_matchChar);

    // iterate through the match set looking for a match, using a
    // caseless compare
    for (stringsize_t i = 0; i < _setLength; i++)
    {
        if (_matchChar == toupper(matchSet->getChar(i)))
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

    stringsize_t _start = optionalPositionArgument(start_, 1, ARG_TWO);
    stringsize_t len = optionalLengthArgument(len_, Numerics::maxVal(getLength(), other->getLength()) - _start + 1, ARG_THREE);

    return primitiveCompareTo(other, _start, len);
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
RexxInteger *RexxString::primitiveCompareTo(RexxString *other, stringsize_t _start, stringsize_t len)
{
    stringsize_t myLength = getLength();
    stringsize_t otherLength = other->getLength();

    // if doing the compare outside of the string length, we're less than the other string
    // unless the start is
    if (_start > myLength)
    {
        return _start > otherLength ? IntegerZero : IntegerMinusOne;
    }
    // if beyond the other length, they we're the larger
    if (_start > otherLength)
    {
        return IntegerOne;
    }

    _start--;      // make the starting point origin zero

    myLength = Numerics::minVal(len, myLength - _start);
    otherLength = Numerics::minVal(len, otherLength - _start);

    len = Numerics::minVal(myLength, otherLength);

    wholenumber_t result = memcmp(getStringData() + _start, other->getStringData() + _start, len);

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

    stringsize_t _start = optionalPositionArgument(start_, 1, ARG_TWO);
    stringsize_t len = optionalLengthArgument(len_, Numerics::maxVal(getLength(), other->getLength()) - _start + 1, ARG_THREE);

    return primitiveCaselessCompareTo(other, _start, len);
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
RexxInteger *RexxString::primitiveCaselessCompareTo(RexxString *other, stringsize_t _start, stringsize_t len)
{
    stringsize_t myLength = getLength();
    stringsize_t otherLength = other->getLength();

    // if doing the compare outside of the string length, we're less than the other string
    // unless the start is
    if (_start > myLength)
    {
        return _start > otherLength ? IntegerZero : IntegerMinusOne;
    }
    // if beyond the other length, they we're the larger
    if (_start > otherLength)
    {
        return IntegerOne;
    }

    _start--;      // make the starting point origin zero

    myLength = Numerics::minVal(len, myLength - _start);
    otherLength = Numerics::minVal(len, otherLength - _start);

    len = Numerics::minVal(myLength, otherLength);

    wholenumber_t result = StringUtil::caselessCompare(getStringData() + _start, other->getStringData() + _start, len);

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
