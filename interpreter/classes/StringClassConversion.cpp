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
/* REXX Kernel                                      StringClassConversion.cpp */
/*                                                                            */
/* REXX string conversion methods                                             */
/*                                                                            */
/******************************************************************************/
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "BufferClass.hpp"

#include "NumberStringMath.hpp"
#include "ActivityManager.hpp"
#include "StringUtil.hpp"


/**
 * Convert the character string into the same string with the
 * characters converted into a Base64 encoding.
 *
 * @return The string reformatted into a Base64 encoding.
 */
RexxString *RexxString::encodeBase64()
{
    size_t inc[3];
    int i;
    static const char cb64[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


    size_t inputLength = this->getLength();     /* get length of string              */
    if (inputLength == 0)               /* null string?                      */
    {
        return OREF_NULLSTRING;
    }
    /* figure out the output string length */
    size_t outputLength = (inputLength / 3) * 4;
    if (inputLength % 3 > 0)
    {
        outputLength += 4;
    }
    /* allocate output string */
    RexxString *retval = raw_string(outputLength);
    const char *source = this->getStringData();  /* point to converted string         */
    /* point to output area              */
    char *destination = retval->getWritableData();
    while (inputLength > 0)
    {              /* while more string                 */
        int buflen = 0;
        for (i = 0; i < 3; i++)
        {        /* get the next 3 characters         */
            if (inputLength)
            {             /*    from the input string          */
                inc[i] = *source & 0xff;
                inputLength--;
                source++;
                buflen++;
            }
            else
            {
                inc[i] = '\0';
            }
        }
        if (buflen) {
            /* now perform the base64 conversion to the next 4 output string chars */
            *destination = cb64[ inc[0] >> 2 ];
            destination++;
            *destination = cb64[ ((inc[0] & 0x03) << 4) | ((inc[1] & 0xf0) >> 4) ];
            destination++;
            *destination = (char) (buflen > 1 ? cb64[ ((inc[1] & 0x0f) << 2) | ((inc[2] & 0xc0) >> 6) ] : '=');
            destination++;
            *destination = (char) (buflen > 2 ? cb64[ inc[2] & 0x3f ] : '=');
            destination++;
        }
    }                                  /* done building the string          */
    return retval;                       /* return converted string           */
}


/**
 * Reverse the effect of an encodebase64 operation, converting
 * a string in Base64 format into a "normal" character string.
 *
 * @return The converted character string.
 */
RexxString *RexxString::decodeBase64()
{
    unsigned int i, j;
    static const char cb64[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


    size_t inputLength = this->getLength();     /* get length of string              */
    if (inputLength == 0)                    /* null string?                      */
    {
        return OREF_NULLSTRING;          // this encodes as a null string
    }
    if (inputLength % 4 > 0) {
        /* the input string is an invalid length */
        reportException(Error_Incorrect_method_invbase64);
    }
    const char *source = this->getStringData();
    /* figure out the output string length */
    size_t outputLength = (inputLength / 4) * 3;
    if (*(source + inputLength - 1) == '=')
    {
        outputLength--;
    }
    if (*(source + inputLength - 2) == '=')
    {
        outputLength--;
    }
    /* allocate output string */
    RexxString *retval = raw_string(outputLength);
    /* point to output area              */
    char *destination = retval->getWritableData();
    while (inputLength)
    {              /* while more string                 */
        for (i = 0; i < 4; i++)
        {
            for (j = 0; j < 64; j++)
            {
                if (*(cb64 + j) == *(source + i))
                {
                    break;
                }
            }
            // if we don't find the digit, this might be an invalid string.
            if (j == 64)
            {
                if (*(source + i) == '=' && inputLength <= 4) {
                    /* this means we are done building the output string */
                    break;
                }
                else {
                    /* we found an invalid char in the middle of the input string */
                    reportException(Error_Incorrect_method_invbase64);
                }
            }
            /* j is now the 6-bit value we need for building our output string */
            switch (i)
            {
                case 0:
                    *destination = (char)(j << 2);
                    break;
                case 1:
                    *destination |= (char)(j >> 4);
                    destination++;
                    *destination = (char)(j << 4);
                    break;
                case 2:
                    *destination |= (char)(j >> 2);
                    destination++;
                    *destination = (char)(j << 6);
                    break;
                case 3:
                    *destination |= (char)j;
                    destination++;
                    break;
                default: /* not really necessary but here anyway */
                    break;
            }
        }
        source += 4;
        inputLength -= 4;
    }                                  /* done building the string          */
    return retval;                       /* return converted string           */
}

RexxString *RexxString::c2x()
/******************************************************************************/
/* Function:  Process the string C2X method/function                          */
/******************************************************************************/
{
    size_t      InputLength;             /* length of converted string        */
    RexxString *Retval;                  /* return value                      */
    const char *Source;                  /* input string pointer              */
    char *      Destination;             /* output string pointer             */
    char        ch;                      /* current character                 */

    InputLength = this->getLength();          /* get length of string              */
    if (!InputLength)                    /* null string?                      */
    {
        Retval = OREF_NULLSTRING;          /* converts to a null string         */
    }
    else
    {                               /* real data to convert              */
                                    /* allocate output string            */
        Retval = raw_string(InputLength * 2);
        Source = this->getStringData();    /* point to converted string         */
                                           /* point to output area              */
        Destination = Retval->getWritableData();
        while (InputLength--)
        {            /* while more string                 */
            ch = *Source++;                  /* get next character                */
            /***********************************************************/
            /* get upper nibble after shifting out lower nibble and do */
            /* logical ANDING with F to convert to integer then convert*/
            /* to hex value and put it in destination                  */
            /***********************************************************/
            *Destination++ = IntToHexDigit((ch>>4) & 0xF);
            /***********************************************************/
            /* logical AND with F to convert lower nibble to integer   */
            /* then convert to hex value and put it in destination     */
            /***********************************************************/
            *Destination++ = IntToHexDigit(ch  & 0xF);
        }
    }
    return Retval;                       /* return converted string           */
}

RexxString *RexxString::d2c(RexxInteger *_length)
/******************************************************************************/
/* Function:  Process the string D2C method/function                          */
/******************************************************************************/
{
    /* convert to a numberstring         */
    RexxNumberString *numberstring = this->numberString();
    if (numberstring == OREF_NULL)       /* not a valid number?               */
    {
        /* report this                       */
        reportException(Error_Incorrect_method_d2c, this);
    }
    /* format as a string value          */
    return numberstring->d2xD2c(_length, true);
}

RexxString *RexxString::d2x(RexxInteger *_length)
/******************************************************************************/
/* Function:  Process the string D2X method/function                          */
/******************************************************************************/
{
    /* convert to a numberstring         */
    RexxNumberString *numberstring = this->numberString();
    if (numberstring == OREF_NULL)       /* not a valid number?               */
    {
        /* report this                       */
        reportException(Error_Incorrect_method_d2x, this);
    }
    /* format as a string value          */
    return numberstring->d2xD2c(_length, false);
}

RexxString *RexxString::x2c()
/******************************************************************************/
/* Function:  Process the string X2C method/function                          */
/******************************************************************************/
{
    size_t InputLength = this->getLength();          /* get length of string       */
    if (InputLength== 0)                    /* null string?               */
    {
        return OREF_NULLSTRING;          /* converts to a null string         */
    }
    else                                 /* real data to convert       */
    {
        /* try to pack the data       */
        return StringUtil::packHex(this->getStringData(), InputLength);
    }
}

RexxString *RexxString::x2d(RexxInteger *_length)
/******************************************************************************/
/* Function:  Process the string X2D method/function                          */
/******************************************************************************/
{
                                       /* forward to the common routine     */
    return this->x2dC2d(_length, false);
}


RexxString *RexxString::x2dC2d(RexxInteger *_length,
                               bool type )
/******************************************************************************/
/* Function:  Common X2D/X2C processing routine                               */
/******************************************************************************/
{
    size_t     ResultSize;               /* size of result string             */
    size_t     TempSize;                 /* temporary size value              */
    int        ch;                       /* addition character                */
    size_t     StringLength;             /* input string length               */
    char       *Scan;                    /* scan pointer                      */
    char       *HighDigit;               /* high digit position               */
    char *     Accumulator;              /* accumulator pointer               */
    bool       Negative;                 /* have a negative number            */
    RexxString *String;                  /* converted string                  */
    char       *StringPtr;               /* string value pointer              */
    size_t     BytePosition;             /* position of high byte             */
    size_t     NibblePosition;           /* position of high nibble           */
    size_t     DecLength;                /* length of accumulator             */
    size_t     TempLength;               /* length of accumulator             */
    RexxString *Retval;                  /* function return value             */
    RexxBuffer *Buffer;                  /* first math buffer                 */
    size_t     CurrentDigits;            /* current digits setting            */

    CurrentDigits = number_digits();     /* get the current digits setting    */
    StringLength = this->getLength();         /* get Argument string length        */
    /* get the target length             */
    ResultSize = optionalLengthArgument(_length, -1, ARG_ONE);
    if (!ResultSize)                     /* zero requested                    */
    {
        return(RexxString *)IntegerZero;  /* always returns zero               */
    }

    String = this;                       /* use this string directly          */
    StringPtr = this->getWritableData(); /* get a string pointer              */
    NibblePosition = 0;                  /* assume an even nibble number      */

    if (type == true)
    {                  /* dealing with character?           */
        if (_length == OREF_NULL)
        {        /* no size specified?                */
            Negative = false;                /* can't be negative                 */
            ResultSize = StringLength;       /* use entire string                 */
        }
        else
        {                             /* have to check for negative        */
            if (ResultSize > StringLength)   /* longer than string?               */
            {
                Negative = false;              /* can't be negative                 */
            }
            else
            {                           /* have to check sign bit            */
                                        /* step to byte position             */
                StringPtr += StringLength - ResultSize;
                StringLength = ResultSize;     /* adjust the size down              */

                if (*StringPtr & 0x80)
                {       /* first bit on?                     */
                    Negative = true;             /* this is a negative number         */
                                                 /* copy the string                   */
                    String = (RexxString *)this->copy();
                    /* point to the string               */
                    StringPtr = String->getWritableData() + this->getLength() - ResultSize;
                }
                else                           /* still a positive number           */
                {
                    Negative = false;            /* remember for later                */
                }
            }
        }
    }
    else
    {                               /* x2d function                      */
                                    /* pack the string                   */
        String = (RexxString *)StringUtil::packHex(StringPtr, StringLength);
        /* get the packed length             */
        StringLength = String->getLength();
        /* point to the packed data          */
        StringPtr = String->getWritableData();
        if (_length == OREF_NULL)
        {        /* no size specified?                */
            Negative = false;                /* can't be negative                 */
            ResultSize = StringLength;       /* use entire string                 */
        }
        else
        {                             /* have to check for negative        */

            BytePosition = ResultSize / 2;   /* Get position of sign bit          */
                                             /* get nibble position               */
            NibblePosition = ResultSize % 2;
            /* Get result size                   */
            ResultSize = (BytePosition + NibblePosition);
            if (ResultSize > StringLength)
            { /* longer than string?               */
                Negative = false;              /* can't be negative                 */
                NibblePosition = 0;            /* leave the high nibble alone       */
            }
            else
            {                           /* have to check sign bit            */
                                        /* step to byte position             */
                StringPtr += StringLength - ResultSize;
                StringLength = ResultSize;     /* adjust the size down              */

                if ((NibblePosition &&         /* odd number of nibbles             */
                     /* and low nibble negative?          */
                     *StringPtr & 0x08) ||
                    (!NibblePosition &&        /* or even number of nibbles         */
                     *StringPtr & 0x80))        /* and high nibble negative?         */
                {
                    Negative = true;             /* this is a negative number         */
                }
                else                           /* still a positive number           */
                {
                    Negative = false;            /* remember for later                */
                }
            }
        }
    }

    if (Negative)
    {                      /* need to negate string?            */
        Scan = StringPtr;                  /* copy the pointer                  */
        TempSize = StringLength;           /* copy the size                     */

        while (TempSize--)
        {               /* reverse each byte                 */
                        /* exclusive or with foxes           */
            *Scan = *Scan ^ 0xff;
            Scan++;                          /* step the pointer                  */
        }
        /* point to the first byte           */
        Scan = StringPtr + StringLength - 1;
        TempSize = StringLength;           /* copy the size                     */
        while (TempSize--)
        {               /* now add one to the number         */
            ch = (*Scan & 0xff);             /* get the character                 */
            ch++;                            /* increment                         */
            if (ch <= 0xff)
            {                /* no carry over?                    */
                *Scan = ch;                    /* set value back                    */
                break;                         /* we're finished                    */
            }
            else
            {                           /* carried out                       */
                *Scan = 0;                     /* this is zero now                  */
                Scan--;                        /* step back one pointer             */
            }
        }
    }
    if (NibblePosition)                  /* Odd number of nibbles?            */
    {
        *StringPtr &= 0x0f;                /* zero out the highest nibble       */
    }

    Scan = StringPtr;                    /* point to the string               */
                                         /* allocate a temp buffer            */
    Buffer = (RexxBuffer *)new_buffer(CurrentDigits + OVERFLOWSPACE + 1);
    /* set accumulator pointer           */
    Accumulator = Buffer->getData() + CurrentDigits + OVERFLOWSPACE;
    /* clear the buffer                  */
    memset(Buffer->getData(), '\0', CurrentDigits + OVERFLOWSPACE + 1);
    HighDigit = Accumulator - 1;         /* set initial high point            */

    while (StringLength--)
    {             /* while more digits                 */
        ch = *Scan++;                      /* get the character                 */
                                           /* add high order nibble             */
        HighDigit = RexxNumberString::addToBaseTen((ch & 0xf0) >> 4, Accumulator, HighDigit);
        /* multiply by 16                    */
        HighDigit = RexxNumberString::multiplyBaseTen(Accumulator, HighDigit);
        /* get accumulator length            */
        DecLength = (Accumulator - HighDigit);
        if (DecLength > CurrentDigits)
        {   /* grown too long?                   */
            if (type == true)                /* c2d version?                      */
            {
                reportException(Error_Incorrect_method_c2dbig, CurrentDigits);
            }
            else                             /* this is the x2d function          */
            {
                reportException(Error_Incorrect_method_x2dbig, CurrentDigits);
            }
        }
        /* add high order nibble             */
        HighDigit = RexxNumberString::addToBaseTen(ch & 0x0f, Accumulator, HighDigit);
        if (StringLength != 0)             /* not the last one?                 */
        {
            /* multiply by 16                    */
            HighDigit = RexxNumberString::multiplyBaseTen(Accumulator, HighDigit);
        }
        /* get accumulator length            */
        DecLength = (Accumulator - HighDigit);
        if (DecLength > CurrentDigits)
        {   /* grown too long?                   */
            if (type == true)                /* c2d version?                      */
            {
                reportException(Error_Incorrect_method_c2dbig, CurrentDigits);
            }
            else                             /* this is the x2d function          */
            {
                reportException(Error_Incorrect_method_x2dbig, CurrentDigits);
            }
        }
    }
    /* get accumulator length            */
    DecLength = (Accumulator - HighDigit);
    TempLength = DecLength;              /* copy the length                   */
    Scan = HighDigit + 1;                /* point to the first digit          */
    while (TempLength--)
    {               /* make into real digits again       */
                    /* add zero to each digit            */
        *Scan = *Scan + '0';
        Scan++;                            /* step the pointer                  */
    }

    ResultSize = DecLength;              /* get the result size               */
    if (Negative)                        /* negative number?                  */
    {
        ResultSize++;                      /* add in space for the sign         */
    }
    Retval = raw_string(ResultSize);     /* allocate output buffer            */
    Scan = Retval->getWritableData();    /* point to output location          */
    if (Negative)                        /* need a sign?                      */
    {
        *Scan++ = '-';                     /* add to the front                  */
    }
                                           /* copy in the number                */
    memcpy(Scan, Accumulator - DecLength + 1, DecLength);
    return Retval;                       /* return converted string           */
}

RexxString *RexxString::b2x()
/******************************************************************************/
/* Function:  Common B2X processing routine                                   */
/******************************************************************************/
{
    RexxString *Retval;                  /* function result                   */
    size_t   Bits;                       /* number of bits in string          */
    const char *Source;                     /* current source pointer            */
    char    *Destination;                /* destination pointer               */
    size_t   Excess;                     /* section boundary                  */
    char     Nibble[4];                  /* current nibble string             */
    size_t   Jump;                       /* string movement offset            */
    size_t   Length;                     /* total string length               */

    if (this->getLength() == 0)               /* null input, i.e. zerolength       */
    {
        Retval = OREF_NULLSTRING;          /* return null                       */
    }
    else
    {                               /* need to do conversion             */
                                    /* validate the string               */
        Bits = StringUtil::validateSet(this->getStringData(), this->getLength(), "01", 4, false);
        /* allocate space for result         */
        Retval = raw_string((Bits + 3) / 4);
        /* point to the data                 */
        Destination = Retval->getWritableData();
        Source = this->getStringData();    /* point to the source               */
        Length = this->getLength();        /* get the string length             */

        while (Bits > 0)
        {                 /* process the string                */
            Excess = Bits % 4;               /* calculate section size            */
            if (Excess == 0)                 /* zero is a multiple of 4           */
            {
                Excess = 4;                    /* so use 4                          */
            }
            else
            {
                memset(Nibble, '0', 4);        /* pad the nibble with zeroes        */
            }
            StringUtil::chGetSm(&Nibble[0] + (4 - Excess), Source, Length, Excess, "01", &Jump);
            /* pack into destination             */
            *Destination++ = StringUtil::packNibble(Nibble);
            Source += Jump;                  /* advance source pointer            */
            Length -= Jump;                  /* reduce remaining length           */
            Bits -= Excess;                  /* reduce remaining amount           */
        }
    }
    return Retval;                       /* return packed string              */
}

RexxString *RexxString::c2d(RexxInteger *_length)
/******************************************************************************/
/* Function:  Common C2D processing routine                                   */
/******************************************************************************/
{
                                       /* forward to the common routine     */
    return this->x2dC2d(_length, true);
}

RexxString *RexxString::x2b()
/******************************************************************************/
/* Function:  Common X2B processing routine                                   */
/******************************************************************************/
{
    RexxString *Retval;                  /* function result                   */
    size_t   Nibbles;                    /* nibbles in hex string             */
    const char *Source;                  /* current source pointer            */
    char    *Destination;                /* destination pointer               */
    char     Nibble[4];                  /* current nibble string             */
    char     ch;                         /* current string character          */
    int      Val;                        /* converted nible                   */

    if (this->getLength() == 0)               /* null input, i.e. zerolength       */
    {
        /* string                            */
        Retval = OREF_NULLSTRING;          /* return null                       */
    }
    else
    {                               /* have real data to pack            */
        Nibbles = StringUtil::validateSet(this->getStringData(), this->getLength(), "0123456789ABCDEFabcdef", 2, true);
        Retval = raw_string(Nibbles * 4);  /* allocate result string            */
                                           /* point to the data                 */
        Destination = Retval->getWritableData();
        Source = this->getStringData();    /* point to the source               */

        while (Nibbles > 0)
        {              /* while still string to pack        */
            ch = *Source++;                  /* get current char and bump         */
                                             /* pointer                           */
            if (ch != ch_SPACE)
            {            /* if not a filler space             */
                Val = StringUtil::hexDigitToInt(ch);       /* convert hex to int first          */
                StringUtil::unpackNibble(Val, Nibble);     /* then convert to binary            */
                                               /* digits                            */
                                               /* copy to the destination           */
                memcpy(Destination, Nibble, 4);
                Destination += 4;              /* bump destination pointer          */
                Nibbles--;                     /* Reduce nibbles count              */
            }
        }
    }
    return Retval;                       /* return the expanded string        */
}
