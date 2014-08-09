/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                                         */
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
/* substring oriented REXX string methods                                     */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "ActivityManager.hpp"
#include "StringUtil.hpp"
#include "MethodArguments.hpp"


/**
 * The center/centre method for the string class.
 *
 * @param _length The length to center in.
 * @param pad     The optional pad character
 *
 * @return The centered string.
 */
RexxString *RexxString::center(RexxInteger *_length, RexxString *pad)
{
    char     PadChar;                    /* pad character                     */
    size_t   LeftPad;                    /* required left pads                */
    size_t   RightPad;                   /* required right pads               */
    size_t   Space;                      /* result string size                */
    size_t   Width;                      /* centering width                   */
    size_t   Len;                        /* string length                     */
    RexxString *Retval;                  /* return string                     */

    // this will be our final length
    size_t width = lengthArgument(_length, ARG_ONE);

    // the pad character is optional, with a blank default
    char padChar = optionalPadArgument(pad, ' ', ARG_TWO);
    size_t len = getLength();
    // if the width and the length are the same, we just return the target
    // string unchanged
    if (width == len)
    {
        return this;
    }

    // centered in zero characters?  This is a null string
    if (width == 0)
    {
        return GlobalNames::NULLSTRING;
    }

    // if the width is bigger than the length, we need to add pad characters
    if (width > len)
    {
        // half the pad is on the left side
        size_t leftPad = (width - len) / 2;
        // the remainder on the right, which also gets the extra if an odd number
        // is required
        size_t rightPad = (width - len) - leftPad;
        RexxString *retval = raw_string(width);
        // set left pad characters
        memset(retval->getWritableData(), padChar, leftPad);
        // copy the string data
        if (len > 0)
        {
            memcpy(retval->getWritableData() + leftPad, getStringData(), len);
        }
        // now the trailing pad chars
        memset(retval->getWritableData() + leftPad + len, padChar, rightPad);
        return retval;
    }
    // the request width is smaller than the input, so we have to truncate
    else
    {
        // we really only need to calculate the left side truncation
        size_t leftPad = (len - width) / 2;
        return new_string(getStringData() + leftPad, width);
    }
}


/**
 * The delstr() method of the string class.
 *
 * @param position The starting postion
 * @param _length  The length to delete.
 *
 * @return The string after the deletion.
 */
RexxString *RexxString::delstr(RexxInteger *position, RexxInteger *_length)
{
    size_t stringLen = getLength();
    size_t deletePos = positionArgument(position, ARG_ONE);
    size_t deleteLen = optionalLengthArgument(_length, stringLen - deletePos + 1, ARG_TWO);

    // if the delete position is beyond the
    // length of the string, just return unchanged
    if (deletePos > stringLen)
    {
        return this;
    }

    // easier to do if origin zero
    deletePos--;

    size_t backLen = 0;
    // if we're not deleting up to or past the end, calculate the
    // size of the trailing section
    if (deleteLen < (stringLen - deletePos))
    {
        backLen = stringLen - (deletePos + deleteLen);
    }

    RexxString *retval = raw_string(deletePos + backLen);

    char *current = retval->getWritableData();
    // copy any leading part, unless we're deleting from the
    // start of the string
    if (deletePos > 0)
    {

        memcpy(current, getStringData(), deletePos);
        current += deletePos;
    }
    // and if we have a trailing section, copy that too
    if (backLen)
    {
        memcpy(current, getStringData() + deletePos + deleteLen, backLen);
    }
    return retval;
}


/**
 * The String insert method.
 *
 * @param newStrObj The string to insert.
 * @param position  The insert postion.
 * @param _length   The optional insert length.
 * @param pad       The optional padding character.
 *
 * @return The string with the new string inserted.
 */
RexxString *RexxString::insert(RexxString  *newStrObj, RexxInteger *position, RexxInteger *_length, RexxString  *pad)
{
    size_t targetLength = getLength();

    newStrObj = stringArgument(newStrObj, ARG_ONE);
    size_t newStringLength = newStrObj->getLength();

    // we're parsing this as a length argument because a postion of zero
    // is valid for insert
    size_t insertPosition = optionalLengthArgument(position, 0, ARG_TWO);
    size_t insertLength = optionalLengthArgument(_length, newStringLength, ARG_THREE);

    // default pad character is a blank
    char padChar = optionalPadArgument(pad, ' ', ARG_FOUR);
    size_t leadPad = 0;
    size_t frontLength;
    size_t backLength;

    // inserting at the beginning?  No leading pad required, no leading part
    // to copy, the back length is the entire string
    if (insertPosition == 0)
    {
        leadPad = 0;
        frontLength = 0;
        backLength = targetLength;
    }
    // insert position beyond the length?  We might need to insert some leading
    // pad characters, the front part is everything and there is no back part to copy
    else if (position >= targetLength)
    {
        leadPad = position - targetLength;
        frontLength = targetLength;
        backLength = 0;
    }

    // we're inserting in the middle.  No leading pad, the insert position
    // determines the length of the front and back sections
    else
    {
        leadPad = 0;
        frontLength = position;
        backLength = targetLength - position;
    }

    // we might need to truncate the inserted string if the specified length is
    // shorter than the inserted string
    newStringLength = Numerics::minVal(newStringLength, insertLength);
    size_t padLength = insertLength - newStringLength;

    size_t resultLength = targetLength + insertLength + leadPad;
    RexxString *retval = raw_string(resultLength);
    char *current = retval->getWritableData();

    // if we have a front length, copy it now
    if (frontLength != 0)
    {
        memcpy(current, getStringData(), frontLength);
        current += frontLength;
    }

    // if there are leading pad characters required, add them now
    if (leadPad != 0)
    {
        memset(current, padChar, leadPad);
        current += leadPad;
    }

    // if we have new string data to copy, this is next
    if (newStringLength != 0)
    {

        memcpy(current, newStr->getStringData(), newStringLength);
        current += newStringLength;
    }

    // and trailing pad required?
    if (padLength != 0)
    {
        memset(current, padChar, padLength);
        current += padLength;
    }

    // and the back length value
    if (backLength != 0)
    {
        memcpy(current, getStringData() + frontLength, backLength);
    }
    return retval;
}


/* the LEFT function */
/******************************************************************************/
/* Arguments:  String len, string pad character                               */
/*                                                                            */
/*  Returned:  string                                                         */
/******************************************************************************/
RexxString *RexxString::left(RexxInteger *_length,
                             RexxString  *pad)
{
    char      PadChar;                   /* pad character                     */
    size_t    Size;                      /* requested size                    */
    size_t    Length;                    /* string length                     */
    RexxString *Retval;                  /* returned result                   */
    char *    Current;                   /* current copy location             */
    size_t    CopyLength;                /* length to copy                    */

                                         /* get the target length             */
    Size = lengthArgument(_length, ARG_ONE);

    /*  is used if omitted.              */
    PadChar = optionalPadArgument(pad, ' ', ARG_TWO);
    Length = this->getLength();               /* get input length                  */

    if (!Size)                           /* requesting zero bytes?            */
    {
        Retval = GlobalNames::NULLSTRING;          /* return a null string              */
    }
    else
    {
        Retval = raw_string(Size);         /* allocate a result string          */
        CopyLength = Numerics::minVal(Length, Size);    /* adjust the length                 */
        /* point to data part                */
        Current = Retval->getWritableData();
        if (CopyLength)
        {                  /* have real data?                   */
                           /* copy it                           */
            memcpy(Current, this->getStringData(), CopyLength);
            Current += CopyLength;           /* bump the pointer                  */
        }
        if (Size > Length)                 /* need to pad?                      */
        {
            /* pad the string                    */
            memset(Current, PadChar, Size - Length);
        }
    }
    return Retval;                       /* return string piece               */
}

/******************************************************************************/
/* Function:  Process the OVERLAY function/method                             */
/******************************************************************************/
RexxString *RexxString::overlay(
    RexxString  *newStrObj,            /* overlayed string                  */
    RexxInteger *position,             /* overlay position                  */
    RexxInteger *_length,               /* overlay length                    */
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
    char     PadChar;                    /* pad character                     */
    char    *Current;                    /* current copy location             */

    TargetLen = this->getLength();       /* get the haystack length           */
                                         /* get the overlay string value      */
    newStr = stringArgument(newStrObj, ARG_ONE);
    NewLen = newStr->getLength();
    /* get the overlay position          */
    OverlayPos = optionalPositionArgument(position, 1, ARG_TWO);
    /* get final overlay length          */
    OverlayLen = optionalLengthArgument(_length, NewLen, ARG_THREE);
    /*  is used if omitted.              */
    PadChar = optionalPadArgument(pad, ' ', ARG_FOUR);

    if (OverlayLen > NewLen)             /* need to pad?                      */
        BackPad = OverlayLen - NewLen;     /* get the pad size                  */
    else
    {                               /* need to truncate                  */
        NewLen = OverlayLen;               /* used specified length             */
        BackPad = 0;                       /* no back padding                   */
    }

    if (OverlayPos > TargetLen)
    {        /* overlaying past the end?          */
             /* get front padding                 */
        FrontPad = OverlayPos - TargetLen - 1;
        FrontLen = TargetLen;              /* copy entire string                */
    }
    else
    {                               /* overlay is within bounds          */
        FrontPad = 0;                      /* no padding here                   */
        FrontLen = OverlayPos - 1;         /* just copy the front part          */
    }
    /* fall off the back side?           */
    if (OverlayPos + OverlayLen > TargetLen)
    {
        BackLen = 0;                       /* no back part                      */
    }
    else
    {
        /* calculate the back part           */
        BackLen = TargetLen - (OverlayPos + OverlayLen - 1);
    }
    /* allocate result string            */
    Retval = raw_string(FrontLen + BackLen + FrontPad + OverlayLen);

    Current = Retval->getWritableData(); /* get copy location                 */

    if (FrontLen)
    {                      /* something in front?               */
                           /* copy the front part               */
        memcpy(Current, this->getStringData(), FrontLen);
        Current += FrontLen;               /* step the pointer                  */
    }

    if (FrontPad)
    {                      /* padded in front?                  */
        memset(Current, PadChar, FrontPad);/* set the pad characters            */
        Current += FrontPad;               /* step the pointer                  */
    }

    if (NewLen)
    {                        /* non-null new string?              */
                             /* copy the string                   */
        memcpy(Current, newStr->getStringData(), NewLen);
        Current += NewLen;                 /* step the pointer                  */
    }

    if (BackPad)
    {                       /* padded in back?                   */
                            /* set the pad characters            */
        memset(Current, PadChar, BackPad);
        Current += BackPad;                /* step the pointer                  */
    }

    if (BackLen)
    {                       /* trailing part?                    */
                            /* copy the string                   */
        memcpy(Current, this->getStringData() + OverlayPos + OverlayLen - 1, BackLen);
    }
    return Retval;                       /* return new string                 */
}


/**
 * Replace a substring starting at a given position and
 * length with another string.  This operation is essentially
 * a delstr() followed by an insert() operation.
 *
 * @param newStrObj The replacement string
 * @param position  The replacement position (required)
 * @param _length   The length of string to replace (optional).  If omitted,
 *                  the length of the replacement string is used and this
 *                  is essentially an overlay operation.
 * @param pad       The padding character if padding is required.  The default
 *                  is a ' '
 *
 * @return A new instance of the string with the value replace.
 */
RexxString *RexxString::replaceAt(RexxString  *newStrObj, RexxInteger *position, RexxInteger *_length, RexxString  *pad)
{
    size_t targetLen = this->getLength();   // get the length of the replacement target
    // the replacement value is required and must be a string
    RexxString *newStr = stringArgument(newStrObj, ARG_ONE);
    // the length of the replacement string is the default replacement length
    size_t newLen = newStr->getLength();
    // the overlay position is required
    size_t replacePos = positionArgument(position, ARG_TWO);
    // the replacement length is optional, and defaults to the length of the replacement string
    size_t replaceLen = optionalLengthArgument(_length, newLen, ARG_THREE);
    // we only pad if the start position is past the end of the string
    char padChar = optionalPadArgument(pad, ' ', ARG_FOUR);
    size_t padding = 0;
    size_t frontLen = 0;
    size_t backLen = 0;
    // the only time we need to pad is if the replacement position is past the
    // end of the string
    if (replacePos > targetLen)
    {
        padding = replacePos - targetLen - 1;
        frontLen = targetLen;
    }
    else
    {
        // this is within bounds, so we copy up to that position
        frontLen = replacePos - 1;
    }
    // is this within the bounds of the string?
    if (replacePos + replaceLen - 1 < targetLen)
    {
        // calculate the back part we need to copy
        backLen = targetLen - (replacePos + replaceLen - 1);
    }
    // allocate a result string
    RexxString *retval = raw_string(frontLen + backLen + padding + newLen);
    // and get a copy location
    char *current = retval->getWritableData();

    if (frontLen > 0)
    {                      /* something in front?               */
                           /* copy the front part               */
        memcpy(current, this->getStringData(), frontLen);
        current += frontLen;               /* step the pointer                  */
    }
    // padding only happens if we've copy the entire front portion
    if (padding > 0)
    {
        memset(current, padChar, padding);
        current += padding;
    }
    // replace with a non-null string?  copy into the current position
    if (newLen > 0)
    {
        memcpy(current, newStr->getStringData(), newLen);
        current += newLen;
    }
    // the remainder, if there is any, get's copied after the
    // replacement string with no padding
    if (backLen > 0)
    {
        memcpy(current, this->getStringData() + replacePos + replaceLen - 1, backLen);
    }
    return retval;
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
    char      *String;                   /* current location                */
    const char *End;                      /* string end position             */

    Length = this->getLength();               /* get first argument              */
    if (Length)
    {                        /* if really data                  */
        Retval = raw_string(Length);       /* get result storage              */
                                           /* get new string pointer          */
        String = Retval->getWritableData();
        /* point to end of original        */
        End = this->getStringData() + Length - 1;

        while (Length--)                   /* reverse entire string           */
        {
            *String++ = *End--;              /* copy a single char              */
        }
                                             /* done building the string          */
    }
    else                                 /* if null input                     */
    {
        Retval = GlobalNames::NULLSTRING;          /* return null output                */
    }
    return Retval;                       /* return the reversed string        */
}

/* the RIGHT function */
/******************************************************************************/
/* Arguments:  length of result                                               */
/*             pad character to use if needed.                                */
/*                                                                            */
/*  Returned:  string right justified.                                        */
/******************************************************************************/
RexxString *RexxString::right(RexxInteger *_length,
                              RexxString  *pad)
{
    char      PadChar;                   /* pad character                     */
    size_t    Size;                      /* requested size                    */
    size_t    Length;                    /* string length                     */
    RexxString *Retval;                  /* returned result                   */
    char *    Current;                   /* current copy location             */
    size_t    CopyLength;                /* length to copy                    */

                                         /* get the target length             */
    Size = lengthArgument(_length, ARG_ONE);

    /*  is used if omitted.              */
    PadChar = optionalPadArgument(pad, ' ', ARG_TWO);
    Length = this->getLength();               /* get input length                  */

    if (!Size)                           /* requesting zero bytes?            */
    {
        /* return a null string              */
        Retval = GlobalNames::NULLSTRING;
    }
    else
    {
        Retval = raw_string(Size);         /* allocate a result string          */
        CopyLength = Numerics::minVal(Length, Size);    /* adjust the length                 */
        /* point to data part                */
        Current = Retval->getWritableData();
        if (Size > Length)
        {               /* need to pad?                      */
                        /* pad the string                    */
            memset(Current, PadChar, Size - Length);
            Current += Size - Length;        /* bump the pointer                  */
        }
        if (CopyLength)                    /* have real data?                   */
        {
            /* copy it                           */
            memcpy(Current, this->getStringData() + Length - CopyLength, CopyLength);
        }
    }
    return Retval;                       /* return string piece               */
}

/**
 * Strip a set of leading and/or trailing characters from
 * a string, returning a new string value.
 *
 * @param option    The option indicating which characters to strip.
 * @param stripchar The set of characters to strip.
 *
 * @return A new string instance, with the target characters removed.
 */
RexxString *RexxString::strip(RexxString *optionString, RexxString *stripchar)
{
    // get the option character
    char option = optionalOptionArgument(optionString, STRIP_BOTH, ARG_ONE);
    if (option != STRIP_TRAILING &&      /* must be a valid option            */
        option != STRIP_LEADING &&
        option != STRIP_BOTH )
    {
        reportException(Error_Incorrect_method_option, "BLT", option);
    }
    // get the strip character set.  The default is to remove spaces and
    // horizontal tabs
    stripchar = optionalStringArgument(stripchar, OREF_NULL, ARG_TWO);

    // the default is to strip whitespace characters
    const char *chars = stripchar == OREF_NULL ? " \t" : stripchar->getStringData();
    size_t charsLen = stripchar == OREF_NULL ? strlen(" \t") : stripchar->getLength();

    const char *front = this->getStringData();       /* point to string start             */
    size_t length = this->getLength();               /* get the length                    */

                                         /* need to strip leading?            */
    if (option == STRIP_LEADING || option == STRIP_BOTH)
    {
        // loop while more string or we don't find one of the stripped characters
        while (length > 0)
        {
            if (!StringUtil::matchCharacter(*front, chars, charsLen))
            {
                break;
            }
            front++;                         /* step the pointer                  */
            length--;                        /* reduce the length                 */
        }
    }

    // need to strip trailing?
    if (option == STRIP_TRAILING || option == STRIP_BOTH)
    {
        // point to the end and scan backwards now
        const char *back = front + length - 1;
        while (length > 0)
        {
            if (!StringUtil::matchCharacter(*back, chars, charsLen))
            {
                break;
            }
            back--;                          /* step the pointer back             */
            length--;                        /* reduce the length                 */
        }
    }

    // if there is anything left, extract the remaining part
    if (length > 0)
    {
        return new_string(front, length);
    }
    else
    {
        // null string, everything stripped away
        return GlobalNames::NULLSTRING;
    }
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
                               RexxInteger *_length,
                               RexxString  *pad)
{
    return StringUtil::substr(getStringData(), getLength(), position, _length, pad);
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
    return StringUtil::subchar(getStringData(), getLength(), positionArg);
}

