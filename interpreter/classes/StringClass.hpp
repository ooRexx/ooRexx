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
                                       /* return values from the is_symbol  */
                                       /* validation method                 */
#define  STRING_BAD_VARIABLE   0
#define  STRING_STEM           1
#define  STRING_COMPOUND_NAME  2
#define  STRING_LITERAL        3
#define  STRING_LITERAL_DOT    4
#define  STRING_NUMERIC        5
#define  STRING_NAME           6

#define  STRING_HASLOWER       0x01    /* string does contain lowercase     */
#define  STRING_NOLOWER        0x02    /* string does not contain lowercase */
#define  STRING_NONNUMERIC     0x04    /* string is non-numeric             */

#define  INITIAL_NAME_SIZE     10      /* first name table allocation       */
#define  EXTENDED_NAME_SIZE    10      /* amount to extend table by         */
                                       /* Strip function options     */
#define  STRIP_BOTH                'B'
#define  STRIP_LEADING             'L'
#define  STRIP_TRAILING            'T'
                                       /* Datatype function options  */
#define  DATATYPE_ALPHANUMERIC     'A'
#define  DATATYPE_BINARY           'B'
#define  DATATYPE_LOWERCASE        'L'
#define  DATATYPE_MIXEDCASE        'M'
#define  DATATYPE_NUMBER           'N'
#define  DATATYPE_SYMBOL           'S'
#define  DATATYPE_VARIABLE         'V'
#define  DATATYPE_UPPERCASE        'U'
#define  DATATYPE_WHOLE_NUMBER     'W'
#define  DATATYPE_HEX              'X'
#define  DATATYPE_9DIGITS          '9'
#define  DATATYPE_LOGICAL          'O' // lOgical.
                                       /* Verify function options    */
#define  VERIFY_MATCH              'M'
#define  VERIFY_NOMATCH            'N'

#define ch_SPACE ' '

                                       /* character validation sets for the */
                                       /* datatype function                 */
#define  HEX_CHAR_STR   "0123456789ABCDEFabcdef"
#define  ALPHANUM       \
"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
#define  BINARI         "01"
#define  LOWER_ALPHA    "abcdefghijklmnopqrstuvwxyz"
#define  MIXED_ALPHA    \
"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define  UPPER_ALPHA    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"


/*********************************************************************/
/*                                                                   */
/*      Name:                   IntToHexdigit                        */
/*                                                                   */
/*      Descriptive name:       convert int to hexadecimal digit     */
/*                                                                   */
/*      Returns:                A hexadecimal digit representing n.  */
/*                                                                   */
/*********************************************************************/

                                       /* convert the number                */
inline char IntToHexDigit(int n)
{
    return "0123456789ABCDEF"[n];
}


 class RexxString : public RexxObject {
  public:
   inline void       *operator new(size_t size, void *ptr){return ptr;};
   inline RexxString() {;} ;
   inline RexxString(RESTORETYPE restoreType) { ; };

   void        live(size_t);
   void        liveGeneral(int reason);
   void        flatten(RexxEnvelope *envelope);
   RexxObject *unflatten(RexxEnvelope *);

   virtual HashCode hash();
   virtual HashCode getHashValue();

   inline HashCode getStringHash()
   {
       if (hashValue == 0)                // if we've never generated this, the code is zero
       {
           stringsize_t len = this->getLength();

           HashCode h = 0;
           // the hash code is generated from all of the string characters.
           // we do this in a lazy fashion, since most string objects never need to
           // calculate a hash code, particularly the long ones.
           // This hashing algorithm is very similar to that used for Java strings.
           for (stringsize_t i = 0; i < len; i++)
           {
               h = 31 * h + this->stringData[i];
           }
           this->hashValue = h;
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
   RexxString  *primitiveMakeString();
   void         copyIntoTail(RexxCompoundTail *buffer);
   RexxString  *stringValue();
   bool         truthValue(int);
   virtual bool logicalValue(logical_t &);

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
   RexxObject *format(RexxObject *Integers, RexxObject *Decimals, RexxObject *MathExp, RexxObject *ExpTrigger);
   RexxObject *isInteger();
   RexxObject *logicalOperation(RexxObject *, RexxObject *, unsigned int);
   RexxString *extract(size_t offset, size_t sublength) { return newString(this->getStringData() + offset, sublength); }
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
   RexxString  *word(RexxInteger *);
   RexxInteger *wordIndex(RexxInteger *);
   RexxInteger *wordLength(RexxInteger *);
   RexxInteger *wordPos(RexxString *, RexxInteger *);
   RexxInteger *caselessWordPos(RexxString *, RexxInteger *);
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
   size_t       pos(RexxString *, size_t);
   size_t       caselessPos(RexxString *, size_t);

   RexxString  *translate(RexxString *, RexxString *, RexxString *, RexxInteger *, RexxInteger *);
   RexxInteger *verify(RexxString *, RexxString *, RexxInteger *, RexxInteger *);
   RexxInteger *countStrRexx(RexxString *);
   RexxInteger *caselessCountStrRexx(RexxString *);
   size_t       caselessCountStr(RexxString *);
                                       /* the following methods are in    */
                                       /* OKBBITS                         */
   RexxString  *bitAnd(RexxString *, RexxString *);
   RexxString  *bitOr(RexxString *, RexxString *);
   RexxString  *bitXor(RexxString *, RexxString *);
                                       /* the following methods are in    */
                                       /* OKBCONV                         */
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

   RexxArray   *makeArray(RexxString *);

/****************************************************************************/
/*                                                                          */
/*      RexxString Methods in OKBMISC.C                                     */
/*                                                                          */
/****************************************************************************/
   int         isSymbol();

/* Inline_functions */

   inline size_t  getLength() { return this->length; };
   inline void    setLength(size_t l) { this->length = l; };
   inline void finish(stringsize_t l) { length = l; }
   inline const char *getStringData() { return this->stringData; };
   inline char *getWritableData() { return &this->stringData[0]; };
   inline void put(size_t s, const void *b, size_t l) { memcpy(getWritableData() + s, b, l); };
   inline void put(size_t s, RexxString *o) { put(s, o->getStringData(), o->getLength()); };
   inline void set(size_t s,int c, size_t l) { memset((this->stringData+s), c, l); };
   inline char getChar(size_t p) { return *(this->stringData+p); };
   inline char putChar(size_t p,char c) { return *(this->stringData+p) = c; };
   inline bool upperOnly() {return (this->Attributes&STRING_NOLOWER) != 0;};
   inline bool hasLower() {return (this->Attributes&STRING_HASLOWER) != 0;};
   inline void  setUpperOnly() { this->Attributes |= STRING_NOLOWER;};
   inline void  setHasLower() { this->Attributes |= STRING_HASLOWER;};
   inline bool  nonNumeric() {return (this->Attributes&STRING_NONNUMERIC) != 0;};
   inline void  setNonNumeric() { this->Attributes |= STRING_NONNUMERIC;};
   inline bool  strCompare(const char * s) {return this->memCompare((s), strlen(s));};
   inline bool  strCaselessCompare(const char * s) { return (size_t)this->length == strlen(s) && Utilities::strCaselessCompare(s, this->stringData) == 0;}
   inline bool  memCompare(const char * s, size_t l) { return l == this->length && memcmp(s, this->stringData, l) == 0; }
   inline bool  memCompare(RexxString *other) { return other->length == this->length && memcmp(other->stringData, this->stringData, length) == 0; }
   inline void  memCopy(char * s) { memcpy(s, stringData, length); }
   inline void  toRxstring(CONSTRXSTRING &r) { r.strptr = getStringData(); r.strlength = getLength(); }
   inline void  toRxstring(RXSTRING &r) { r.strptr = getWritableData(); r.strlength = getLength(); }
          void  copyToRxstring(RXSTRING &r);

   RexxNumberString *createNumberString();

   inline RexxNumberString *fastNumberString() {
       if (this->nonNumeric())              /* Did we already try and convert to */
                                            /* to a numberstring and fail?       */
        return OREF_NULL;                   /* Yes, no need to try agian.        */

       if (this->NumberString != OREF_NULL) /* see if we have already converted  */
         return this->NumberString;         /* return the numberString Object.   */
       return createNumberString();         /* go build the number string version */
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

 protected:

   HashCode hashValue;                 // stored has value
   size_t length;                      /* string length                   */
   RexxNumberString *NumberString;     /* lookaside information           */
   size_t Attributes;                  /* string attributes               */
   char stringData[4];                 /* Start of the string data part   */
 };


// some handy functions for doing cstring/RexxString manipulations

 inline void * rmemcpy(void *t, RexxString *s, size_t len)
 {
     return memcpy(t, s->getStringData(), len);
 }

 inline int rmemcmp(const void *t, RexxString *s, size_t len)
 {
     return memcmp(t, s->getStringData(), len);
 }

 inline char * rstrcpy(char *t, RexxString *s)
 {
     return strcpy(t, s->getStringData());
 }

 inline char * rstrcat(char *t, RexxString *s)
 {
     return strcat(t, s->getStringData());
 }

 inline int rstrcmp(const char *t, RexxString *s)
 {
     return strcmp(t, s->getStringData());
 }


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
