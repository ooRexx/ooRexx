/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2024 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                                        */
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

class LanguageParser;

/**
 * A class for an entry in the various token keyword tables.
 */
class KeywordEntry
{
public:
    /**
     * Constructor for a keyword table entry.
     *
     * @param n      The string name of the keyword.
     * @param code   The associated keyword tag mapping.
     */
    inline KeywordEntry(const char *n, int code)
    {
        name = n;
        length = strlen(name);
        keywordCode = code;
    }

    const char *name;                       // the keyword name
    size_t      length;                     // length of the keyword string
    int         keywordCode;                // the string-to-identifier mapping
};


// major token types
typedef enum
{
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
typedef enum
{
    SUBTYPE_NONE = 0,

    // Operator sub-types.
    // NOTE:  The operator subtypes are used for table
    // lookups, so these need start with the index of
    // 1.
    OPERATOR_PLUS      = 1,
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

    // symbol characters
    SYMBOL_CONSTANT  =  100,
    SYMBOL_VARIABLE,
    SYMBOL_NAME,
    SYMBOL_COMPOUND,
    SYMBOL_STEM,
    SYMBOL_DUMMY,
    SYMBOL_DOTSYMBOL,
    INTEGER_CONSTANT,
    LITERAL_STRING,
    LITERAL_HEX,
    LITERAL_BIN,

    // special clause end types
    CLAUSEEND_EOF = 200,
    CLAUSEEND_SEMICOLON,
    CLAUSEEND_EOL,
    CLAUSEEND_NULL,

} TokenSubclass;

// various keyword enumerations

// the keyword instruction and directive identifiers
// IMPORTANT NOTE:  These constants are set into instruction
// objects.  To preserve release-to-release compiled image
// compatibility, new constants need to be added to the end
// of this list.
typedef enum
{
    KEYWORD_NONE = 0,
    KEYWORD_ADDRESS,
    KEYWORD_ARG,
    KEYWORD_CALL,
    KEYWORD_DO,
    KEYWORD_DROP,
    KEYWORD_EXIT,
    KEYWORD_IF,
    KEYWORD_INTERPRET,
    KEYWORD_ITERATE,
    KEYWORD_LEAVE,
    KEYWORD_NOP,
    KEYWORD_NUMERIC,
    KEYWORD_OPTIONS,
    KEYWORD_PARSE,
    KEYWORD_PROCEDURE,
    KEYWORD_PULL,
    KEYWORD_PUSH,
    KEYWORD_QUEUE,
    KEYWORD_REPLY,
    KEYWORD_RETURN,
    KEYWORD_SAY,
    KEYWORD_SELECT,
    KEYWORD_SIGNAL,
    KEYWORD_TRACE,
    KEYWORD_GUARD,
    KEYWORD_USE,
    KEYWORD_EXPOSE,
    KEYWORD_RAISE,
    KEYWORD_ELSE,
    KEYWORD_THEN,
    KEYWORD_END,
    KEYWORD_OTHERWISE,
    KEYWORD_IFTHEN,
    KEYWORD_WHENTHEN,
    KEYWORD_WHEN,
    KEYWORD_ASSIGNMENT,
    KEYWORD_COMMAND,
    KEYWORD_MESSAGE,
    KEYWORD_LABEL,
    KEYWORD_ENDIF,
    KEYWORD_BLOCK,
    KEYWORD_FIRST,
    KEYWORD_LAST,
    KEYWORD_ENDELSE,
    KEYWORD_ENDTHEN,
    KEYWORD_ENDWHEN,
    KEYWORD_INSTRUCTION,
    KEYWORD_FORWARD,
    KEYWORD_LOOP,
    KEYWORD_SIGNAL_VALUE,
    KEYWORD_SIGNAL_ON,
    KEYWORD_CALL_VALUE,
    KEYWORD_CALL_QUALIFIED,
    KEYWORD_CALL_ON,
    KEYWORD_SIMPLE_BLOCK,
    KEYWORD_LOOP_FOREVER,
    KEYWORD_LOOP_OVER,
    KEYWORD_LOOP_OVER_UNTIL,
    KEYWORD_LOOP_OVER_WHILE,
    KEYWORD_LOOP_OVER_FOR,
    KEYWORD_LOOP_OVER_FOR_UNTIL,
    KEYWORD_LOOP_OVER_FOR_WHILE,
    KEYWORD_LOOP_CONTROLLED,
    KEYWORD_LOOP_CONTROLLED_UNTIL,
    KEYWORD_LOOP_CONTROLLED_WHILE,
    KEYWORD_LOOP_COUNT,
    KEYWORD_LOOP_COUNT_UNTIL,
    KEYWORD_LOOP_COUNT_WHILE,
    KEYWORD_LOOP_WHILE,
    KEYWORD_LOOP_UNTIL,
    KEYWORD_LOOP_WITH,
    KEYWORD_LOOP_WITH_UNTIL,
    KEYWORD_LOOP_WITH_WHILE,
    KEYWORD_LOOP_WITH_FOR,
    KEYWORD_LOOP_WITH_FOR_UNTIL,
    KEYWORD_LOOP_WITH_FOR_WHILE,
    KEYWORD_MESSAGE_DOUBLE,
    KEYWORD_SELECT_CASE,
    KEYWORD_WHEN_CASE,
    KEYWORD_CLASS,
    KEYWORD_LIBRARY,
    KEYWORD_REQUIRES,
    KEYWORD_USE_LOCAL,
    KEYWORD_CONSTANT,
} InstructionKeyword;

// instruction subkeyword types
typedef enum
{
    SUBKEY_NONE = 0,
    SUBKEY_ARG,
    SUBKEY_BY,
    SUBKEY_DIGITS,
    SUBKEY_COUNTER,
    SUBKEY_END,
    SUBKEY_ELSE,
    SUBKEY_ENGINEERING,
    SUBKEY_EXPOSE,
    SUBKEY_FOR,
    SUBKEY_FOREVER,
    SUBKEY_FORM,
    SUBKEY_FUZZ,
    SUBKEY_INDEX,
    SUBKEY_ITEM,
    SUBKEY_LINEIN,
    SUBKEY_LOCAL,
    SUBKEY_LOWER,
    SUBKEY_CASELESS,
    SUBKEY_NAME,
    SUBKEY_NOVALUE,
    SUBKEY_OFF,
    SUBKEY_ON,
    SUBKEY_OTHERWISE,
    SUBKEY_OVER,
    SUBKEY_PULL,
    SUBKEY_SCIENTIFIC,
    SUBKEY_SOURCE,
    SUBKEY_THEN,
    SUBKEY_TO,
    SUBKEY_UNTIL,
    SUBKEY_UPPER,
    SUBKEY_VALUE,
    SUBKEY_VAR,
    SUBKEY_VERSION,
    SUBKEY_WHEN,
    SUBKEY_WHILE,
    SUBKEY_WITH,
    SUBKEY_DESCRIPTION,
    SUBKEY_ADDITIONAL,
    SUBKEY_RESULT,
    SUBKEY_ARRAY,
    SUBKEY_RETURN,
    SUBKEY_EXIT,
    SUBKEY_CONTINUE,
    SUBKEY_CLASS,
    SUBKEY_MESSAGE,
    SUBKEY_ARGUMENTS,
    SUBKEY_LABEL,
    SUBKEY_STRICT,
    SUBKEY_TRUE,
    SUBKEY_FALSE,
    SUBKEY_CASE,
    SUBKEY_INPUT,
    SUBKEY_OUTPUT,
    SUBKEY_ERROR,
    SUBKEY_STEM,
    SUBKEY_STREAM,
    SUBKEY_USING,
    SUBKEY_NORMAL,
    SUBKEY_APPEND,
    SUBKEY_REPLACE,
} InstructionSubKeyword;


/**
 * Keyword identifiers for directives.
 */
typedef enum
{
    DIRECTIVE_NONE = 0,
    DIRECTIVE_METHOD,
    DIRECTIVE_OPTIONS,
    DIRECTIVE_REQUIRES,
    DIRECTIVE_ROUTINE,
    DIRECTIVE_CLASS,
    DIRECTIVE_ATTRIBUTE,
    DIRECTIVE_LIBRARY,
    DIRECTIVE_CONSTANT,
    DIRECTIVE_RESOURCE,
    DIRECTIVE_ANNOTATE,
} DirectiveKeyword;


// Identify different token types returned by
// locateToken()
typedef enum
{
    NORMAL_CHAR,
    SIGNIFICANT_BLANK,
    CLAUSE_EOF,
    CLAUSE_EOL,
} CharacterClass;


/**
 * Keywords allowed on directive statements.
 */
typedef enum
{
    SUBDIRECTIVE_NONE = 0,
    SUBDIRECTIVE_PUBLIC,
    SUBDIRECTIVE_METACLASS,
    SUBDIRECTIVE_INHERIT,
    SUBDIRECTIVE_PRIVATE,
    SUBDIRECTIVE_GUARDED,
    SUBDIRECTIVE_CLASS,
    SUBDIRECTIVE_EXTERNAL,
    SUBDIRECTIVE_SUBCLASS,
    SUBDIRECTIVE_UNGUARDED,
    SUBDIRECTIVE_MIXINCLASS,
    SUBDIRECTIVE_ATTRIBUTE,
    SUBDIRECTIVE_PROTECTED,
    SUBDIRECTIVE_ABSTRACT,
    SUBDIRECTIVE_UNPROTECTED,
    SUBDIRECTIVE_GET,
    SUBDIRECTIVE_SET,
    SUBDIRECTIVE_LIBRARY,
    SUBDIRECTIVE_DIGITS,
    SUBDIRECTIVE_FORM,
    SUBDIRECTIVE_FUZZ,
    SUBDIRECTIVE_TRACE,
    SUBDIRECTIVE_NAMESPACE,
    SUBDIRECTIVE_END,
    SUBDIRECTIVE_NOVALUE,
    SUBDIRECTIVE_ERROR,
    SUBDIRECTIVE_CONDITION,
    SUBDIRECTIVE_PROLOG,
    SUBDIRECTIVE_NOPROLOG,
    SUBDIRECTIVE_PACKAGE,
    SUBDIRECTIVE_METHOD,
    SUBDIRECTIVE_ROUTINE,
    SUBDIRECTIVE_CONSTANT,
    SUBDIRECTIVE_DELEGATE,
    SUBDIRECTIVE_SYNTAX,
    SUBDIRECTIVE_ALL,
    SUBDIRECTIVE_FAILURE,
    SUBDIRECTIVE_LOSTDIGITS,
    SUBDIRECTIVE_NOSTRING,
    SUBDIRECTIVE_NOTREADY,
} DirectiveSubKeyword;


/**
 * Condition keyword posibilities
 */
typedef enum
{
    CONDITION_NONE = 0,
    CONDITION_ANY,
    CONDITION_ERROR,
    CONDITION_FAILURE,
    CONDITION_HALT,
    CONDITION_NOMETHOD,
    CONDITION_NOSTRING,
    CONDITION_NOTREADY,
    CONDITION_NOVALUE,
    CONDITION_PROPAGATE,
    CONDITION_SYNTAX,
    CONDITION_USER,
    CONDITION_LOSTDIGITS
} ConditionKeyword;


// markers for the builtin function
typedef enum
{
    NO_BUILTIN  = 0,
    BUILTIN_ABBREV,
    BUILTIN_ABS,
    BUILTIN_ADDRESS,
    BUILTIN_ARG,
    BUILTIN_B2X,
    BUILTIN_BITAND,
    BUILTIN_BITOR,
    BUILTIN_BITXOR,
    BUILTIN_C2D,
    BUILTIN_C2X,
    BUILTIN_CENTER,
    BUILTIN_CENTRE,
    BUILTIN_CHANGESTR,
    BUILTIN_CHARIN,
    BUILTIN_CHAROUT,
    BUILTIN_CHARS,
    BUILTIN_COMPARE,
    BUILTIN_CONDITION,
    BUILTIN_COPIES,
    BUILTIN_COUNTSTR,
    BUILTIN_D2C,
    BUILTIN_D2X,
    BUILTIN_DATATYPE,
    BUILTIN_DATE,
    BUILTIN_DELSTR,
    BUILTIN_DELWORD,
    BUILTIN_DIGITS,
    BUILTIN_ERRORTEXT,
    BUILTIN_FORM,
    BUILTIN_FORMAT,
    BUILTIN_FUZZ,
    BUILTIN_INSERT,
    BUILTIN_LASTPOS,
    BUILTIN_LEFT,
    BUILTIN_LENGTH,
    BUILTIN_LINEIN,
    BUILTIN_LINEOUT,
    BUILTIN_LINES,
    BUILTIN_MAX,
    BUILTIN_MIN,
    BUILTIN_OVERLAY,
    BUILTIN_POS,
    BUILTIN_QUEUED,
    BUILTIN_RANDOM,
    BUILTIN_REVERSE,
    BUILTIN_RIGHT,
    BUILTIN_RXQUEUE,
    BUILTIN_SIGN,
    BUILTIN_SOURCELINE,
    BUILTIN_SPACE,
    BUILTIN_STREAM,
    BUILTIN_STRIP,
    BUILTIN_SUBSTR,
    BUILTIN_SUBWORD,
    BUILTIN_SYMBOL,
    BUILTIN_TIME,
    BUILTIN_TRACE,
    BUILTIN_TRANSLATE,
    BUILTIN_TRUNC,
    BUILTIN_VALUE,
    BUILTIN_VAR,
    BUILTIN_VERIFY,
    BUILTIN_WORD,
    BUILTIN_WORDINDEX,
    BUILTIN_WORDLENGTH,
    BUILTIN_WORDPOS,
    BUILTIN_WORDS,
    BUILTIN_X2B,
    BUILTIN_X2C,
    BUILTIN_X2D,
    BUILTIN_XRANGE,
    BUILTIN_USERID,
    BUILTIN_LOWER,
    BUILTIN_UPPER,
    BUILTIN_RXFUNCADD,
    BUILTIN_RXFUNCDROP,
    BUILTIN_RXFUNCQUERY,
    BUILTIN_ENDLOCAL,
    BUILTIN_SETLOCAL,
    BUILTIN_QUALIFY,
} BuiltinCode;


/******************************************************************************/
/* various expression terminator sets                                         */
/******************************************************************************/
#define   TERM_EOC     0x00000001u     // terminate on end of clause
#define   TERM_RIGHT   0x00000002u     // terminate on left paren
#define   TERM_SQRIGHT 0x00000004u     // terminate on left square bracket
#define   TERM_TO      0x00000008u     // terminate on TO keyword
#define   TERM_BY      0x00000010u     // terminate on BY keyword
#define   TERM_FOR     0x00000020u     // terminate on FOR keyword
#define   TERM_WHILE   0x00000040u     // terminate on WHILE/UNTIL keywords
#define   TERM_WITH    0x00000100u     // terminate on WITH keyword
#define   TERM_THEN    0x00000200u     // terminate on THEN keyword
#define   TERM_KEYWORD 0x10000000u     // perform keyword terminator checks
                                       // terminate on controlled DO keywords
#define   TERM_CONTROL (TERM_KEYWORD | TERM_TO | TERM_BY | TERM_FOR | TERM_WHILE)
                                       // terminate on DO conditionals
#define   TERM_COND    (TERM_KEYWORD | TERM_WHILE)
                                       // terminators for an OVER condition
#define   TERM_OVER    (TERM_KEYWORD | TERM_FOR | TERM_WHILE)
                                       // terminate on a THEN keyword
#define   TERM_IF      (TERM_KEYWORD | TERM_THEN)


class RexxToken : public RexxInternalObject
{
 public:
    void        *operator new(size_t);
    inline void  operator delete(void *) { ; }

    inline RexxToken() { }
    inline RexxToken(TokenClass c, SourceLocation &l, TokenSubclass sc = SUBTYPE_NONE, RexxString *v = OREF_NULL) : classId(c), subclass(sc),
        stringValue(v), numeric(SUBTYPE_NONE), tokenLocation(l) { };

    inline RexxToken(RESTORETYPE restoreType) { ; };

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;

    inline void setStart(size_t l, size_t o) { tokenLocation.setStart(l, o); }
    inline void setEnd(size_t l, size_t o) { tokenLocation.setEnd(l, o); }
    inline size_t getLineNumber() { return tokenLocation.getLineNumber(); }

    inline bool       isType(TokenClass t) { return classId == t; }
    inline bool       isType(TokenClass t1, TokenClass t2) { return classId == t1 || classId == t2; }
    inline bool       isType(TokenClass t1, TokenClass t2, TokenClass t3) { return classId == t1 || classId == t2 || classId == t3; }
    inline bool       isSubtype(TokenSubclass t) { return subclass == t; }
    inline bool       isSubtype(TokenSubclass t1, TokenSubclass t2) { return subclass == t1 || subclass == t2; }
    inline TokenClass type() { return classId; }
    inline TokenSubclass  subtype() { return subclass; }
    inline RexxString *value() { return stringValue; }
           RexxString *upperValue();
    inline void       setType(TokenClass t) { classId = t; }
    inline void       setSubtype(TokenSubclass t) { subclass = t; }
    inline void       setValue(RexxString *v) { stringValue = v; }
    inline bool       isVariable() { return (subclass == SYMBOL_VARIABLE || subclass == SYMBOL_STEM || subclass == SYMBOL_COMPOUND); };
    inline bool       isStem() { return (subclass == SYMBOL_STEM); }
    inline bool       isNonCompoundVariable() { return subclass == SYMBOL_VARIABLE || subclass == SYMBOL_STEM; };
    inline bool       isSimpleVariable() { return subclass == SYMBOL_VARIABLE; };
    inline bool       isVariableOrDot() { return isVariable() || subclass == SYMBOL_DOTSYMBOL; };
    inline bool       isDotSymbol() { return (subclass == SYMBOL_DOTSYMBOL); }
    inline bool       isDot() { return (subclass == SYMBOL_DUMMY); }
    inline bool       isLiteral()  { return classId == TOKEN_LITERAL; };
    inline bool       isSymbolOrLiteral()  { return classId == TOKEN_LITERAL || this->classId == TOKEN_SYMBOL; };
    inline bool       isConstant()  { return (classId == TOKEN_SYMBOL && subclass != SYMBOL_VARIABLE && subclass != SYMBOL_STEM && subclass != SYMBOL_COMPOUND); };
    inline bool       isNumericSymbol()  { return (subclass == SYMBOL_CONSTANT); }
    inline bool       isIntegerConstant() { return numeric == INTEGER_CONSTANT; }
    inline bool       isSymbol() { return classId == TOKEN_SYMBOL; };
    inline bool       isOperator() { return classId == TOKEN_OPERATOR; }
    inline bool       isOperator(TokenSubclass t) { return isType(TOKEN_OPERATOR) && isSubtype(t); }
    inline bool       isBlank() { return classId == TOKEN_BLANK; }
    inline bool       isComma() { return classId == TOKEN_COMMA; }
    inline bool       isLeftParen() { return classId == TOKEN_LEFT; }
    inline bool       isRightParen() { return classId == TOKEN_RIGHT; }
    inline bool       isLeftBracket() { return classId == TOKEN_SQLEFT; }
    inline bool       isRightBracket() { return classId == TOKEN_SQRIGHT; }
    inline bool       isEndOfClause() { return classId == TOKEN_EOC; }
    inline bool       isEndOfFile() { return isEndOfClause() && isSubtype(CLAUSEEND_EOF); }
    inline bool       isBlankSignificant() { return (classId == TOKEN_SYMBOL || classId == TOKEN_LITERAL ||
         classId == TOKEN_RIGHT || classId == TOKEN_SQRIGHT); }
    inline bool       isMessageOperator() { return classId == TOKEN_TILDE || classId == TOKEN_DTILDE || classId == TOKEN_SQLEFT; }
    inline void       setNumeric(TokenSubclass v)   { this->numeric = v; };
    inline const SourceLocation &getLocation() { return tokenLocation; }
    inline void       setLocation(const SourceLocation &l) { tokenLocation = l; }
           void       checkAssignment(LanguageParser *parser, RexxString *newValue);
           int        precedence();
           bool       isTerminator(int);
           RexxString *displayValue();

           InstructionKeyword keyword();
           InstructionSubKeyword subKeyword();
           BuiltinCode builtin();
           ConditionKeyword condition();
           InstructionSubKeyword parseOption();
           DirectiveKeyword keyDirective();
           DirectiveSubKeyword subDirective();
    static BuiltinCode resolveBuiltin(RexxString *value);

protected:

    static int resolveKeyword(RexxString *token, KeywordEntry *table, size_t table_size);

    RexxString   *stringValue;             // token string value
    TokenClass    classId;                 // class of token
    TokenSubclass subclass;                // specialized type of token
    TokenSubclass numeric;                 // even further specialization
    SourceLocation tokenLocation;          // token source location

    // Tables of different keywords using in various contexts.
    static KeywordEntry directives[];
    static KeywordEntry keywordInstructions[];
    static KeywordEntry subKeywords[];
    static KeywordEntry builtinFunctions[];
    static KeywordEntry conditionKeywords[];
    static KeywordEntry parseOptions[];
    static KeywordEntry subDirectives[];
};

inline RexxToken *new_token(TokenClass c, TokenSubclass s, RexxString *v, SourceLocation &l) { return new RexxToken (c, l, s, v); }

#endif
