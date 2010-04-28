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
/* Parser keyword constants                                                   */
/*                                                                            */
/*   NOTE!!! It is critical for all the following tables to be                */
/*           in ASCII alphabetic order.                                       */
/*                                                                            */
/******************************************************************************/

#define DEFINING
#include "RexxCore.h"
#include "SourceFile.hpp"
#include "Token.hpp"

/*********************************************************************/
/*                                                                   */
/* Table of directive instructions used for translation              */
/*                                                                   */
/*********************************************************************/

KeywordEntry RexxSource::directives[] = {              /* language directive table          */
    KeywordEntry(CHAR_ATTRIBUTE,   DIRECTIVE_ATTRIBUTE),
    KeywordEntry(CHAR_CLASS,       DIRECTIVE_CLASS),
    KeywordEntry(CHAR_CONSTANT,    DIRECTIVE_CONSTANT),
    KeywordEntry(CHAR_METHOD,      DIRECTIVE_METHOD),
    KeywordEntry(CHAR_OPTIONS,     DIRECTIVE_OPTIONS),
    KeywordEntry(CHAR_REQUIRES,    DIRECTIVE_REQUIRES),
    KeywordEntry(CHAR_ROUTINE,     DIRECTIVE_ROUTINE),
};

/*********************************************************************/
/*                                                                   */
/* Table of keyword instructions used for translation                */
/*                                                                   */
/*********************************************************************/

KeywordEntry RexxSource::keywordInstructions[] = {     /* language keyword table     */
   KeywordEntry(CHAR_ADDRESS,    KEYWORD_ADDRESS),
   KeywordEntry(CHAR_ARG,        KEYWORD_ARG),
   KeywordEntry(CHAR_CALL,       KEYWORD_CALL),
   KeywordEntry(CHAR_DO,         KEYWORD_DO),
   KeywordEntry(CHAR_DROP,       KEYWORD_DROP),
   KeywordEntry(CHAR_ELSE,       KEYWORD_ELSE),
   KeywordEntry(CHAR_END,        KEYWORD_END),
   KeywordEntry(CHAR_EXIT,       KEYWORD_EXIT),
   KeywordEntry(CHAR_EXPOSE,     KEYWORD_EXPOSE),
   KeywordEntry(CHAR_FORWARD,    KEYWORD_FORWARD),
   KeywordEntry(CHAR_GUARD,      KEYWORD_GUARD),
   KeywordEntry(CHAR_IF,         KEYWORD_IF),
   KeywordEntry(CHAR_INTERPRET,  KEYWORD_INTERPRET),
   KeywordEntry(CHAR_ITERATE,    KEYWORD_ITERATE),
   KeywordEntry(CHAR_LEAVE,      KEYWORD_LEAVE),
   KeywordEntry(CHAR_LOOP,       KEYWORD_LOOP),
   KeywordEntry(CHAR_NOP,        KEYWORD_NOP),
   KeywordEntry(CHAR_NUMERIC,    KEYWORD_NUMERIC),
   KeywordEntry(CHAR_OPTIONS,    KEYWORD_OPTIONS),
   KeywordEntry(CHAR_OTHERWISE,  KEYWORD_OTHERWISE),
   KeywordEntry(CHAR_PARSE,      KEYWORD_PARSE),
   KeywordEntry(CHAR_PROCEDURE,  KEYWORD_PROCEDURE),
   KeywordEntry(CHAR_PULL,       KEYWORD_PULL),
   KeywordEntry(CHAR_PUSH,       KEYWORD_PUSH),
   KeywordEntry(CHAR_QUEUE,      KEYWORD_QUEUE),
   KeywordEntry(CHAR_RAISE,      KEYWORD_RAISE),
   KeywordEntry(CHAR_REPLY,      KEYWORD_REPLY),
   KeywordEntry(CHAR_RETURN,     KEYWORD_RETURN),
   KeywordEntry(CHAR_SAY,        KEYWORD_SAY),
   KeywordEntry(CHAR_SELECT,     KEYWORD_SELECT),
   KeywordEntry(CHAR_SIGNAL,     KEYWORD_SIGNAL),
   KeywordEntry(CHAR_THEN,       KEYWORD_THEN),
   KeywordEntry(CHAR_TRACE,      KEYWORD_TRACE),
   KeywordEntry(CHAR_USE,        KEYWORD_USE),
   KeywordEntry(CHAR_WHEN,       KEYWORD_WHEN),
};

/*********************************************************************/
/*                                                                   */
/* Table of instruction subkeywords used for translation             */
/*                                                                   */
/*********************************************************************/

KeywordEntry RexxSource::subKeywords[] = {             /* language keyword table     */
   KeywordEntry(CHAR_ADDITIONAL,  SUBKEY_ADDITIONAL),
   KeywordEntry(CHAR_ARG,         SUBKEY_ARG),
   KeywordEntry(CHAR_ARGUMENTS,   SUBKEY_ARGUMENTS),
   KeywordEntry(CHAR_ARRAY,       SUBKEY_ARRAY),
   KeywordEntry(CHAR_BY,          SUBKEY_BY),
   KeywordEntry(CHAR_CLASS,       SUBKEY_CLASS),
   KeywordEntry(CHAR_CONTINUE,    SUBKEY_CONTINUE),
   KeywordEntry(CHAR_DESCRIPTION, SUBKEY_DESCRIPTION),
   KeywordEntry(CHAR_DIGITS,      SUBKEY_DIGITS),
   KeywordEntry(CHAR_ENGINEERING, SUBKEY_ENGINEERING),
   KeywordEntry(CHAR_EXIT,        SUBKEY_EXIT),
   KeywordEntry(CHAR_EXPOSE,      SUBKEY_EXPOSE),
   KeywordEntry(CHAR_FALSE,       SUBKEY_FALSE),
   KeywordEntry(CHAR_FOR,         SUBKEY_FOR),
   KeywordEntry(CHAR_FOREVER,     SUBKEY_FOREVER),
   KeywordEntry(CHAR_FORM,        SUBKEY_FORM),
   KeywordEntry(CHAR_FUZZ,        SUBKEY_FUZZ),
   KeywordEntry(CHAR_LABEL,       SUBKEY_LABEL),
   KeywordEntry(CHAR_MESSAGE,     SUBKEY_MESSAGE),
   KeywordEntry(CHAR_NAME,        SUBKEY_NAME),
   KeywordEntry(CHAR_OFF,         SUBKEY_OFF),
   KeywordEntry(CHAR_ON,          SUBKEY_ON),
   KeywordEntry(CHAR_OVER,        SUBKEY_OVER),
   KeywordEntry(CHAR_RETURN,      SUBKEY_RETURN),
   KeywordEntry(CHAR_SCIENTIFIC,  SUBKEY_SCIENTIFIC),
   KeywordEntry(CHAR_STRICT,      SUBKEY_STRICT),
   KeywordEntry(CHAR_THEN,        SUBKEY_THEN),
   KeywordEntry(CHAR_TO,          SUBKEY_TO),
   KeywordEntry(CHAR_TRUE,        SUBKEY_TRUE),
   KeywordEntry(CHAR_UNTIL,       SUBKEY_UNTIL),
   KeywordEntry(CHAR_VALUE,       SUBKEY_VALUE),
   KeywordEntry(CHAR_WHEN,        SUBKEY_WHEN),
   KeywordEntry(CHAR_WHILE,       SUBKEY_WHILE),
   KeywordEntry(CHAR_WITH,        SUBKEY_WITH),
};

/*************************************************************************/
/*                                                                   */
/* Table of built-in functions used for translation                  */
/*                                                                   */
/*********************************************************************/

KeywordEntry RexxSource::builtinFunctions[] = {        /* built-in function table    */
#ifdef EBCDIC
    KeywordEntry(CHAR_QUEUEEXIT,   BUILTIN_QUEUEEXIT),
    KeywordEntry(CHAR_ABBREV,      BUILTIN_ABBREV),
    KeywordEntry(CHAR_ABS,         BUILTIN_ABS),
    KeywordEntry(CHAR_ADDRESS,     BUILTIN_ADDRESS),
    KeywordEntry(CHAR_ARG,         BUILTIN_ARG),
    KeywordEntry(CHAR_BITAND,      BUILTIN_BITAND),
    KeywordEntry(CHAR_BITOR,       BUILTIN_BITOR),
    KeywordEntry(CHAR_BITXOR,      BUILTIN_BITXOR),
    KeywordEntry(CHAR_B2X,         BUILTIN_B2X),
    KeywordEntry(CHAR_CENTER,      BUILTIN_CENTER),
    KeywordEntry(CHAR_CENTRE,      BUILTIN_CENTRE),
    KeywordEntry(CHAR_CHANGESTR,   BUILTIN_CHANGESTR),
    KeywordEntry(CHAR_CHARIN,      BUILTIN_CHARIN),
    KeywordEntry(CHAR_CHAROUT,     BUILTIN_CHAROUT),
    KeywordEntry(CHAR_CHARS,       BUILTIN_CHARS),
    KeywordEntry(CHAR_COMPARE,     BUILTIN_COMPARE),
    KeywordEntry(CHAR_CONDITION,   BUILTIN_CONDITION),
    KeywordEntry(CHAR_COPIES,      BUILTIN_COPIES),
    KeywordEntry(CHAR_COUNTSTR,    BUILTIN_COUNTSTR),
    KeywordEntry(CHAR_DATATYPE,    BUILTIN_DATATYPE),
    KeywordEntry(CHAR_C2D,         BUILTIN_C2D),
    KeywordEntry(CHAR_C2X,         BUILTIN_C2X),
    KeywordEntry(CHAR_DATE,        BUILTIN_DATE),
    KeywordEntry(CHAR_DELSTR,      BUILTIN_DELSTR),
    KeywordEntry(CHAR_DELWORD,     BUILTIN_DELWORD),
    KeywordEntry(CHAR_DIGITS,      BUILTIN_DIGITS),
    KeywordEntry(CHAR_D2C,         BUILTIN_D2C),
    KeywordEntry(CHAR_D2X,         BUILTIN_D2X),
    KeywordEntry(CHAR_ENDLOCAL,    BUILTIN_ENDLOCAL),
    KeywordEntry(CHAR_ERRORTEXT,   BUILTIN_ERRORTEXT),
    KeywordEntry(CHAR_FORM,        BUILTIN_FORM),
    KeywordEntry(CHAR_FORMAT,      BUILTIN_FORMAT),
    KeywordEntry(CHAR_FUZZ,        BUILTIN_FUZZ),
    KeywordEntry(CHAR_INSERT,      BUILTIN_INSERT),
    KeywordEntry(CHAR_LASTPOS,     BUILTIN_LASTPOS),
    KeywordEntry(CHAR_LEFT,        BUILTIN_LEFT),
    KeywordEntry(CHAR_LENGTH,      BUILTIN_LENGTH),
    KeywordEntry(CHAR_LINEIN,      BUILTIN_LINEIN),
    KeywordEntry(CHAR_LINEOUT,     BUILTIN_LINEOUT),
    KeywordEntry(CHAR_LINES,       BUILTIN_LINES),
    KeywordEntry(CHAR_LOWER,       BUILTIN_LOWER),
    KeywordEntry(CHAR_ORXMAX,         BUILTIN_MAX),
    KeywordEntry(CHAR_ORXMIN,         BUILTIN_MIN),
    KeywordEntry(CHAR_OVERLAY,     BUILTIN_OVERLAY),
    KeywordEntry(CHAR_POS,         BUILTIN_POS),
    KeywordEntry(CHAR_QUALIFY,     BUILTIN_QUALIFY),
    KeywordEntry(CHAR_QUEUED,      BUILTIN_QUEUED),
    KeywordEntry(CHAR_RANDOM,      BUILTIN_RANDOM),
    KeywordEntry(CHAR_REVERSE,     BUILTIN_REVERSE),
    KeywordEntry(CHAR_RIGHT,       BUILTIN_RIGHT),
    KeywordEntry(CHAR_RXFUNCADD,   BUILTIN_RXFUNCADD),
    KeywordEntry(CHAR_RXFUNCDROP,  BUILTIN_RXFUNCDROP),
    KeywordEntry(CHAR_RXFUNCQUERY, BUILTIN_RXFUNCQUERY),
    KeywordEntry(CHAR_SETLOCAL,    BUILTIN_SETLOCAL),
    KeywordEntry(CHAR_SIGN,        BUILTIN_SIGN),
    KeywordEntry(CHAR_SOURCELINE,  BUILTIN_SOURCELINE),
    KeywordEntry(CHAR_SPACE,       BUILTIN_SPACE),
    KeywordEntry(CHAR_STREAM,      BUILTIN_STREAM),
    KeywordEntry(CHAR_STRIP,       BUILTIN_STRIP),
    KeywordEntry(CHAR_SUBSTR,      BUILTIN_SUBSTR),
    KeywordEntry(CHAR_SUBWORD,     BUILTIN_SUBWORD),
    KeywordEntry(CHAR_SYMBOL,      BUILTIN_SYMBOL),
    KeywordEntry(CHAR_TIME,        BUILTIN_TIME),
    KeywordEntry(CHAR_TRACE,       BUILTIN_TRACE),
    KeywordEntry(CHAR_TRANSLATE,   BUILTIN_TRANSLATE),
    KeywordEntry(CHAR_TRUNC,       BUILTIN_TRUNC),
    KeywordEntry(CHAR_UPPER,       BUILTIN_UPPER),
    KeywordEntry(CHAR_USERID,      BUILTIN_USERID),
    KeywordEntry(CHAR_VALUE,       BUILTIN_VALUE),
    KeywordEntry(CHAR_VAR,         BUILTIN_VAR),
    KeywordEntry(CHAR_VERIFY,      BUILTIN_VERIFY),
    KeywordEntry(CHAR_WORD,        BUILTIN_WORD),
    KeywordEntry(CHAR_WORDINDEX,   BUILTIN_WORDINDEX),
    KeywordEntry(CHAR_WORDLENGTH,  BUILTIN_WORDLENGTH),
    KeywordEntry(CHAR_WORDPOS,     BUILTIN_WORDPOS),
    KeywordEntry(CHAR_WORDS,       BUILTIN_WORDS),
    KeywordEntry(CHAR_XRANGE,      BUILTIN_XRANGE),
    KeywordEntry(CHAR_X2B,         BUILTIN_X2B),
    KeywordEntry(CHAR_X2C,         BUILTIN_X2C),
    KeywordEntry(CHAR_X2D,         BUILTIN_X2D),
#else
    KeywordEntry(CHAR_QUEUEEXIT,   BUILTIN_QUEUEEXIT),
    KeywordEntry(CHAR_ABBREV,      BUILTIN_ABBREV),
    KeywordEntry(CHAR_ABS,         BUILTIN_ABS),
    KeywordEntry(CHAR_ADDRESS,     BUILTIN_ADDRESS),
    KeywordEntry(CHAR_ARG,         BUILTIN_ARG),
    KeywordEntry(CHAR_B2X,         BUILTIN_B2X),
    KeywordEntry(CHAR_BITAND,      BUILTIN_BITAND),
    KeywordEntry(CHAR_BITOR,       BUILTIN_BITOR),
    KeywordEntry(CHAR_BITXOR,      BUILTIN_BITXOR),
    KeywordEntry(CHAR_C2D,         BUILTIN_C2D),
    KeywordEntry(CHAR_C2X,         BUILTIN_C2X),
    KeywordEntry(CHAR_CENTER,      BUILTIN_CENTER),
    KeywordEntry(CHAR_CENTRE,      BUILTIN_CENTRE),
    KeywordEntry(CHAR_CHANGESTR,   BUILTIN_CHANGESTR),
    KeywordEntry(CHAR_CHARIN,      BUILTIN_CHARIN),
    KeywordEntry(CHAR_CHAROUT,     BUILTIN_CHAROUT),
    KeywordEntry(CHAR_CHARS,       BUILTIN_CHARS),
    KeywordEntry(CHAR_COMPARE,     BUILTIN_COMPARE),
    KeywordEntry(CHAR_CONDITION,   BUILTIN_CONDITION),
    KeywordEntry(CHAR_COPIES,      BUILTIN_COPIES),
    KeywordEntry(CHAR_COUNTSTR,    BUILTIN_COUNTSTR),
    KeywordEntry(CHAR_D2C,         BUILTIN_D2C),
    KeywordEntry(CHAR_D2X,         BUILTIN_D2X),
    KeywordEntry(CHAR_DATATYPE,    BUILTIN_DATATYPE),
    KeywordEntry(CHAR_DATE,        BUILTIN_DATE),
    KeywordEntry(CHAR_DELSTR,      BUILTIN_DELSTR),
    KeywordEntry(CHAR_DELWORD,     BUILTIN_DELWORD),
    KeywordEntry(CHAR_DIGITS,      BUILTIN_DIGITS),
    KeywordEntry(CHAR_ENDLOCAL,    BUILTIN_ENDLOCAL),
    KeywordEntry(CHAR_ERRORTEXT,   BUILTIN_ERRORTEXT),
    KeywordEntry(CHAR_FORM,        BUILTIN_FORM),
    KeywordEntry(CHAR_FORMAT,      BUILTIN_FORMAT),
    KeywordEntry(CHAR_FUZZ,        BUILTIN_FUZZ),
    KeywordEntry(CHAR_INSERT,      BUILTIN_INSERT),
    KeywordEntry(CHAR_LASTPOS,     BUILTIN_LASTPOS),
    KeywordEntry(CHAR_LEFT,        BUILTIN_LEFT),
    KeywordEntry(CHAR_LENGTH,      BUILTIN_LENGTH),
    KeywordEntry(CHAR_LINEIN,      BUILTIN_LINEIN),
    KeywordEntry(CHAR_LINEOUT,     BUILTIN_LINEOUT),
    KeywordEntry(CHAR_LINES,       BUILTIN_LINES),
    KeywordEntry(CHAR_LOWER,       BUILTIN_LOWER),
    KeywordEntry(CHAR_ORXMAX,         BUILTIN_MAX),
    KeywordEntry(CHAR_ORXMIN,         BUILTIN_MIN),
    KeywordEntry(CHAR_OVERLAY,     BUILTIN_OVERLAY),
    KeywordEntry(CHAR_POS,         BUILTIN_POS),
    KeywordEntry(CHAR_QUALIFY,     BUILTIN_QUALIFY),
    KeywordEntry(CHAR_QUEUED,      BUILTIN_QUEUED),
    KeywordEntry(CHAR_RANDOM,      BUILTIN_RANDOM),
    KeywordEntry(CHAR_REVERSE,     BUILTIN_REVERSE),
    KeywordEntry(CHAR_RIGHT,       BUILTIN_RIGHT),
    KeywordEntry(CHAR_RXFUNCADD,   BUILTIN_RXFUNCADD),
    KeywordEntry(CHAR_RXFUNCDROP,  BUILTIN_RXFUNCDROP),
    KeywordEntry(CHAR_RXFUNCQUERY, BUILTIN_RXFUNCQUERY),
    KeywordEntry(CHAR_SETLOCAL,    BUILTIN_SETLOCAL),
    KeywordEntry(CHAR_SIGN,        BUILTIN_SIGN),
    KeywordEntry(CHAR_SOURCELINE,  BUILTIN_SOURCELINE),
    KeywordEntry(CHAR_SPACE,       BUILTIN_SPACE),
    KeywordEntry(CHAR_STREAM,      BUILTIN_STREAM),
    KeywordEntry(CHAR_STRIP,       BUILTIN_STRIP),
    KeywordEntry(CHAR_SUBSTR,      BUILTIN_SUBSTR),
    KeywordEntry(CHAR_SUBWORD,     BUILTIN_SUBWORD),
    KeywordEntry(CHAR_SYMBOL,      BUILTIN_SYMBOL),
    KeywordEntry(CHAR_TIME,        BUILTIN_TIME),
    KeywordEntry(CHAR_TRACE,       BUILTIN_TRACE),
    KeywordEntry(CHAR_TRANSLATE,   BUILTIN_TRANSLATE),
    KeywordEntry(CHAR_TRUNC,       BUILTIN_TRUNC),
    KeywordEntry(CHAR_UPPER,       BUILTIN_UPPER),
    KeywordEntry(CHAR_USERID,      BUILTIN_USERID),
    KeywordEntry(CHAR_VALUE,       BUILTIN_VALUE),
    KeywordEntry(CHAR_VAR,         BUILTIN_VAR),
    KeywordEntry(CHAR_VERIFY,      BUILTIN_VERIFY),
    KeywordEntry(CHAR_WORD,        BUILTIN_WORD),
    KeywordEntry(CHAR_WORDINDEX,   BUILTIN_WORDINDEX),
    KeywordEntry(CHAR_WORDLENGTH,  BUILTIN_WORDLENGTH),
    KeywordEntry(CHAR_WORDPOS,     BUILTIN_WORDPOS),
    KeywordEntry(CHAR_WORDS,       BUILTIN_WORDS),
    KeywordEntry(CHAR_X2B,         BUILTIN_X2B),
    KeywordEntry(CHAR_X2C,         BUILTIN_X2C),
    KeywordEntry(CHAR_X2D,         BUILTIN_X2D),
    KeywordEntry(CHAR_XRANGE,      BUILTIN_XRANGE),
#endif
};

/*********************************************************************/
/*                                                                   */
/* Table of condition keywords used for translation                  */
/*                                                                   */
/*********************************************************************/

KeywordEntry RexxSource::conditionKeywords[] = {       /* condition option table     */
  KeywordEntry(CHAR_ANY,            CONDITION_ANY),
  KeywordEntry(CHAR_ERROR,          CONDITION_ERROR),
  KeywordEntry(CHAR_FAILURE,        CONDITION_FAILURE),
  KeywordEntry(CHAR_HALT,           CONDITION_HALT),
  KeywordEntry(CHAR_LOSTDIGITS,     CONDITION_LOSTDIGITS),
  KeywordEntry(CHAR_NOMETHOD,       CONDITION_NOMETHOD),
  KeywordEntry(CHAR_NOSTRING,       CONDITION_NOSTRING),
  KeywordEntry(CHAR_NOTREADY,       CONDITION_NOTREADY),
  KeywordEntry(CHAR_NOVALUE,        CONDITION_NOVALUE),
  KeywordEntry(CHAR_PROPAGATE,      CONDITION_PROPAGATE),
  KeywordEntry(CHAR_SYNTAX,         CONDITION_SYNTAX),
  KeywordEntry(CHAR_USER,           CONDITION_USER),
};

/*********************************************************************/
/*                                                                   */
/* Table of parse options used for translation                       */
/*                                                                   */
/*********************************************************************/

KeywordEntry RexxSource::parseOptions[] = {            /* parse option table         */
  KeywordEntry(CHAR_ARG,           SUBKEY_ARG),
  KeywordEntry(CHAR_CASELESS,      SUBKEY_CASELESS),
  KeywordEntry(CHAR_LINEIN,        SUBKEY_LINEIN),
  KeywordEntry(CHAR_LOWER,         SUBKEY_LOWER),
  KeywordEntry(CHAR_PULL,          SUBKEY_PULL),
  KeywordEntry(CHAR_SOURCE,        SUBKEY_SOURCE),
  KeywordEntry(CHAR_UPPER,         SUBKEY_UPPER),
  KeywordEntry(CHAR_VALUE,         SUBKEY_VALUE),
  KeywordEntry(CHAR_VAR,           SUBKEY_VAR),
  KeywordEntry(CHAR_VERSION,       SUBKEY_VERSION),
};

/*********************************************************************/
/*                                                                   */
/* Table of directive subkeywords used for translation               */
/*                                                                   */
/*********************************************************************/

KeywordEntry RexxSource::subDirectives[] = {           /* language directive subkeywords    */
    KeywordEntry(CHAR_ABSTRACT,    SUBDIRECTIVE_ABSTRACT),
    KeywordEntry(CHAR_ATTRIBUTE,   SUBDIRECTIVE_ATTRIBUTE),
    KeywordEntry(CHAR_CLASS,       SUBDIRECTIVE_CLASS),
    KeywordEntry(CHAR_DIGITS,      SUBDIRECTIVE_DIGITS),
    KeywordEntry(CHAR_EXTERNAL,    SUBDIRECTIVE_EXTERNAL),
    KeywordEntry(CHAR_FORM,        SUBDIRECTIVE_FORM),
    KeywordEntry(CHAR_FUZZ,        SUBDIRECTIVE_FUZZ),
    KeywordEntry(CHAR_GET,         SUBDIRECTIVE_GET),
    KeywordEntry(CHAR_GUARDED,     SUBDIRECTIVE_GUARDED),
    KeywordEntry(CHAR_INHERIT,     SUBDIRECTIVE_INHERIT),
    KeywordEntry(CHAR_LIBRARY,     SUBDIRECTIVE_LIBRARY),
    KeywordEntry(CHAR_METACLASS,   SUBDIRECTIVE_METACLASS),
    KeywordEntry(CHAR_MIXINCLASS,  SUBDIRECTIVE_MIXINCLASS),
    KeywordEntry(CHAR_PRIVATE,     SUBDIRECTIVE_PRIVATE),
    KeywordEntry(CHAR_PROTECTED,   SUBDIRECTIVE_PROTECTED),
    KeywordEntry(CHAR_PUBLIC,      SUBDIRECTIVE_PUBLIC),
    KeywordEntry(CHAR_SET,         SUBDIRECTIVE_SET),
    KeywordEntry(CHAR_SUBCLASS,    SUBDIRECTIVE_SUBCLASS),
    KeywordEntry(CHAR_TRACE,       SUBDIRECTIVE_TRACE),
    KeywordEntry(CHAR_UNGUARDED,   SUBDIRECTIVE_UNGUARDED),
    KeywordEntry(CHAR_UNPROTECTED, SUBDIRECTIVE_UNPROTECTED),
};

/**
 * Resolve a token to a potential subkeyword.
 *
 * @param token      The token to resolve.
 * @param Table      The table to search.
 * @param Table_Size The size of the table.
 *
 * @return The numeric identifier for the constant.  Returns 0 if not
 *         found in the target table.
 */
int RexxSource::resolveKeyword(RexxString *token, KeywordEntry *Table, int Table_Size)
{
    const char *Name = token->getStringData();
    stringsize_t Length = token->getLength();

    int Lower = 0;                           /* set initial lower bound    */
    int Upper = Table_Size - 1;              /* set the upper bound        */
    char FirstChar = *Name;                   /* get the first character    */

    while (Lower <= Upper)
    {             /* while still a range        */
                  /* set new middle location    */
        int Middle = Lower + ((Upper - Lower) / 2);
        /* if first character matches */
        if (*Table[Middle].name == FirstChar)
        {
            int rc = memcmp(Name, Table[Middle].name, Numerics::minVal(Length, Table[Middle].length));
            if (!rc)
            {                       /* compared equal             */
                                    /* lengths equal?             */
                if (Length == Table[Middle].length)
                {
                    /* return this keyword code   */
                    return Table[Middle].keyword_code;
                }
                /* have to go up?             */
                else if (Length > Table[Middle].length)
                {
                    Lower = Middle + 1;          /* set new lower bound        */
                }
                else                           /* going down                 */
                {
                    Upper = Middle - 1;          /* set new upper bound        */
                }
            }
            else if (rc > 0)                 /* name is larger             */
            {
                Lower = Middle + 1;            /* set new lower bound        */
            }
            else                             /* going down                 */
            {
                Upper = Middle - 1;            /* set new upper bound        */
            }
        }
        /* name is larger             */
        else if (*Table[Middle].name < FirstChar)
        {
            Lower = Middle + 1;              /* set new lower bound        */
        }
        else                               /* going down                 */
        {
            Upper = Middle - 1;              /* set new upper bound        */
        }
    }
    return 0;                            /* return failure flag        */
}

#define tabSize(t) (sizeof(t)/sizeof(t[0]))


int RexxSource::subKeyword(
    RexxToken  *token)                 /* token to check                    */
/******************************************************************************/
/* Function:  Return a numeric subkeyword identifier for a token              */
/******************************************************************************/
{
    // not a symbol?  not a keyword
    if (!token->isSymbol())
    {
        return 0;
    }
    return resolveKeyword(token->value, subKeywords, tabSize(subKeywords));
}

int RexxSource::keyword(
    RexxToken  *token)                 /* token to check                    */
/****************************************************************************/
/* Function:  Return a numeric keyword identifier for a token               */
/****************************************************************************/
{
    // not a symbol?  not a keyword
    if (!token->isSymbol())
    {
        return 0;
    }
    return resolveKeyword(token->value, keywordInstructions, tabSize(keywordInstructions));
}

int RexxSource::builtin(
    RexxToken  *token)                 /* token to check                    */
/****************************************************************************/
/* Function:  Return a numeric builtin function identifier for a token      */
/****************************************************************************/
{
    // This can be a symbol or a literal
    if (!token->isSymbolOrLiteral())
    {
        return 0;
    }
    return resolveKeyword(token->value, builtinFunctions, tabSize(builtinFunctions));
}


size_t RexxSource::resolveBuiltin(
    RexxString *value)                 /* name to check                     */
/******************************************************************************/
/* Function:  Return a numeric keyword identifier for a string                */
/******************************************************************************/
{
    return (size_t)resolveKeyword(value, builtinFunctions, tabSize(builtinFunctions));
}


int RexxSource::condition(
    RexxToken  *token)                 /* token to check                    */
/****************************************************************************/
/* Function:  Return a numeric condition identifier for a token             */
/****************************************************************************/
{
    // not a symbol?  not a keyword
    if (!token->isSymbol())
    {
        return 0;
    }
    return resolveKeyword(token->value, conditionKeywords, tabSize(conditionKeywords));
}

int RexxSource::parseOption(
    RexxToken  *token)                 /* token to check                    */
/****************************************************************************/
/* Function:  Return a numeric condition identifier for a token             */
/****************************************************************************/
{
    // not a symbol?  not a keyword
    if (!token->isSymbol())
    {
        return 0;
    }
    return resolveKeyword(token->value, parseOptions, tabSize(parseOptions));
}

int RexxSource::keyDirective(
    RexxToken  *token)                 /* token to check                    */
/****************************************************************************/
/* Function:  Return a numeric directive identifier for a token             */
/****************************************************************************/
{
    // not a symbol?  not a keyword
    if (!token->isSymbol())
    {
        return 0;
    }
    return resolveKeyword(token->value, directives, tabSize(directives));
}

int RexxSource::subDirective(
    RexxToken  *token)                 /* token to check                    */
/****************************************************************************/
/* Function:  Return a numeric directive option identifier for a token      */
/****************************************************************************/
{
    // not a symbol?  not a keyword
    if (!token->isSymbol())
    {
        return 0;
    }
    return resolveKeyword(token->value, subDirectives, tabSize(subDirectives));
}
