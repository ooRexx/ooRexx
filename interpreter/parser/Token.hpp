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
/* Primitive Translator Token Class Definitions                               */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxToken
#define Included_RexxToken

#include "SourceLocation.hpp"


// major token types
enum {
    TOKEN_NULL       = 1,
    TOKEN_BLANK,
    TOKEN_SYMBOL,
    TOKEN_LITERAL,
    TOKEN_OPERATOR,
    TOKEN_EOC,
    TOKEN_COMMA,
    TOKEN_PREFIX,
    TOKEN_LEFT,
    TOKEN_RIGHT,
    TOKEN_POINT,
    TOKEN_COLON,
    TOKEN_TILDE,
    TOKEN_DTILDE,
    TOKEN_SQLEFT,
    TOKEN_SQRIGHT,
    TOKEN_DCOLON,
    TOKEN_CONTINUE,
    TOKEN_ASSIGNMENT,
} TokenClass;

// token extended types.  Each category gets is own
// section of the numeric range.
enum {
    SUBTYPE_NONE = 0,

    // symbol characters
    SYMBOL_CONSTANT  =  100,
    SYMBOL_VARIABLE,
    SYMBOL_NAME,
    SYMBOL_COMPOUND,
    SYMBOL_STEM,
    SYMBOL_DUMMY,
    SYMBOL_DOTSYMBOL,
    INTEGER_CONSTANT,
    LITERAL_HEX,
    LITERAL_BIN,

    // Operator sub-types
    OPERATOR_PLUS      = 200,
    OPERATOR_SUBTRACT,
    OPERATOR_MULTIPLY,
    OPERATOR_DIVIDE,
    OPERATOR_INTDIV,
    OPERATOR_REMAINDER,
    OPERATOR_POWER,
    OPERATOR_ABUTTAL,
    OPERATOR_CONCATENATE,
    OPERATOR_BLANK,
    OPERATOR_EQUAL,
    OPERATOR_BACKSLASH_EQUAL,
    OPERATOR_GREATERTHAN,
    OPERATOR_BACKSLASH_GREATERTHAN,
    OPERATOR_LESSTHAN,
    OPERATOR_BACKSLASH_LESSTHAN,
    OPERATOR_GREATERTHAN_EQUAL,
    OPERATOR_LESSTHAN_EQUAL,
    OPERATOR_STRICT_EQUAL,
    OPERATOR_STRICT_BACKSLASH_EQUAL,
    OPERATOR_STRICT_GREATERTHAN,
    OPERATOR_STRICT_BACKSLASH_GREATERTHAN,
    OPERATOR_STRICT_LESSTHAN,
    OPERATOR_STRICT_BACKSLASH_LESSTHAN,
    OPERATOR_STRICT_GREATERTHAN_EQUAL,
    OPERATOR_STRICT_LESSTHAN_EQUAL,
    OPERATOR_LESSTHAN_GREATERTHAN,
    OPERATOR_GREATERTHAN_LESSTHAN,
    OPERATOR_AND,
    OPERATOR_OR,
    OPERATOR_XOR,
    OPERATOR_BACKSLASH,
} TokenSubclass;


class RexxToken : public RexxInternalObject {
 public:
    void        *operator new(size_t);
    inline void *operator new(size_t size, void *ptr) {return ptr;};
    inline void  operator delete(void *) { ; }
    inline void  operator delete(void *, void *) { ; }

    inline RexxToken(TokenClass c, SourceLocation &l, TokenSubclass sc = SUBTYPE_NONE, RexxString *v = OREF_NULL) : class(c), subclass(sc),
        value(v), numeric(SUBTYPE_NONE), location(l) { };

    inline RexxToken(RESTORETYPE restoreType) { ; };
    void       live(size_t);
    void       liveGeneral(int reason);

    inline void setStart(size_t l, size_t o) { tokenLocation.setStart(l, o); }
    inline void setEnd(size_t l, size_t o) { tokenLocation.setEnd(l, o); }

    inline bool       isVariable() { return (subclass == SYMBOL_VARIABLE || subclass == SYMBOL_STEM || subclass == SYMBOL_COMPOUND); };
    inline bool       isSimpleVariable() { return subclass == SYMBOL_VARIABLE; };
    inline bool       isDot() { return (subclass == SYMBOL_DOTSYMBOL); }
    inline bool       isLiteral()  { return classId == TOKEN_LITERAL; };
    inline bool       isSymbolOrLiteral()  { return classId == TOKEN_LITERAL || this->classId == TOKEN_SYMBOL; };
    inline bool       isConstant()  { return (classId == TOKEN_SYMBOL && subclass != SYMBOL_VARIABLE && subclass != SYMBOL_STEM && subclass != SYMBOL_COMPOUND); };
    inline bool       isSymbol() { return classId == TOKEN_SYMBOL; };
    inline bool       isOperator() { return classId == TOKEN_OPERATOR; }
    inline bool       isEndOfClause() { return classId == TOKEN_EOC; }
    inline void       setNumeric(TokenSubclass v)   { this->numeric = v; };
    inline const SourceLocation &getLocation() { return tokenLocation; }
    inline void  setLocation(SourceLocation &l) { tokenLocation = l; }
           void       checkAssignment(RexxSource *source, RexxString *newValue);

    SourceLocation tokenLocation;          // token source location
    RexxString   *value;                   // token string value
    TokenClass    classId;                 // class of token
    TokenSubclass subclass;                // specialized type of token
    TokenSubclass numeric;                 // even further specialization
};


inline RexxArray *new_arrayOfTokens(size_t n) { return memoryObject.newObjects(sizeof(RexxToken), n, T_Token); }
inline RexxToken *new_token(int c, int s, RexxString *v, SourceLocation &l) { return new RexxToken (c, s, v, l); }

#endif
