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
#include "RexxBuiltinFunctions.h"
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
    WordPos = get_position(position, ARG_ONE);
    /* get num of words to delete, the   */
    /* default is "a very large number"  */
    Count = optional_length(plength, MAXNUM, ARG_TWO);

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
    Spaces = optional_length(space_count, 1, ARG_ONE);

    /* get the pad character             */
    PadChar = get_pad(pad, ' ', ARG_TWO);

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
RexxString *RexxString::subWord(RexxInteger *position,
                                RexxInteger *plength)
{
    const char *Word;                    /* current word pointer              */
    const char *WordStart;               /* start of substring                */
    const char *WordEnd;                 /* end of the substring              */
    const char *NextSite;                /* next word                         */
    size_t      WordPos;                 /* needed word position              */
    size_t      Count;                   /* count of words                    */
    size_t      Length;                  /* remaining length                  */
    size_t      WordLength;              /* word size                         */
    RexxString *Retval;                  /* return value                      */

                                         /* convert position to binary        */
    WordPos = get_position(position, ARG_ONE);
    /* get num of words to delete, the   */
    /* default is "a very large number"  */
    Count = optional_length(plength, MAXNUM, ARG_TWO);

    Length = this->getLength();               /* get Argument length               */
    if (!Length || !Count)               /* null string?                      */
    {
        Retval = OREF_NULLSTRING;          /* result is null also               */
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
            Retval = OREF_NULLSTRING;        /* again a null string               */
        }
        else
        {                             /* count off number of words         */
            WordStart = Word;                /* save start position               */
            WordEnd = Word;                  /* default end is the same           */
                                             /* loop until we reach tArget        */
            while (Count-- && WordLength)
            {
                WordEnd = Word + WordLength;   /* point to the word end             */
                Word = NextSite;               /* copy the start pointer            */
                                               /* get the next word                 */
                WordLength = StringUtil::nextWord(&Word, &Length, &NextSite);
            }
            /* extract the substring             */
            Retval = (RexxString *)new_string(WordStart, WordEnd - WordStart);
        }
    }
    return Retval;                       /* return extracted string           */
}

/* the WORD function */
/******************************************************************************/
/* Arguments:  which word we want.                                            */
/*                                                                            */
/*  Returned:  string, containing nth word.                                   */
/******************************************************************************/
RexxString *RexxString::word(RexxInteger *position)
{
    const char *Word;                    /* current word pointer              */
    const char *NextSite;                /* next word                         */
    size_t      WordPos;                 /* needed word position              */
    size_t      Length;                  /* remaining length                  */
    size_t      WordLength;              /* word size                         */
    RexxString *Retval;                  /* return value                      */

                                         /* convert position to binary        */
    WordPos = get_position(position, ARG_ONE);

    Length = this->getLength();          /* get Argument length               */
    if (Length == 0)                     /* null string?                      */
    {
        Retval = OREF_NULLSTRING;          /* result is null also               */
    }
    else
    {
        Word = this->getStringData();      /* point to the string               */
                                           /* get the first word                */
        WordLength = StringUtil::nextWord(&Word, &Length, &NextSite);
        while (--WordPos && WordLength)
        {  /* loop until we reach target        */
            Word = NextSite;                 /* copy the start pointer            */
                                             /* get the next word                 */
            WordLength = StringUtil::nextWord(&Word, &Length, &NextSite);
        }
        if (WordLength)                    /* have a word                       */
        {
            /* extract the string                */
            Retval = (RexxString *)new_string(Word, WordLength);
        }
        else
        {
            Retval = OREF_NULLSTRING;        /* no word, return a null            */
        }
    }
    return Retval;                       /* return extracted string           */
}

/* the WORDINDEX function */
/******************************************************************************/
/* Arguments:  word we want position of.                                      */
/*                                                                            */
/*  Returned:  integer,  actual char position of nth word                     */
/******************************************************************************/
RexxInteger *RexxString::wordIndex(RexxInteger *position)
{
    const char *Word;                    /* current word pointer              */
    const char *NextSite;                /* next word                         */
    size_t      Length;                  /* string length                     */
    size_t      WordLength;              /* word length                       */
    size_t      WordPos;                 /* desired word position             */
    RexxInteger *Retval;                 /* return value                      */
    size_t      tempIndex;

    Length = this->getLength();          /* get the string length             */
                                         /* convert count to binary           */
    WordPos = get_position(position, ARG_ONE);
    Word = this->getStringData();        /* point to word data                */

                                         /* get the first word                */
    WordLength = StringUtil::nextWord(&Word, &Length, &NextSite);
    while (--WordPos && WordLength)
    {    /* loop until we reach target        */
        Word = NextSite;                   /* copy the start pointer            */
                                           /* get the next word                 */
        WordLength = StringUtil::nextWord(&Word, &Length, &NextSite);
    }
    if (WordLength == 0)                 /* ran out of string                 */
    {
        Retval = IntegerZero;              /* no index                          */
    }
    else
    {                               /* calc the word index               */
        tempIndex = Word - this->getStringData() + 1;
        Retval = new_integer(tempIndex);
    }
    return Retval;                       /* return appopriate position        */
}

/* the WORDLENGTH function */
/******************************************************************************/
/* Arguments:  nth word we want length of                                     */
/*                                                                            */
/*  Returned:  integer, length of nth word                                    */
/******************************************************************************/
RexxInteger *RexxString::wordLength(RexxInteger *position)
{
    const char *Word;                    /* current word pointer              */
    const char *NextSite;                /* next word                         */
    size_t      Length;                  /* string length                     */
    size_t      WordLength;              /* word length                       */
    size_t      WordPos;                 /* desired word position             */

    Length = this->getLength();               /* get the string length             */
    /* convert count to binary           */
    WordPos = get_position(position , ARG_ONE);
    Word = this->getStringData();             /* point to word data                */

    /* get the first word                */
    WordLength = StringUtil::nextWord(&Word, &Length, &NextSite);
    while (--WordPos && WordLength)
    {    /* loop until we reach target        */
        Word = NextSite;                   /* copy the start pointer            */
                                           /* get the next word                 */
        WordLength = StringUtil::nextWord(&Word, &Length, &NextSite);
    }
    return new_integer(WordLength);      /* return the word length            */
}

/* the WORDPOS function */
/******************************************************************************/
/* Arguments:  phrase of words to lookfor                                     */
/*             which word to satrt looking at.                                */
/*                                                                            */
/*  Returned:  position of 1st word in source matching phrase                 */
/******************************************************************************/
RexxInteger *RexxString::wordPos(RexxString  *phrase,
                                 RexxInteger *pstart)
{
    RexxInteger *Retval;                 /* return value                      */
    const char *Needle;                  /* start of needle string            */
    const char *Haystack;                /* current haystack positon          */
    const char *NextNeedle;              /* next search position              */
    const char *NextHaystack;            /* next search position              */
    size_t   Count;                      /* current haystack word pos         */
    size_t   NeedleWords;                /* needle word count                 */
    size_t   HaystackWords;              /* haystack word count               */
    size_t   HaystackScanLength;         /* length to scan                    */
    size_t   NeedleScanLength;           /* length of scan                    */
    size_t   HaystackWordLength;         /* haystack word length              */
    size_t   NeedleWordLength;           /* needle word length                */
    size_t   SearchCount;                /* possible searches to do           */
    size_t   FirstNeedle;                /* length of first needle word       */
    size_t   NeedleLength;               /* length of needle                  */
    size_t   HaystackLength;             /* length of haystack                */
    const char *NeedlePosition;          /* temporary pointers for            */
    const char *HaystackPosition;        /* the searches                      */
    const char *NextHaystackPtr;         /* pointer to next word              */
    const char *NextNeedlePtr;           /* pointer to next word              */
    size_t   i;                          /* loop counter                      */

    phrase = get_string(phrase, ARG_ONE);/* get the phrase we are looking for */
    NeedleLength = phrase->getLength();       /* get the length also               */
    /* get starting position, the default*/
    /* is the first word                 */
    Count = optional_position(pstart, 1, ARG_TWO);

    Needle = phrase->getStringData();    /* get friendly pointer              */
    Haystack = this->getStringData();    /* and the second also               */
    HaystackLength = this->getLength();  /* get the haystack length           */
                                         /* count the words in needle         */
    NeedleWords = StringUtil::wordCount(Needle, NeedleLength);
    /* and haystack                      */
    HaystackWords = StringUtil::wordCount(Haystack, HaystackLength);
    /* if search string is longer        */
    /* or no words in search             */
    /* or count is longer than           */
    /* haystack                          */
    if (NeedleWords > (HaystackWords - Count + 1) ||
        NeedleWords == 0 ||
        Count > HaystackWords)
    {
        Retval = IntegerZero;              /* can't find anything,              */
    }
    else
    {                               /* need to search                    */
                                    /* point at first word               */
        HaystackWordLength = StringUtil::nextWord(&Haystack, &HaystackLength, &NextHaystack);
        /* now skip over count-1             */
        for (i = Count - 1; i && HaystackWordLength != 0; i--)
        {
            Haystack = NextHaystack;         /* step past current word            */
                                             /* find the next word                */
            HaystackWordLength = StringUtil::nextWord(&Haystack, &HaystackLength, &NextHaystack);
        }
        /* get number of searches            */
        SearchCount = (HaystackWords - NeedleWords - Count) + 2;
        /* position at first needle          */
        FirstNeedle = StringUtil::nextWord(&Needle, &NeedleLength, &NextNeedle);
        /* loop for the possible times       */
        for (; SearchCount; SearchCount--)
        {
            NeedleWordLength = FirstNeedle;  /* set the length                    */
            NeedlePosition = Needle;         /* get the start of phrase           */
            HaystackPosition = Haystack;     /* and the target string loop        */
                                             /* for needlewords                   */
            NextHaystackPtr = NextHaystack;  /* copy nextword information         */
            NextNeedlePtr = NextNeedle;
            /* including the lengths             */
            HaystackScanLength = HaystackLength;
            NeedleScanLength = NeedleLength;

            for (i = NeedleWords; i; i--)
            {  /* possible times                    */

                if (HaystackWordLength !=      /* if wrong length, then it          */
                    NeedleWordLength)          /* can't be a match...just           */
                {
                    break;                       /* leave the loop                    */
                }

                if (memcmp(NeedlePosition,     /* if the two words don't            */
                           HaystackPosition,          /* match, then no sense              */
                           NeedleWordLength))         /* checking the rest                 */
                {
                    break;                       /* get out fast.                     */
                }

                                                 /* the last words matched, so        */
                                                 /* continue searching.               */

                                                 /* set new search information        */
                HaystackPosition = NextHaystackPtr;
                NeedlePosition = NextNeedlePtr;
                /* Scan off the next word            */
                HaystackWordLength = StringUtil::nextWord(&HaystackPosition,
                                              &HaystackScanLength,
                                              &NextHaystackPtr);
                /* repeat for the needle             */
                NeedleWordLength = StringUtil::nextWord(&NeedlePosition,
                                            &NeedleScanLength,
                                            &NextNeedlePtr);
            }

            if (!i)                          /* all words matched, we             */
            {
                break;                         /* found it                          */
            }
            Haystack = NextHaystack;         /* set the search position           */
                                             /* step to next haytack pos          */
            HaystackWordLength = StringUtil::nextWord(&Haystack,
                                          &HaystackLength,
                                          &NextHaystack);
            Count++;                         /* remember the word position        */
        }

        if (SearchCount != 0)              /* if we haven't scanned the         */
        {
            /* entire string                     */
            Retval = new_integer(Count);     /* return the position               */
        }
        else                               /* it wasn't found, just             */
        {
            Retval = IntegerZero;            /* return a zero.                    */
        }
    }
    return Retval;                       /* return the position               */
}


RexxInteger *RexxString::caselessWordPos(RexxString  *phrase, RexxInteger *pstart)
{
    phrase = get_string(phrase, ARG_ONE);/* get the phrase we are looking for */
    stringsize_t needleLength = phrase->getLength();       /* get the length also               */
                                         /* get starting position, the default*/
                                         /* is the first word                 */
    stringsize_t count = optional_position(pstart, 1, ARG_TWO);

    const char *needle = phrase->getStringData();  /* get friendly pointer              */
    const char *haystack = this->getStringData();  /* and the second also               */
    stringsize_t haystackLength = this->getLength();  /* get the haystack length           */
                                                 /* count the words in needle         */
    stringsize_t needleWords = StringUtil::wordCount(needle, needleLength);
                                         /* and haystack                      */
    stringsize_t haystackWords = StringUtil::wordCount(haystack, haystackLength);
                                         /* if search string is longer        */
                                         /* or no words in search             */
                                         /* or count is longer than           */
                                         /* haystack, this is a failure       */
    if (needleWords > (haystackWords - count + 1) || needleWords == 0 || count > haystackWords)
    {
        return IntegerZero;
    }

    const char *nextHaystack;
    const char *nextNeedle;
                                       /* point at first word               */
    stringsize_t haystackWordLength = StringUtil::nextWord(&haystack, &haystackLength, &nextHaystack);
                                       /* now skip over count-1             */
    for (stringsize_t i = count - 1; i && haystackWordLength != 0; i--)
    {
        haystack = nextHaystack;         /* step past current word            */
                                       /* find the next word                */
        haystackWordLength = StringUtil::nextWord(&haystack, &haystackLength, &nextHaystack);
    }
                                       /* get number of searches            */
    stringsize_t searchCount = (haystackWords - needleWords - count) + 2;
                                       /* position at first needle          */
    stringsize_t firstNeedle = StringUtil::nextWord(&needle, &needleLength, &nextNeedle);
                                       /* loop for the possible times       */
    for (; searchCount; searchCount--)
    {
        stringsize_t needleWordLength = firstNeedle;   /* set the length                    */
        const char *needlePosition = needle;         /* get the start of phrase           */
        const char *haystackPosition = haystack;     /* and the target string loop        */
                                         /* for needlewords                   */
        const char *nextHaystackPtr = nextHaystack;  /* copy nextword information         */
        const char *nextNeedlePtr = nextNeedle;
                                         /* including the lengths             */
        stringsize_t haystackScanLength = haystackLength;
        stringsize_t needleScanLength = needleLength;

        stringsize_t i;

        for (i = needleWords; i; i--)
        {
            // length mismatch, can't be a match

            if (haystackWordLength != needleWordLength)
            {
                break;
            }

            // now compare the two words, using a caseless comparison
            // if the words don't match, terminate now
            if (StringUtil::caselessCompare(needlePosition, haystackPosition, needleWordLength))
            {
                break;                       /* get out fast.                     */
            }

                                           /* the last words matched, so        */
                                           /* continue searching.               */

                                           /* set new search information        */
            haystackPosition = nextHaystackPtr;
            needlePosition = nextNeedlePtr;
                                           /* Scan off the next word            */
            haystackWordLength = StringUtil::nextWord(&haystackPosition, &haystackScanLength, &nextHaystackPtr);
                                           /* repeat for the needle             */
            needleWordLength = StringUtil::nextWord(&needlePosition, &needleScanLength, &nextNeedlePtr);
        }

        if (i == 0)                      /* all words matched, we             */
        {
            return new_integer(count);   // return the position
        }
        haystack = nextHaystack;         /* set the search position           */
                                         /* step to next haytack pos          */
        haystackWordLength = StringUtil::nextWord(&haystack, &haystackLength, &nextHaystack);
        count++;                         /* remember the word position        */
    }

    return IntegerZero;                // not found
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


