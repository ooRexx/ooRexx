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
/* REXX Kernel                                               StringClass.hpp  */
/*                                                                            */
/* Primitive String Class Definition                                          */
/*                                                                            */
/******************************************************************************/
#ifndef Included_StringClass
#define Included_StringClass

#include "IntegerClass.hpp"
#include "StringUtil.hpp"
#include "Utilities.hpp"
#include "FlagSet.hpp"
#include <string.h>

/**
 * Return type from string isSymbol() method.
 */
typedef enum
{
    STRING_BAD_VARIABLE,
    STRING_STEM,
    STRING_COMPOUND_NAME,
    STRING_LITERAL,
    STRING_LITERAL_DOT,
    STRING_NUMERIC,
    STRING_NAME
} StringSymbolType;


class RexxString : public RexxObject
{
 public:
     typedef enum
     {
        STRING_HASLOWER,
        STRING_NOLOWER,
        STRING_HASUPPER,
        STRING_NOUPPER,
        STRING_NONNUMERIC,
     } StringFlag;


     /**
      * A class for constructing a string value from a sequence
      * of append steps.
      */
     class StringBuilder
     {
     public:
         inline StringBuilder()  {}
         inline StringBuilder(char *b) : current(b) {}
         inline StringBuilder(RexxString *s) : current(s->getWritableData()) {}

         inline void init(RexxString *s)  { current = s->getWritableData(); }
         inline void append(const char *d, size_t l)  { memcpy(current, d, l); current += l; }
         inline void append(const char *d)  { size_t l = strlen(d); memcpy(current, d, l); current += l; }
         inline void append(char c) { *current++ = c; }
         inline void append(RexxString *s) { append(s->getStringData(), s->getLength()); }
         inline void pad(char c, size_t l)  { memset(current, c, l); current += l; }

     protected:
         char *current;   // current output pointer
     };


     /**
      * A class for constructing a string value from a sequence
      * of right-to-left put steps.
      */
     class StringBuilderRtL
     {
     public:
         inline StringBuilderRtL(RexxString *s) : current(s->getWritableData() + s->getLength() - 1) {}

         inline void put(char c) { *current-- = c; }

     protected:
         char *current;   // current output pointer
     };


     /**
      * A class for iterating through the words of a string.
      */
     class WordIterator
     {
     public:
         inline WordIterator(const char *b, size_t l) : nextPosition(b), scanLength(l) {}
         inline WordIterator(RexxString *s) : nextPosition(s->getStringData()), scanLength(s->getLength()) {}

         /**
          * Skip leading blanks in a string.
          *
          * @param String The target string.
          * @param scanLength
          *               The length of the string segment.
          */
         void skipBlanks(const char *&string, size_t &scanLength )
         {
             const char *scan = string;
             size_t length = scanLength;

             // just skip over any white space characters
             for (;length > 0; scan++, length--)
             {
                 if (*scan != ' ' && *scan != '\t')
                 {
                     break;
                 }
             }

             // we've either found a non blank or we ran out of string
             string = scan;
             scanLength = length;
         }

         /**
          * Skip non-blank characters to the next whitespace char.
          *
          * @param String The source string.
          * @param scanLength
          *               The string length (update on return);
          */
         void skipNonBlanks(const char *&string, size_t &scanLength )
         {
             const char *scan = string;
             size_t length = scanLength;

             for (;length > 0; scan++, length--)
             {
                 // stop if we find a white space character
                 if (*scan == ' ' || *scan == '\t')
                 {
                     break;
                 }
             }
             string = scan;
             scanLength = length;
         }


         /**
          * Scan forward to the next word in the string.
          *
          * @return true if a word was found, false if we've run out out words.
          */
         inline bool next()
         {
             // NOTE: We leave the word pointer and length untouched until we
             // have a hit so that after an iteration failure, we can still retrieve
             // the last word.

             // if out of string, this is a failure
             if (scanLength == 0)
             {
                 return false;
             }

             // if we have string left
             // skip over any blanks
             skipBlanks(nextPosition, scanLength);
             // if we ran out of string, there are no more words
             if (scanLength == 0)
             {
                 return false;
             }

             // save the starting length
             currentWordLength = scanLength;
             // save the start of the word
             currentWord = nextPosition;

             // skip over the non-blank charactes
             skipNonBlanks(nextPosition, scanLength);
             // get the length of the next word
             currentWordLength -= scanLength;
             // we have a word
             return true;
         }


         /**
          * Skip over a given number of words.  Returns true if
          * the count was reached before the end of the string, false
          * if the end is reached first.
          *
          * @param count  The count to skip.
          *
          * @return true if count words were found.
          */
         inline bool skipWords(size_t count)
         {
             while (count--)
             {
                 if (!next())
                 {
                     return false;
                 }
             }
             return true;
         }


         // skip from the end of the current word to to the next word
         // or the end of the string
         inline void skipBlanks()
         {
             skipBlanks(nextPosition, scanLength);
         }


         /**
          * Compare the current word in two iterators.
          *
          * @param other  The other comparison.
          *
          * @return true if the words match, false otherwise
          */
         inline bool compare(WordIterator &other)
         {
             // length mismatch, can't be a match
             if (currentWordLength != other.wordLength())
             {
                 return false;
             }

             // now compare the two words
             return memcmp(currentWord, other.wordPointer(), currentWordLength) == 0;
         }


         /**
          * Compare the current word in two iterators.
          *
          * @param other  The other comparison.
          *
          * @return true if the words match, false otherwise
          */
         inline bool caselessCompare(WordIterator &other)
         {
             // length mismatch, can't be a match
             if (currentWordLength != other.wordLength())
             {
                 return false;
             }

             // now compare the two words
             return StringUtil::caselessCompare(currentWord, other.wordPointer(), currentWordLength) == 0;
         }


         inline size_t wordLength() { return currentWordLength; }
         inline const char *wordPointer() { return currentWord; }
         inline const char *wordEndPointer() { return currentWord + currentWordLength; }
         inline void append(StringBuilder &builder) { builder.append(currentWord, currentWordLength); }
         inline void appendRemainder(StringBuilder &builder) { builder.append(nextPosition, scanLength); }
         inline const char *scanPosition() { return nextPosition; }
         inline size_t length() { return scanLength; }

     protected:
         const char *currentWord;       // the current word position
         const char *nextPosition;      // the next scan position
         size_t currentWordLength;      // the length of the current word match
         size_t scanLength;             // the remaining scan length
     };


    inline RexxString() {;} ;
    inline RexxString(RESTORETYPE restoreType) { ; }

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope *envelope) override;
    RexxInternalObject *unflatten(Envelope *) override;

    HashCode getHashValue() override;

    inline HashCode getStringHash()
    {
        if (hashValue == 0)                // if we've never generated this, the code is zero
        {
            size_t len = getLength();

            HashCode h = 0;
            // the hash code is generated from all of the string characters.
            // we do this in a lazy fashion, since most string objects never need to
            // calculate a hash code, particularly the long ones.
            // This hashing algorithm is very similar to that used for Java strings.
            for (size_t i = 0; i < len; i++)
            {
                h = 31 * h + stringData[i];
            }
            hashValue = h;
        }
        return hashValue;
    }

    HashCode getObjectHashCode();

    bool numberValue(wholenumber_t &result, wholenumber_t precision) override;
    bool numberValue(wholenumber_t &result) override;
    bool unsignedNumberValue(size_t &result, wholenumber_t precision) override;
    bool unsignedNumberValue(size_t &result) override;
    bool doubleValue(double &result) override;
    NumberString *numberString() override;
    RexxInteger *integerValue(wholenumber_t) override;
    RexxString  *makeString() override;
    ArrayClass  *makeArray() override;
    RexxString  *primitiveMakeString() override;
    void         copyIntoTail(CompoundVariableTail *buffer) override;
    RexxString  *stringValue() override;
    bool  truthValue(RexxErrorCodes) override;
    bool logicalValue(logical_t &) override;

    // comparison methods
    bool isEqual(RexxInternalObject *) override;

    bool        primitiveIsEqual(RexxObject *);
    bool        primitiveCaselessIsEqual(RexxObject *);
    wholenumber_t strictComp(RexxObject *);
    wholenumber_t comp(RexxObject *);
    wholenumber_t primitiveStrictComp(RexxObject *);
    wholenumber_t stringComp(RexxString *);
    wholenumber_t compareTo(RexxInternalObject *) override;
    RexxObject  *equal(RexxObject *);
    RexxObject  *strictEqual(RexxObject *);
    RexxObject  *notEqual(RexxObject *);
    RexxObject  *strictNotEqual(RexxObject *);
    RexxObject  *isGreaterThan(RexxObject *);
    RexxObject  *isLessThan(RexxObject *);
    RexxObject  *isGreaterOrEqual(RexxObject *);
    RexxObject  *isLessOrEqual(RexxObject *);
    RexxObject  *strictGreaterThan(RexxObject *);
    RexxObject  *strictLessThan(RexxObject *);
    RexxObject  *strictGreaterOrEqual(RexxObject *);
    RexxObject  *strictLessOrEqual(RexxObject *);

    size_t      copyData(size_t, char *, size_t);
    RexxObject *lengthRexx();
    RexxString *concatRexx(RexxObject *);
    RexxString *concat(RexxString *);
    RexxString *concatToCstring(const char *);
    RexxString *concatWithCstring(const char *);
    RexxString *concatBlank(RexxObject *);
    bool        checkLower();
    bool        checkUpper();
    RexxString *upper();
    RexxString *upper(size_t, size_t);
    RexxString *upperRexx(RexxInteger *, RexxInteger *);
    RexxString *lower();
    RexxString *lower(size_t, size_t);
    RexxString *lowerRexx(RexxInteger *, RexxInteger *);
    RexxString *stringTrace();
    void        setNumberString(NumberString *);
    RexxString *concatWith(RexxString *, char);

    RexxObject *plus(RexxObject *right);
    RexxObject *minus(RexxObject *right);
    RexxObject *multiply(RexxObject *right);
    RexxObject *divide(RexxObject *right);
    RexxObject *integerDivide(RexxObject *right);
    RexxObject *remainder(RexxObject *right);
    RexxObject *power(RexxObject *right);
    RexxObject *abs();
    RexxObject *sign();
    RexxObject *notOp();
    RexxObject *operatorNot(RexxObject *);
    RexxObject *andOp(RexxObject *);
    RexxObject *orOp(RexxObject *);
    RexxObject *xorOp(RexxObject *);
    RexxObject *choiceRexx(RexxObject *trueResult, RexxObject *falseResult);
    RexxObject *Max(RexxObject **args, size_t argCount);
    RexxObject *Min(RexxObject **args, size_t argCount);
    RexxObject *modulo(RexxObject *divisor);
    RexxObject *trunc(RexxInteger *decimals);
    RexxObject *floor();
    RexxObject *ceiling();
    RexxObject *round();
    RexxObject *format(RexxObject *Integers, RexxObject *Decimals, RexxObject *MathExp, RexxObject *ExpTrigger);
    RexxObject *logicalOperation(RexxObject *, RexxObject *, unsigned int);
    RexxString *extract(size_t offset, size_t sublength) { return newString(getStringData() + offset, sublength); }

    RexxObject *evaluate(RexxActivation *, ExpressionStack *) override;
    RexxObject *getValue(RexxActivation *) override;
    RexxObject *getValue(VariableDictionary *) override;
    RexxObject *getRealValue(RexxActivation *) override;
    RexxObject *getRealValue(VariableDictionary *) override;
                                        /* the following methods are in    */
                                        /* OKBSUBS                         */
    RexxString  *center(RexxInteger *, RexxString *);
    RexxString  *delstr(RexxInteger *, RexxInteger *);
    RexxString  *insert(RexxString *, RexxInteger *, RexxInteger *, RexxString *);
    RexxString  *left(RexxInteger *, RexxString *);
    RexxString  *overlay(RexxString *, RexxInteger *, RexxInteger *, RexxString *);
    RexxString  *replaceAt(RexxString *, RexxInteger *, RexxInteger *, RexxString *);
    RexxString  *reverse();
    RexxString  *right(RexxInteger *, RexxString *);
    RexxString  *strip(RexxString *, RexxString *);
    RexxString  *substr(RexxInteger *, RexxInteger *, RexxString *);
    RexxString  *brackets(RexxInteger *, RexxInteger *);
    RexxString  *subchar(RexxInteger *);
    RexxString  *delWord(RexxInteger *, RexxInteger *);
    RexxString  *space(RexxInteger *, RexxString *);
    RexxString  *subWord(RexxInteger *, RexxInteger *);
    ArrayClass   *subWords(RexxInteger *, RexxInteger *);
    RexxString  *word(RexxInteger *);
    RexxInteger *wordIndex(RexxInteger *);
    RexxInteger *wordLength(RexxInteger *);
    RexxInteger *wordPos(RexxString *, RexxInteger *);
    RexxInteger *caselessWordPos(RexxString *, RexxInteger *);
    RexxObject  *containsWord(RexxString *, RexxInteger *);
    RexxObject  *caselessContainsWord(RexxString *, RexxInteger *);
    RexxInteger *words();
                                        /* the following methods are in    */
                                        /* OKBMISC                         */
    RexxString  *changeStr(RexxString *, RexxString *, RexxInteger *);
    RexxString  *caselessChangeStr(RexxString *, RexxString *, RexxInteger *);
    RexxObject  *abbrev(RexxString *, RexxInteger *);
    RexxObject  *caselessAbbrev(RexxString *, RexxInteger *);
    RexxInteger *compare(RexxString *, RexxString *);
    RexxInteger *caselessCompare(RexxString *, RexxString *);
    RexxString  *copies(RexxInteger *);
    RexxObject  *dataType(RexxString *);

    RexxInteger *lastPosRexx(RexxString *, RexxInteger *, RexxInteger *);
    RexxInteger *caselessLastPosRexx(RexxString *, RexxInteger *, RexxInteger *);
    size_t       lastPos(RexxString  *needle, size_t start);
    size_t       caselessLastPos(RexxString  *needle, size_t start);
    const char * caselessLastPos(const char *needle, size_t needleLen, const char *haystack, size_t haystackLen);

    RexxInteger *posRexx(RexxString *, RexxInteger *, RexxInteger *);
    RexxInteger *caselessPosRexx(RexxString *, RexxInteger *, RexxInteger *);
    RexxObject  *containsRexx(RexxString *, RexxInteger *, RexxInteger *);
    RexxObject  *caselessContains(RexxString *, RexxInteger *, RexxInteger *);
    size_t       pos(RexxString *, size_t);
    size_t       caselessPos(RexxString *, size_t);

    RexxString  *translate(RexxString *, RexxString *, RexxString *, RexxInteger *, RexxInteger *);
    RexxInteger *verify(RexxString *, RexxString *, RexxInteger *, RexxInteger *);
    RexxInteger *countStrRexx(RexxString *);
    RexxInteger *caselessCountStrRexx(RexxString *);
    size_t       caselessCountStr(RexxString *);

    RexxString  *bitAnd(RexxString *, RexxString *);
    RexxString  *bitOr(RexxString *, RexxString *);
    RexxString  *bitXor(RexxString *, RexxString *);

    RexxString  *b2x();
    RexxString  *c2d(RexxInteger *);
    RexxString  *c2x();
    RexxString  *encodeBase64();
    RexxString  *decodeBase64();
    RexxString  *d2c(RexxInteger *);
    RexxString  *d2x(RexxInteger *);
    RexxString  *x2b();
    RexxString  *x2c();
    RexxString  *x2d(RexxInteger *);
    RexxString  *x2dC2d(RexxInteger *, bool);

    RexxObject  *match(RexxInteger *start_, RexxString *other, RexxInteger *offset_, RexxInteger *len_);
    RexxObject  *caselessMatch(RexxInteger *start_, RexxString *other, RexxInteger *offset_, RexxInteger *len_);
    RexxObject  *startsWithRexx(RexxString *other);
    RexxObject  *endsWithRexx(RexxString *other);
    RexxObject  *caselessStartsWithRexx(RexxString *other);
    RexxObject  *caselessEndsWithRexx(RexxString *other);
    bool primitiveMatch(size_t start, RexxString *other, size_t offset, size_t len);
    bool primitiveCaselessMatch(size_t start, RexxString *other, size_t offset, size_t len);
    RexxObject  *matchChar(RexxInteger *position_, RexxString *matchSet);
    RexxObject  *caselessMatchChar(RexxInteger *position_, RexxString *matchSet);

    RexxInteger *compareToRexx(RexxString *other, RexxInteger *start_, RexxInteger *len_);
    RexxInteger *caselessCompareToRexx(RexxString *other, RexxInteger *start_, RexxInteger *len_);
    wholenumber_t primitiveCompareTo(RexxString *other, size_t start, size_t len);
    wholenumber_t primitiveCaselessCompareTo(RexxString *other, size_t start, size_t len);
    wholenumber_t primitiveCompareTo(RexxString *other);
    wholenumber_t primitiveCaselessCompareTo(RexxString *other);

    RexxObject  *equals(RexxString *other);
    RexxObject  *caselessEquals(RexxString *other);

    ArrayClass   *makeArrayRexx(RexxString *);

    StringSymbolType isSymbol();

// Inline_functions

    inline RexxString *baseString() { return isBaseClass() ? this : requestString(); }
    inline size_t  getLength() const { return length; }
    inline bool isNullString() const { return length == 0; }
    inline void  setLength(size_t l) { length = l; }
    inline void  finish(size_t l) { length = l; }
    inline const char *getStringData() const { return stringData; }
    inline char *getWritableData() { return &stringData[0]; }
    inline void  put(size_t s, const void *b, size_t l) { memcpy(getWritableData() + s, b, l); }
    inline void  put(size_t s, RexxString *o) { put(s, o->getStringData(), o->getLength()); }
    inline void  set(size_t s,int c, size_t l) { memset((stringData+s), c, l); }
    inline char  getChar(size_t p) const { return *(stringData+p); }
    inline char  putChar(size_t p,char c) { return *(stringData+p) = c; }
    inline bool  upperOnly() const {return attributes[STRING_NOLOWER];}
    inline bool  hasLower() const {return attributes[STRING_HASLOWER]; }
    inline void  setUpperOnly() { attributes.set(STRING_NOLOWER);}
    inline void  setHasLower() { attributes.set(STRING_HASLOWER);}
    inline bool  lowerOnly() const {return attributes[STRING_NOUPPER];}
    inline bool  hasUpper() const {return attributes[STRING_HASUPPER]; }
    inline void  setLowerOnly() { attributes.set(STRING_NOUPPER);}
    inline void  setHasUpper() { attributes.set(STRING_HASUPPER);}
    inline bool  nonNumeric() const {return attributes[STRING_NONNUMERIC];}
    inline void  setNonNumeric() { attributes.set(STRING_NONNUMERIC);}
    inline bool  strCompare(const char * s) const { return memCompare((s), strlen(s)); }
    inline bool  strCaselessCompare(const char * s) const { return (size_t)length == strlen(s) && Utilities::strCaselessCompare(s, stringData) == 0;}
    inline bool  strCaselessCompare(RexxString *s) const { return length == s->getLength() && Utilities::strCaselessCompare(s->getStringData(), stringData) == 0;}
    inline bool  memCompare(const char * s, size_t l) const { return l == length && memcmp(s, stringData, l) == 0; }
    inline bool  memCompare(RexxString *other) const { return other->length == length && memcmp(other->stringData, stringData, length) == 0; }
    inline bool  strCompare(RexxString *other) const { return other->length == length && memcmp(other->stringData, stringData, length) == 0; }
    inline void  memCopy(char * s) const { memcpy(s, stringData, length); }
    inline void  toRxstring(CONSTRXSTRING &r) { r.strptr = getStringData(); r.strlength = getLength(); }
    inline void  toRxstring(RXSTRING &r) { r.strptr = getWritableData(); r.strlength = getLength(); }
           void  copyToRxstring(RXSTRING &r);
    inline bool  endsWith(char c) const { return length > 0 && stringData[length - 1] == c; }
    inline bool  startsWith(char c) const { return length > 0 && stringData[0] == c; }
    inline bool  startsWith(const char *c) const { size_t clen = strlen(c); return length >=clen && memcmp(getStringData(), c, clen) == 0; }

    inline int sortCompare(RexxString *other)
    {
        size_t compareLength = length;
        if (compareLength > other->length)
        {
            compareLength = other->length;
        }
        int result = memcmp(stringData, other->stringData, compareLength);
        if (result == 0)
        {
            if (length > other->length)
            {
                result = 1;
            }
            else if (length < other->length)
            {
                result = -1;
            }
        }
        return result;
    }

    inline int sortCaselessCompare(RexxString *other)
    {
        size_t compareLength = length;
        if (compareLength > other->length)
        {
            compareLength = other->length;
        }
        int result = StringUtil::caselessCompare(stringData, other->stringData, compareLength);
        if (result == 0)
        {
            if (length > other->length)
            {
                result = 1;
            }
            else if (length < other->length)
            {
                result = -1;
            }
        }
        return result;
    }

    inline int sortCompare(RexxString *other, size_t startCol, size_t colLength)
    {
        int result = 0;
        if ((startCol < length ) && (startCol < other->length))
        {
            size_t stringLength = length;
            if (stringLength > other->length)
            {
                stringLength = other->length;
            }
            stringLength = stringLength - startCol + 1;
            size_t compareLength = colLength;
            if (compareLength > stringLength)
            {
                compareLength = stringLength;
            }

            result = memcmp(stringData + startCol, other->stringData + startCol, compareLength);
            if (result == 0 && stringLength < colLength)
            {
                if (length > other->length)
                {
                    result = 1;
                }
                else if (length < other->length)
                {
                    result = -1;
                }
            }
        }
        else
        {
            if (length == other->length)
            {
                result = 0;
            }
            else
            {
                result = length < other->length ? -1 : 1;
            }
        }
        return result;
    }

    inline int sortCaselessCompare(RexxString *other, size_t startCol, size_t colLength)
    {
        int result = 0;
        if ((startCol < length ) && (startCol < other->length))
        {
            size_t stringLength = length;
            if (stringLength > other->length)
            {
                stringLength = other->length;
            }
            stringLength = stringLength - startCol + 1;
            size_t compareLength = colLength;
            if (compareLength > stringLength)
            {
                compareLength = stringLength;
            }

            result = StringUtil::caselessCompare(stringData + startCol, other->stringData + startCol, compareLength);
            if (result == 0 && stringLength < colLength)
            {
                if (length > other->length)
                {
                    result = 1;
                }
                else if (length < other->length)
                {
                    result = -1;
                }
            }
        }
        else
        {
            if (length == other->length)
            {
                result = 0;
            }
            else
            {
                result = length < other->length ? -1 : 1;
            }
        }
        return result;
    }

    static inline char intToHexDigit(int n)
    {
        return "0123456789ABCDEF"[n];
    }

    static inline int hexDigitToInt(char ch)
    {
        return DIGITS_HEX_LOOKUP[(unsigned char)ch];
    }

    static inline char packByte2(const char *bytes)
    {
        // covert each hex digit and combind into a single value
        int nibble1 = hexDigitToInt(bytes[0]);
        int nibble2 = hexDigitToInt(bytes[1]);
        /* combine the two digits            */

        return ((nibble1 << 4) | nibble2);
    }

    static RexxString *newString(const char *, size_t);
    static RexxString *newString(const char *, size_t, const char *, size_t);
    static RexxString *rawString(size_t);
    static RexxString *newUpperString(const char *, size_t);
    static RexxString *newString(double d);
    static RexxString *newString(double d, size_t precision);
    static RexxString *newProxy(const char *);
    // NB:  newRexx() cannot be static and exported as an ooRexx method.
           RexxString *newRexx(RexxObject **, size_t);
    static PCPPM operatorMethods[];

    static void createInstance();
    static RexxClass *classInstance;

    // Strip method options
    static const char STRIP_BOTH =              'B';
    static const char STRIP_LEADING =           'L';
    static const char STRIP_TRAILING =          'T';

    // Datatype method options
    static const char DATATYPE_ALPHANUMERIC =   'A';
    static const char DATATYPE_BINARY =         'B';
    static const char DATATYPE_INTERNAL_WHOLE = 'I';
    static const char DATATYPE_LOWERCASE =      'L';
    static const char DATATYPE_MIXEDCASE =      'M';
    static const char DATATYPE_NUMBER =         'N';
    static const char DATATYPE_SYMBOL =         'S';
    static const char DATATYPE_VARIABLE =       'V';
    static const char DATATYPE_UPPERCASE =      'U';
    static const char DATATYPE_WHOLE_NUMBER =   'W';
    static const char DATATYPE_HEX =            'X';
    static const char DATATYPE_9DIGITS =        '9';
    static const char DATATYPE_LOGICAL =        'O';  // lOgical.

    // Verify method options
    static const char VERIFY_MATCH =            'M';
    static const char VERIFY_NOMATCH =          'N';

    // string white space characters
    static const char ch_SPACE = ' ';
    static const char ch_TAB   = '\t';

    // Define char data used in in number string parsing
    static const char ch_MINUS;
    static const char ch_PLUS;
    static const char ch_PERIOD;
    static const char ch_ZERO;
    static const char ch_ONE;
    static const char ch_FIVE;
    static const char ch_NINE;

    // character validation sets for the datatype function
    static const char *DIGITS_HEX;
    static const char *ALPHANUM;
    static const char *DIGITS_BIN;
    static const char *LOWER_ALPHA;
    static const char *MIXED_ALPHA;
    static const char *UPPER_ALPHA;
    static const char *DIGITS_BASE64;

    // POSIX character ranges returned by XRANGE() and .String class methods
    static const char *ALNUM;
    static const char *ALPHA;
    static const char *BLANK;
    static const char *CNTRL;
    static const char *DIGIT;
    static const char *GRAPH;
    static const char *LOWER;
    static const char *PRINT;
    static const char *PUNCT;
    static const char *SPACE;
    static const char *UPPER;
    static const char *XDIGIT;

    // Mapped character validation sets allow direct lookup.
    // Those will be created from their unmapped string counterparts.
    static struct lookupInit { lookupInit(); } lookupInitializer;
    static char DIGITS_HEX_LOOKUP[256];
    static char DIGITS_BASE64_LOOKUP[256];
    static char DIGITS_BIN_LOOKUP[256];
    static char ALPHANUM_LOOKUP[256];
    static char LOWER_ALPHA_LOOKUP[256];
    static char MIXED_ALPHA_LOOKUP[256];
    static char UPPER_ALPHA_LOOKUP[256];


  protected:

    HashCode hashValue;                      // stored has value
    size_t length;                           // string length
    NumberString *numberStringValue;         // lookaside information
    FlagSet<StringFlag, 32> attributes;      // string attributes
    char stringData[4];                      // Start of the string data part
};


// String creation inline functions

inline RexxString *new_string(const char *s, size_t l)
{
    return RexxString::newString(s, l);
}


// String creation inline functions

inline RexxString *new_string(const char *s1, size_t l1, const char *s2, size_t l2)
{
    return RexxString::newString(s1, l1, s2, l2);
}

inline RexxString *raw_string(size_t l)
{
    return RexxString::rawString(l);
}

inline RexxString *new_string(double d)
{
    return RexxString::newString(d);
}

inline RexxString *new_string(double d, size_t p)
{
    return RexxString::newString(d, p);
}


inline RexxString *new_string(const char *string)
{
    return new_string(string, strlen(string));
}


inline RexxString *new_string(char cc)
{
    return new_string(&cc, 1);
}

inline RexxString *new_string(RXSTRING &r)
{
    return new_string(r.strptr, r.strlength);
}


inline RexxString *new_string(CONSTRXSTRING &r)
{
    return new_string(r.strptr, r.strlength);
}

inline RexxString *new_proxy(const char *name)
{

    return RexxString::newProxy(name);
}

inline RexxString *new_upper_string(const char *s, size_t l)
{
    return RexxString::newUpperString(s, l);
}

inline RexxString *new_upper_string(const char *string)
{
    return new_upper_string(string, strlen(string));
}


#endif
