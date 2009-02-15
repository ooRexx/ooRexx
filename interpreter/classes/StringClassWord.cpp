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
/* Word-related REXX string methods                                           */
/*                                                                            */
/******************************************************************************/

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "StringUtil.hpp"


/* the DELWORD function */
/******************************************************************************/
/* Arguments:  nth word to start deleting                                     */
/*             number of words to delete                                      */
/*                                                                            */
/*  Returned:  string, with length words deleted                              */
/******************************************************************************/
RexxString *RexxString::delWord(RexxInteger *position,
                                RexxInteger *plength)
{
    char       *Current;                 /* current pointer position          */
    const char *Word;                    /* current word pointer              */
    const char *NextSite;                /* next word                         */
    size_t      WordPos;                 /* needed word position              */
    size_t      Count;                   /* count of words                    */
    size_t      Length;                  /* remaining length                  */
    size_t      WordLength;              /* word size                         */
    size_t      FrontLength;             /* front substring                   */
    RexxString *Retval;                  /* return value                      */

                                         /* convert position to binary        */
    WordPos = positionArgument(position, ARG_ONE);
    /* get num of words to delete, the   */
    /* default is "a very large number"  */
    Count = optionalLengthArgument(plength, Numerics::MAX_WHOLENUMBER, ARG_TWO);

    Length = this->getLength();               /* get string length                 */
    if (!Length)                         /* null string?                      */
    {
        Retval = OREF_NULLSTRING;          /* result is null also               */
    }
    else if (!Count)                     /* deleting zero words?              */
    {
        Retval = this;                     /* just use this string              */
    }
    else
    {
        Word = this->getStringData();      /* point to the string               */
                                           /* get the first word                */
        WordLength = StringUtil::nextWord(&Word, &Length, &NextSite);
        while (--WordPos && WordLength)
        {  /* loop until we reach tArget        */
            Word = NextSite;                 /* copy the start pointer            */
                                             /* get the next word                 */
            WordLength = StringUtil::nextWord(&Word, &Length, &NextSite);
        }
        if (WordPos)                       /* run out of words first            */
        {
            Retval = this;                   /* return entire string              */
        }
        else
        {                             /* count off number of words         */
                                      /* calculate front length            */
            FrontLength = (size_t)(Word - this->getStringData());
            while (--Count && WordLength)
            {  /* loop until we reach tArget        */
                Word = NextSite;               /* copy the start pointer            */
                                               /* get the next word                 */
                WordLength = StringUtil::nextWord(&Word, &Length, &NextSite);
            }
            if (Length)                      /* didn't use up the string          */
            {
                StringUtil::skipBlanks(&NextSite, &Length);/* skip over trailing blanks         */
            }
                                               /* allocate return string            */
            Retval = raw_string(FrontLength + Length);
            /* point to data portion             */
            Current = Retval->getWritableData();
            if (FrontLength)
            {               /* have a leading portion?           */
                            /* copy into the result              */
                memcpy(Current, this->getStringData(), FrontLength);
                Current += FrontLength;        /* step output position              */
            }
            if (Length)                      /* any string left?                  */
            {
                /* copy what's left                  */
                memcpy(Current, NextSite, Length);
            }
        }
    }
    return Retval;                       /* return deleted string             */
}

/* the SPACE function */
/******************************************************************************/
/* Arguments:  number of pad characters between each word                     */
/*             pad character                                                  */
/*                                                                            */
/*  Returned:  string                                                         */
/******************************************************************************/
RexxString *RexxString::space(RexxInteger *space_count,
                              RexxString  *pad)
{
    size_t      Spaces;                  /* requested spacing                 */
    char        PadChar;                 /* pad character                     */
    char       *Current;                 /* current pointer position          */
    const char *Word;                    /* current word pointer              */
    const char *NextSite;                /* next word                         */
    size_t      Count;                   /* count of words                    */
    size_t      WordSize;                /* size of words                     */
    size_t      Length;                  /* remaining length                  */
    size_t      WordLength;              /* word size                         */
    RexxString *Retval;                  /* return value                      */

                                         /* get the spacing count             */
    Spaces = optionalLengthArgument(space_count, 1, ARG_ONE);

    /* get the pad character             */
    PadChar = optionalPadArgument(pad, ' ', ARG_TWO);

    Length = this->getLength();               /* get the string length             */
    Count = 0;                           /* no words yet                      */
    WordSize = 0;                        /* no characters either              */
    Word = this->getStringData();        /* point to the string               */
                                         /* get the first word                */
    WordLength = StringUtil::nextWord(&Word, &Length, &NextSite);

    while (WordLength)
    {                 /* loop until we reach tArget        */
        Count++;                           /* count the word                    */
        WordSize += WordLength;            /* add in the word length            */
        Word = NextSite;                   /* copy the start pointer            */
                                           /* get the next word                 */
        WordLength = StringUtil::nextWord(&Word, &Length, &NextSite);
    }
    if (!Count)                          /* no words?                         */
    {
        Retval = OREF_NULLSTRING;          /* this is a null string             */
    }
    else
    {                               /* real words                        */
        Count--;                           /* step back one                     */
                                           /* get space for output              */
        Retval = raw_string(WordSize + Count * Spaces);
        /* point to output area              */
        Current = Retval->getWritableData();

        Length = this->getLength();             /* recover the length                */
        Word = this->getStringData();      /* point to the string               */
                                           /* get the first word                */
        WordLength = StringUtil::nextWord(&Word, &Length, &NextSite);

        while (Count--)
        {                  /* loop for each word                */
                           /* copy the word over                */
            memcpy(Current, Word, WordLength);
            Current += WordLength;           /* step over the word                */
            if (Spaces)
            {                    /* if have gaps...                   */
                                 /* fill in the pad chars             */
                memset(Current, PadChar, Spaces);
                Current += Spaces;             /* step over the pad chars           */
            }
            Word = NextSite;                 /* copy the start pointer            */
                                             /* get the next word                 */
            WordLength = StringUtil::nextWord(&Word, &Length, &NextSite);
        }
        /* copy the word over                */
        memcpy(Current, Word, (size_t)WordLength);
    }
    return Retval;                       /* return spaced string              */
}


/* the SUBWORD function */
/******************************************************************************/
/* Arguments:  Starting word postion                                          */
/*             number of words                                                */
/*                                                                            */
/*  Returned:  string, contains the requested number of words from source     */
/******************************************************************************/
RexxString *RexxString::subWord(RexxInteger *position, RexxInteger *plength)
{
    return StringUtil::subWord(getStringData(), getLength(), position, plength);
}


/* the WORD function */
/******************************************************************************/
/* Arguments:  which word we want.                                            */
/*                                                                            */
/*  Returned:  string, containing nth word.                                   */
/******************************************************************************/
RexxString *RexxString::word(RexxInteger *position)
{
    return StringUtil::word(getStringData(), getLength(), position);
}

/* the WORDINDEX function */
/******************************************************************************/
/* Arguments:  word we want position of.                                      */
/*                                                                            */
/*  Returned:  integer,  actual char position of nth word                     */
/******************************************************************************/
RexxInteger *RexxString::wordIndex(RexxInteger *position)
{
    return StringUtil::wordIndex(getStringData(), getLength(), position);
}

/* the WORDLENGTH function */
/******************************************************************************/
/* Arguments:  nth word we want length of                                     */
/*                                                                            */
/*  Returned:  integer, length of nth word                                    */
/******************************************************************************/
RexxInteger *RexxString::wordLength(RexxInteger *position)
{
    return StringUtil::wordLength(getStringData(), getLength(), position);
}


/**
 * Perform a wordpos search on a string object.
 *
 * @param phrase The search phrase
 * @param pstart The starting search position.
 *
 * @return The index of the match location.
 */
RexxInteger *RexxString::wordPos(RexxString  *phrase, RexxInteger *pstart)
{
    return StringUtil::wordPos(getStringData(), getLength(), phrase, pstart);
}


/**
 * Perform a caseless wordpos search on a string object.
 *
 * @param phrase The search phrase
 * @param pstart The starting search position.
 *
 * @return The index of the match location.
 */
RexxInteger *RexxString::caselessWordPos(RexxString  *phrase, RexxInteger *pstart)
{
    return StringUtil::caselessWordPos(getStringData(), getLength(), phrase, pstart);
}

/* the WORDS function */
/******************************************************************************/
/* Arguments:  none                                                           */
/*                                                                            */
/*  Returned:  integer, number os words in source                             */
/******************************************************************************/
RexxInteger *RexxString::words()
{
    size_t tempCount = StringUtil::wordCount(this->getStringData(), this->getLength());
    return new_integer(tempCount);
}


