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
/* Word-related REXX string methods                                           */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "StringUtil.hpp"
#include "MethodArguments.hpp"


/**
 * Delete a word from a given postion.
 *
 * @param position The target word position.
 * @param plength  The number of words to delete.
 *
 * @return The string with the range of words deleted.
 */
RexxString *RexxString::delWord(RexxInteger *position, RexxInteger *plength)
{
    size_t wordPos = positionArgument(position, ARG_ONE);
    // get the number of words to delete...the default is "a very large number"
    size_t count = optionalLengthArgument(plength, Numerics::MAX_WHOLENUMBER, ARG_TWO);

    // can't take something from nothing.  If we start with a nullstring, we
    // finish with a null string.
    if (isNullString())
    {
        return GlobalNames::NULLSTRING;
    }
    // if the count of words to delete is zero, then return
    // the target unchanged.
    if (count == 0)
    {
        return this;
    }

    // create an iterator for traversing the words
    WordIterator iterator(this);

    // to the given word position...if we don't get there,
    // there is nothing to delete so we can just return the
    // original string.
    if (!iterator.skipWords(wordPos))
    {
        return this;
    }

    // we now know the leading portion of the result string
    size_t frontLength = iterator.wordPointer() - getStringData();

    // if we managed to locate the desired number of words, then skip
    // over the trailing blanks to the next word or the end of the string.  Note
    // that we are positioned at the first word already, so we skip one fewer.
    if (iterator.skipWords(count - 1))
    {
        iterator.skipBlanks();
    }

    // we know know the leading and trailing string lengths, so we can allocate
    // the result string
    RexxString *retval = raw_string(frontLength + iterator.length());
    StringBuilder builder(retval);

    // add on the two string sections
    builder.append(getStringData(), frontLength);
    // the iterator knows what the remainder is.
    iterator.appendRemainder(builder);
    return retval;
}


/**
 * The String space method.
 *
 * @param space_count
 *               The number of spaces to insert between words.
 * @param pad    The character to use for the word spacing.
 *
 * @return The original string with the words separated by the new space count.
 */
RexxString *RexxString::space(RexxInteger *space_count, RexxString *pad)
{
    size_t spaces = optionalLengthArgument(space_count, 1, ARG_ONE);
    char padChar = optionalPadArgument(pad, ' ', ARG_TWO);

    size_t sourceLength = getLength();
    size_t count = 0;
    // this is the total size consumed by the words...used to
    // calculate the result string size
    size_t wordSize = 0;

    // get an iterator to count up the number of words and get the sizes
    WordIterator counter(this);

    while (counter.next())
    {
        count++;
        wordSize += counter.wordLength();
    }

    // no words found?  The result is a null string
    if (count == 0)
    {
        return GlobalNames::NULLSTRING;
    }

    // count is now the number of inter-word gaps in the result
    count--;

    RexxString *retval = raw_string(wordSize + count * spaces);
    StringBuilder builder(retval);

    // get a fresh iterator for building the string
    WordIterator iterator(this);

    // we know the number of words, so we can just traverse through this
    while (count--)
    {
        iterator.next();
        // add the current word to the result
        iterator.append(builder);
        // add the interword spacing
        builder.pad(padChar, spaces);
    }

    // one last word to append
    iterator.next();
    iterator.append(builder);
    return retval;
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


/**
 * Returns an array of all words contained in the given range
 * of the string, using the same extraction rules used
 * for subWord() and word().
 *
 * @param position The optional starting position.  If not provided, extraction
 *                 starts with the first word.
 * @param plength  The number of words to extract.  If omitted, will extract
 *                 from the starting postion to the end of the string.
 *
 * @return An array containing the extracted words.  If no words are
 *         available within the given range, this returns an empty
 *         array.
 */
ArrayClass *RexxString::subWords(RexxInteger *position, RexxInteger *plength)
{
    return StringUtil::subWords(getStringData(), getLength(), position, plength);
}


/**
 * Retrieve a specific word from a string.
 *
 * @param position The target word position.
 *
 * @return The word at the given postion, or a null string if that
 *         word is not found.
 */
RexxString *RexxString::word(RexxInteger *position)
{
    return StringUtil::word(getStringData(), getLength(), position);
}


/**
 * Retrieve string position of a given word within a string.
 *
 * @param position The target word position.
 *
 * @return The index of the target word, or zero if the string
 *         does not contain that many words.
 */
RexxInteger *RexxString::wordIndex(RexxInteger *position)
{
    return StringUtil::wordIndex(getStringData(), getLength(), position);
}


/**
 * Retrieve the length of a given word within a string.
 *
 * @param position The target word position.
 *
 * @return The length of the word at that position, or zero of the string
 *         does not contain that many words.
 */
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
    return new_integer(StringUtil::wordPos(getStringData(), getLength(), phrase, pstart));
}


/**
 * Test if a string contains a given phrase
 *
 * @param phrase The search phrase
 * @param pstart The starting search position.
 *
 * @return The index of the match location.
 */
RexxObject *RexxString::containsWord(RexxString  *phrase, RexxInteger *pstart)
{
    return booleanObject(StringUtil::wordPos(getStringData(), getLength(), phrase, pstart) > 0);
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
    return new_integer(StringUtil::caselessWordPos(getStringData(), getLength(), phrase, pstart));
}


/**
 * Perform a caseless wordpos search on a string object.
 *
 * @param phrase The search phrase
 * @param pstart The starting search position.
 *
 * @return The index of the match location.
 */
RexxObject *RexxString::caselessContainsWord(RexxString  *phrase, RexxInteger *pstart)
{
    return booleanObject(StringUtil::caselessWordPos(getStringData(), getLength(), phrase, pstart)  > 0);
}


/**
 * Return the count of words within the string.
 *
 * @return The total number of words in the string.
 */
RexxInteger *RexxString::words()
{
    size_t tempCount = StringUtil::wordCount(getStringData(), getLength());
    return new_integer(tempCount);
}


