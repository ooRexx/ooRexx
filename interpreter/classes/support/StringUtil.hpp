/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2019 Rexx Language Association. All rights reserved.    */
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
/* String Utilities shared between String class and MutableBuffer class       */
/*                                                                            */
/******************************************************************************/
#ifndef Included_StringUtil
#define Included_StringUtil

#include <ctype.h>

class RexxInteger;
class ArrayClass;
class MutableBuffer;

class StringUtil
{
public:
    static RexxString *substr(const char *, size_t, RexxInteger *, RexxInteger *, RexxString *);
    static RexxString *substr(const char *, size_t, RexxInteger *, RexxInteger *);
    static RexxInteger *posRexx(const char *stringData, size_t length, RexxString *needle, RexxInteger *pstart, RexxInteger *range);
    static RexxObject *containsRexx(const char *stringData, size_t length, RexxString *needle, RexxInteger *pstart, RexxInteger *range);
    static size_t pos(const char *stringData, size_t haystack_length, RexxString *needle, size_t _start, size_t _range);
    static size_t caselessPos(const char *stringData, size_t haystack_length, RexxString *needle, size_t _start, size_t _range);
    static RexxInteger *lastPosRexx(const char *stringData, size_t haystackLen, RexxString  *needle, RexxInteger *_start, RexxInteger *_range);
    static size_t lastPos(const char *stringData, size_t hastackLen, RexxString  *needle, size_t _start, size_t _range);
    static const char *lastPos(const char *needle, size_t needleLen, const char *haystack, size_t haystackLen);
    static RexxString *subchar(const char *stringData, size_t stringLength, RexxInteger *positionArg);
    static ArrayClass *makearray(const char *start, size_t length, RexxString *separator);
    static size_t caselessLastPos(const char *stringData, size_t hastackLen, RexxString  *needle, size_t _start, size_t range);
    static const char *caselessLastPos(const char *needle, size_t needleLen, const char *haystack, size_t haystackLen);
    static int caselessCompare(const char *, const char *, size_t);
    static char packByte(const char *String);
    static void unpackNibble(int Val, char *p);
    static char packNibble(const char *String);
    static RexxString *packHex(const char *String, size_t StringLength);
    static size_t copyGroupedChars(char *destination, const char *source, size_t length, size_t count, char set[256], size_t &scannedSize);
    static size_t validateGroupedSet(const char *string, size_t length, char set[256], int modulus, bool hex);
    static bool validateGroupedSetQuiet(const char *string, size_t length, char set[256], int modulus, size_t &packedSize);
    static const char* validateStrictSet(const char *string, char set[256], size_t length);
    static RexxObject *dataType(RexxString *String, char Option );
    static size_t wordCount(const char *String, size_t   StringLength );
    static size_t countStr(const char *hayStack, size_t hayStackLength, RexxString *needle, size_t maxCount);
    static size_t caselessCountStr(const char *hayStack, size_t hayStackLength, RexxString *needle, size_t maxCount);
    static size_t memPos(const char *string, size_t length, char target);
    static RexxInteger *verify(const char *data, size_t stringLen, RexxString  *ref, RexxString  *option, RexxInteger *_start, RexxInteger *range);
    static RexxString *subWord(const char *data, size_t length, RexxInteger *position, RexxInteger *plength);
    static ArrayClass *subWords(const char *data, size_t length, RexxInteger *position, RexxInteger *plength);
    static RexxString *word(const char *data, size_t length, RexxInteger *position);
    static RexxInteger *wordIndex(const char *data, size_t length, RexxInteger *position);
    static RexxInteger *wordLength(const char *data, size_t length, RexxInteger *position);
    static size_t wordPos(const char *data, size_t length, RexxString  *phrase, RexxInteger *pstart);
    static size_t caselessWordPos(const char *data, size_t length, RexxString  *phrase, RexxInteger *pstart);
    static ArrayClass   *words(const char *data, size_t length);
    static const char  *locateSeparator(const char *start, const char *end, const char *sepData, size_t sepLength);
    static bool decodeBase64(const char *source, size_t inputLength, char *destination, size_t &outputLength);
    static void encodeBase64(const char *source, size_t inputLength, MutableBuffer *destination, size_t chunkSize);

    static inline bool matchCharacter(char ch, const char *charSet, size_t len)
    {
        while (len-- > 0)
        {
            if (ch == *charSet++)
            {
                return true;
            }
        }
        return false;
    }
};

#endif

