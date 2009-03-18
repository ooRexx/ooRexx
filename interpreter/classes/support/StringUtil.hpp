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
/* String Utilities shared between String class and MutableBuffer class       */
/*                                                                            */
/******************************************************************************/
#ifndef Included_StringUtil
#define Included_StringUtil

class RexxString;
class RexxInteger;
class RexxArray;

class StringUtil
{
public:
    static RexxString *substr(const char *, size_t, RexxInteger *, RexxInteger *, RexxString *);
    static RexxInteger *posRexx(const char *stringData, size_t length, RexxString *needle, RexxInteger *pstart, RexxInteger *range);
    static size_t pos(const char *stringData, size_t haystack_length, RexxString *needle, size_t _start, size_t _range);
    static size_t caselessPos(const char *stringData, size_t haystack_length, RexxString *needle, size_t _start, size_t _range);
    static RexxInteger *lastPosRexx(const char *stringData, size_t haystackLen, RexxString  *needle, RexxInteger *_start, RexxInteger *_range);
    static size_t lastPos(const char *stringData, size_t hastackLen, RexxString  *needle, size_t _start, size_t _range);
    static const char *lastPos(const char *needle, size_t needleLen, const char *haystack, size_t haystackLen);
    static RexxString *subchar(const char *stringData, size_t stringLength, RexxInteger *positionArg);
    static RexxArray *makearray(const char *start, size_t length, RexxString *separator);
    static size_t caselessLastPos(const char *stringData, size_t hastackLen, RexxString  *needle, size_t _start, size_t range);
    static const char * caselessLastPos(const char *needle, size_t needleLen, const char *haystack, size_t haystackLen);
    static int caselessCompare(const char *, const char *, size_t);
    static int hexDigitToInt(char  ch);
    static char packByte(const char *String);
    static void unpackNibble(int Val, char *p);
    static char packNibble(const char *String);
    static RexxString *packHex(const char *String, size_t StringLength);
    static size_t chGetSm(char *Destination, const char *Source, size_t Length, size_t Count, const char *Set, size_t *ScannedSize);
    static size_t validateSet(const char *String, size_t Length, const char *Set, int Modulus, bool Hex);
    static char packByte2(const char *Byte);
    static int valSet(const char *String, size_t Length, const char *Set, int Modulus, size_t *PackedSize );
    static const char *memcpbrk(const char *String, const char *Set, size_t Length);
    static RexxObject *dataType(RexxString *String, char Option );
    static size_t wordCount(const char *String, size_t   StringLength );
    static void skipNonBlanks(const char **String, size_t *StringLength);
    static void skipBlanks(const char **String, size_t *StringLength);
    static size_t nextWord(const char **String, size_t *StringLength, const char **NextString );
    static size_t countStr(const char *hayStack, size_t hayStackLength, RexxString *needle);
    static size_t caselessCountStr(const char *hayStack, size_t hayStackLength, RexxString *needle);
    static size_t memPos(const char *string, size_t length, char target);
    static RexxInteger *verify(const char *data, size_t stringLen, RexxString  *ref, RexxString  *option, RexxInteger *_start, RexxInteger *range);
    static RexxString *subWord(const char *data, size_t length, RexxInteger *position, RexxInteger *plength);
    static RexxString *word(const char *data, size_t length, RexxInteger *position);
    static RexxInteger *wordIndex(const char *data, size_t length, RexxInteger *position);
    static RexxInteger *wordLength(const char *data, size_t length, RexxInteger *position);
    static RexxInteger *wordPos(const char *data, size_t length, RexxString  *phrase, RexxInteger *pstart);
    static RexxInteger *caselessWordPos(const char *data, size_t length, RexxString  *phrase, RexxInteger *pstart);
    static RexxArray   *words(const char *data, size_t length);
    static const char  *locateSeparator(const char *start, const char *end, const char *sepData, size_t sepLength);
};

#endif

