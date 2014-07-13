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
/* REXX Kernel                                               StringClass.hpp  */
/*                                                                            */
/* Primitive String Class Definition                                          */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxString
#define Included_RexxString

#include "NumberStringClass.hpp"
#include "IntegerClass.hpp"
#include "StringUtil.hpp"
#include "Utilities.hpp"
#include "FlagSet.hpp"

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
        STRING_NONNUMERIC,
     } StringFlag;

    inline void       *operator new(size_t size, void *ptr){return ptr;}
    inline RexxString() {;} ;
    inline RexxString(RESTORETYPE restoreType) { ; }

    virtual void live(size_t);
    virtual void liveGeneral(MarkReason reason);
    virtual void flatten(RexxEnvelope *envelope);
    virtual RexxObject *unflatten(RexxEnvelope *);

    virtual HashCode hash();
    virtual HashCode getHashValue();

    inline HashCode getStringHash()
    {
        if (hashValue == 0)                // if we've never generated this, the code is zero
        {
            stringsize_t len = getLength();

            HashCode h = 0;
            // the hash code is generated from all of the string characters.
            // we do this in a lazy fashion, since most string objects never need to
            // calculate a hash code, particularly the long ones.
            // This hashing algorithm is very similar to that used for Java strings.
            for (stringsize_t i = 0; i < len; i++)
            {
                h = 31 * h + stringData[i];
            }
            hashValue = h;
        }
        return hashValue;
    }

    HashCode getObjectHashCode();

    bool         numberValue(wholenumber_t &result, size_t precision);
    bool         numberValue(wholenumber_t &result);
    bool         unsignedNumberValue(stringsize_t &result, size_t precision);
    bool         unsignedNumberValue(stringsize_t &result);
    bool         doubleValue(double &result);
    RexxNumberString *numberString();
    RexxInteger *integerValue(size_t);
    RexxString  *makeString();
    RexxArray   *makeArray();
    RexxString  *primitiveMakeString();
    void         copyIntoTail(RexxCompoundTail *buffer);
    RexxString  *stringValue();
    bool         truthValue(int);
    virtual bool logicalValue(logical_t &);

    // comparison methods
    bool        isEqual(RexxObject *);
    bool        primitiveIsEqual(RexxObject *);
    bool        primitiveCaselessIsEqual(RexxObject *);
    wholenumber_t strictComp(RexxObject *);
    wholenumber_t comp(RexxObject *);
    wholenumber_t compareTo(RexxObject *);
    RexxInteger *equal(RexxObject *);
    RexxInteger *strictEqual(RexxObject *);
    RexxInteger *notEqual(RexxObject *);
    RexxInteger *strictNotEqual(RexxObject *);
    RexxInteger *isGreaterThan(RexxObject *);
    RexxInteger *isLessThan(RexxObject *);
    RexxInteger *isGreaterOrEqual(RexxObject *);
    RexxInteger *isLessOrEqual(RexxObject *);
    RexxInteger *strictGreaterThan(RexxObject *);
    RexxInteger *strictLessThan(RexxObject *);
    RexxInteger *strictGreaterOrEqual(RexxObject *);
    RexxInteger *strictLessOrEqual(RexxObject *);

    size_t      copyData(size_t, char *, size_t);
    RexxObject *lengthRexx();
    RexxString *concatRexx(RexxObject *);
    RexxString *concat(RexxString *);
    RexxString *concatToCstring(const char *);
    RexxString *concatWithCstring(const char *);
    RexxString *concatBlank(RexxObject *);
    bool        checkLower();
    RexxString *upper();
    RexxString *upper(size_t, size_t);
    RexxString *upperRexx(RexxInteger *, RexxInteger *);
    RexxString *lower();
    RexxString *lower(size_t, size_t);
    RexxString *lowerRexx(RexxInteger *, RexxInteger *);
    RexxString *stringTrace();
    void        setNumberString(RexxObject *);
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
    RexxObject *Max(RexxObject **args, size_t argCount);
    RexxObject *Min(RexxObject **args, size_t argCount);
    RexxObject *trunc(RexxInteger *decimals);
    RexxObject *floor();
    RexxObject *ceiling();
    RexxObject *round();
    RexxObject *format(RexxObject *Integers, RexxObject *Decimals, RexxObject *MathExp, RexxObject *ExpTrigger);
    RexxObject *isInteger();
    RexxObject *logicalOperation(RexxObject *, RexxObject *, unsigned int);
    RexxString *extract(size_t offset, size_t sublength) { return newString(getStringData() + offset, sublength); }
    RexxObject *evaluate(RexxActivation *, RexxExpressionStack *);
    RexxObject *getValue(RexxActivation *);
    RexxObject *getValue(RexxVariableDictionary *);
    RexxObject *getRealValue(RexxActivation *);
    RexxObject *getRealValue(RexxVariableDictionary *);
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
    RexxString  *subchar(RexxInteger *);
    RexxString  *delWord(RexxInteger *, RexxInteger *);
    RexxString  *space(RexxInteger *, RexxString *);
    RexxString  *subWord(RexxInteger *, RexxInteger *);
    RexxArray   *subWords(RexxInteger *, RexxInteger *);
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
    RexxInteger *abbrev(RexxString *, RexxInteger *);
    RexxInteger *caselessAbbrev(RexxString *, RexxInteger *);
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

    RexxInteger *match(RexxInteger *start_, RexxString *other, RexxInteger *offset_, RexxInteger *len_);
    RexxInteger *caselessMatch(RexxInteger *start_, RexxString *other, RexxInteger *offset_, RexxInteger *len_);
    bool primitiveMatch(stringsize_t start, RexxString *other, stringsize_t offset, stringsize_t len);
    bool primitiveCaselessMatch(stringsize_t start, RexxString *other, stringsize_t offset, stringsize_t len);
    RexxInteger *matchChar(RexxInteger *position_, RexxString *matchSet);
    RexxInteger *caselessMatchChar(RexxInteger *position_, RexxString *matchSet);

    RexxInteger *compareToRexx(RexxString *other, RexxInteger *start_, RexxInteger *len_);
    RexxInteger *caselessCompareToRexx(RexxString *other, RexxInteger *start_, RexxInteger *len_);
    RexxInteger *primitiveCompareTo(RexxString *other, stringsize_t start, stringsize_t len);
    RexxInteger *primitiveCaselessCompareTo(RexxString *other, stringsize_t start, stringsize_t len);

    RexxInteger *equals(RexxString *other);
    RexxInteger *caselessEquals(RexxString *other);

    RexxArray   *makeArrayRexx(RexxString *);

    StringSymbolType isSymbol();

// Inline_functions

    inline size_t  getLength() const { return length; }
    inline void  setLength(size_t l) { length = l; }
    inline void  finish(stringsize_t l) { length = l; }
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
    inline bool  nonNumeric() const {return attributes[STRING_NONNUMERIC];}
    inline void  setNonNumeric() { attributes.set(STRING_NONNUMERIC);}
    inline bool  strCompare(const char * s) const {return memCompare((s), strlen(s));}
    inline bool  strCaselessCompare(const char * s) const { return (size_t)length == strlen(s) && Utilities::strCaselessCompare(s, stringData) == 0;}
    inline bool  memCompare(const char * s, size_t l) const { return l == length && memcmp(s, stringData, l) == 0; }
    inline bool  memCompare(RexxString *other) const { return other->length == length && memcmp(other->stringData, stringData, length) == 0; }
    inline void  memCopy(char * s) const { memcpy(s, stringData, length); }
    inline void  toRxstring(CONSTRXSTRING &r) { r.strptr = getStringData(); r.strlength = getLength(); }
    inline void  toRxstring(RXSTRING &r) { r.strptr = getWritableData(); r.strlength = getLength(); }
           void  copyToRxstring(RXSTRING &r);
    inline bool  endsWith(char c) const { return length > 0 && stringData[length - 1] == c; }

    RexxNumberString *createNumberString();

    inline RexxNumberString *fastNumberString()
    {
        // already converted?  Done!
        if (numberStringValue != OREF_NULL)
        {
            return numberStringValue;
        }
        // did we already try to convert and fail?
        if (nonNumeric())
        {
            return OREF_NULL;
        }

        // build the value (if possible)
        return createNumberString();
    }

    inline int sortCompare(RexxString *other) {
        size_t compareLength = length;
        if (compareLength > other->length) {
            compareLength = other->length;
        }
        int result = memcmp(stringData, other->stringData, compareLength);
        if (result == 0) {
            if (length > other->length) {
                result = 1;
            }
            else if (length < other->length) {
                result = -1;
            }
        }
        return result;
    }

    inline int sortCaselessCompare(RexxString *other) {
        size_t compareLength = length;
        if (compareLength > other->length) {
            compareLength = other->length;
        }
        int result = StringUtil::caselessCompare(stringData, other->stringData, compareLength);
        if (result == 0) {
            if (length > other->length) {
                result = 1;
            }
            else if (length < other->length) {
                result = -1;
            }
        }
        return result;
    }

    inline int sortCompare(RexxString *other, size_t startCol, size_t colLength) {
        int result = 0;
        if ((startCol < length ) && (startCol < other->length)) {
            size_t stringLength = length;
            if (stringLength > other->length) {
                stringLength = other->length;
            }
            stringLength = stringLength - startCol + 1;
            size_t compareLength = colLength;
            if (compareLength > stringLength) {
                compareLength = stringLength;
            }

            result = memcmp(stringData + startCol, other->stringData + startCol, compareLength);
            if (result == 0 && stringLength < colLength) {
                if (length > other->length) {
                    result = 1;
                }
                else if (length < other->length) {
                    result = -1;
                }
            }
        }
        else {
            if (length == other->length) {
                result = 0;
            }
            else {
                result = length < other->length ? -1 : 1;
            }
        }
        return result;
    }

    inline int sortCaselessCompare(RexxString *other, size_t startCol, size_t colLength) {
        int result = 0;
        if ((startCol < length ) && (startCol < other->length)) {
            size_t stringLength = length;
            if (stringLength > other->length) {
                stringLength = other->length;
            }
            stringLength = stringLength - startCol + 1;
            size_t compareLength = colLength;
            if (compareLength > stringLength) {
                compareLength = stringLength;
            }

            result = StringUtil::caselessCompare(stringData + startCol, other->stringData + startCol, compareLength);
            if (result == 0 && stringLength < colLength) {
                if (length > other->length) {
                    result = 1;
                }
                else if (length < other->length) {
                    result = -1;
                }
            }
        }
        else {
            if (length == other->length) {
                result = 0;
            }
            else {
                result = length < other->length ? -1 : 1;
            }
        }
        return result;
    }

    static inline char intToHexDigit(int n)
    {
        return "0123456789ABCDEF"[n];
    }


    static RexxString *newString(const char *, size_t);
    static RexxString *rawString(size_t);
    static RexxString *newUpperString(const char *, stringsize_t);
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
    static const char ch_MINUS  = '-';
    static const char ch_PLUS   = '+';
    static const char ch_PERIOD = '.';
    static const char ch_ZERO   = '0';
    static const char ch_ONE    = '1';
    static const char ch_FIVE   = '5';
    static const char ch_NINE   = '9';

    // character validation sets for the datatype function
    static const char *HEX_CHAR_STR;
    static const char *ALPHANUM;
    static const char *BINARY;
    static const char *LOWER_ALPHA;
    static const char *MIXED_ALPHA;
    static const char *UPPER_ALPHA;

  protected:

    HashCode hashValue;                      // stored has value
    size_t length;                           // string length
    RexxNumberString *numberStringValue;     // lookaside information
    FlagSet<StringFlag, 32> attributes;      // string attributes
    char stringData[4];                      // Start of the string data part
};


// String creation inline functions

inline RexxString *new_string(const char *s, stringsize_t l)
{
    return RexxString::newString(s, l);
}

inline RexxString *raw_string(stringsize_t l)
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

inline RexxString *new_upper_string(const char *s, stringsize_t l)
{
    return RexxString::newUpperString(s, l);
}

inline RexxString *new_upper_string(const char *string)
{
    return new_upper_string(string, strlen(string));
}
#endif
