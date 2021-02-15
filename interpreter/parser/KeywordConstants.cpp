/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2020 Rexx Language Association. All rights reserved.    */
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
/* Parser keyword constants                                                   */
/*                                                                            */
/*   NOTE!!! It is critical for all the following tables to be                */
/*           in ASCII alphabetic order.                                       */
/*                                                                            */
/******************************************************************************/

#include "RexxCore.h"
#include "Token.hpp"

// language directive table
KeywordEntry RexxToken::directives[] =
{
    KeywordEntry("ANNOTATE",    DIRECTIVE_ANNOTATE),
    KeywordEntry("ATTRIBUTE",   DIRECTIVE_ATTRIBUTE),
    KeywordEntry("CLASS",       DIRECTIVE_CLASS),
    KeywordEntry("CONSTANT",    DIRECTIVE_CONSTANT),
    KeywordEntry("METHOD",      DIRECTIVE_METHOD),
    KeywordEntry("OPTIONS",     DIRECTIVE_OPTIONS),
    KeywordEntry("REQUIRES",    DIRECTIVE_REQUIRES),
    KeywordEntry("RESOURCE",    DIRECTIVE_RESOURCE),
    KeywordEntry("ROUTINE",     DIRECTIVE_ROUTINE),
};

// The keyword instruction table
KeywordEntry RexxToken::keywordInstructions[] =
{
    KeywordEntry("ADDRESS",    KEYWORD_ADDRESS),
    KeywordEntry("ARG",        KEYWORD_ARG),
    KeywordEntry("CALL",       KEYWORD_CALL),
    KeywordEntry("DO",         KEYWORD_DO),
    KeywordEntry("DROP",       KEYWORD_DROP),
    KeywordEntry("ELSE",       KEYWORD_ELSE),
    KeywordEntry("END",        KEYWORD_END),
    KeywordEntry("EXIT",       KEYWORD_EXIT),
    KeywordEntry("EXPOSE",     KEYWORD_EXPOSE),
    KeywordEntry("FORWARD",    KEYWORD_FORWARD),
    KeywordEntry("GUARD",      KEYWORD_GUARD),
    KeywordEntry("IF",         KEYWORD_IF),
    KeywordEntry("INTERPRET",  KEYWORD_INTERPRET),
    KeywordEntry("ITERATE",    KEYWORD_ITERATE),
    KeywordEntry("LEAVE",      KEYWORD_LEAVE),
    KeywordEntry("LOOP",       KEYWORD_LOOP),
    KeywordEntry("NOP",        KEYWORD_NOP),
    KeywordEntry("NUMERIC",    KEYWORD_NUMERIC),
    KeywordEntry("OPTIONS",    KEYWORD_OPTIONS),
    KeywordEntry("OTHERWISE",  KEYWORD_OTHERWISE),
    KeywordEntry("PARSE",      KEYWORD_PARSE),
    KeywordEntry("PROCEDURE",  KEYWORD_PROCEDURE),
    KeywordEntry("PULL",       KEYWORD_PULL),
    KeywordEntry("PUSH",       KEYWORD_PUSH),
    KeywordEntry("QUEUE",      KEYWORD_QUEUE),
    KeywordEntry("RAISE",      KEYWORD_RAISE),
    KeywordEntry("REPLY",      KEYWORD_REPLY),
    KeywordEntry("RETURN",     KEYWORD_RETURN),
    KeywordEntry("SAY",        KEYWORD_SAY),
    KeywordEntry("SELECT",     KEYWORD_SELECT),
    KeywordEntry("SIGNAL",     KEYWORD_SIGNAL),
    KeywordEntry("THEN",       KEYWORD_THEN),
    KeywordEntry("TRACE",      KEYWORD_TRACE),
    KeywordEntry("USE",        KEYWORD_USE),
    KeywordEntry("WHEN",       KEYWORD_WHEN),
};

// instruction subkeyword table
KeywordEntry RexxToken::subKeywords[] =
{
    KeywordEntry("ADDITIONAL",  SUBKEY_ADDITIONAL),
    KeywordEntry("APPEND",      SUBKEY_APPEND),
    KeywordEntry("ARG",         SUBKEY_ARG),
    KeywordEntry("ARGUMENTS",   SUBKEY_ARGUMENTS),
    KeywordEntry("ARRAY",       SUBKEY_ARRAY),
    KeywordEntry("BY",          SUBKEY_BY),
    KeywordEntry("CASE",        SUBKEY_CASE),
    KeywordEntry("CLASS",       SUBKEY_CLASS),
    KeywordEntry("CONTINUE",    SUBKEY_CONTINUE),
    KeywordEntry("COUNTER",     SUBKEY_COUNTER),
    KeywordEntry("DESCRIPTION", SUBKEY_DESCRIPTION),
    KeywordEntry("DIGITS",      SUBKEY_DIGITS),
    KeywordEntry("ENGINEERING", SUBKEY_ENGINEERING),
    KeywordEntry("ERROR",       SUBKEY_ERROR),
    KeywordEntry("EXIT",        SUBKEY_EXIT),
    KeywordEntry("EXPOSE",      SUBKEY_EXPOSE),
    KeywordEntry("FALSE",       SUBKEY_FALSE),
    KeywordEntry("FOR",         SUBKEY_FOR),
    KeywordEntry("FOREVER",     SUBKEY_FOREVER),
    KeywordEntry("FORM",        SUBKEY_FORM),
    KeywordEntry("FUZZ",        SUBKEY_FUZZ),
    KeywordEntry("INDEX",       SUBKEY_INDEX),
    KeywordEntry("INPUT",       SUBKEY_INPUT),
    KeywordEntry("ITEM",        SUBKEY_ITEM),
    KeywordEntry("LABEL",       SUBKEY_LABEL),
    KeywordEntry("LOCAL",       SUBKEY_LOCAL),
    KeywordEntry("MESSAGE",     SUBKEY_MESSAGE),
    KeywordEntry("NAME",        SUBKEY_NAME),
    KeywordEntry("NORMAL",      SUBKEY_NORMAL),
    KeywordEntry("OFF",         SUBKEY_OFF),
    KeywordEntry("ON",          SUBKEY_ON),
    KeywordEntry("OUTPUT",      SUBKEY_OUTPUT),
    KeywordEntry("OVER",        SUBKEY_OVER),
    KeywordEntry("REPLACE",     SUBKEY_REPLACE),
    KeywordEntry("RETURN",      SUBKEY_RETURN),
    KeywordEntry("SCIENTIFIC",  SUBKEY_SCIENTIFIC),
    KeywordEntry("STEM",        SUBKEY_STEM),
    KeywordEntry("STREAM",      SUBKEY_STREAM),
    KeywordEntry("STRICT",      SUBKEY_STRICT),
    KeywordEntry("THEN",        SUBKEY_THEN),
    KeywordEntry("TO",          SUBKEY_TO),
    KeywordEntry("TRUE",        SUBKEY_TRUE),
    KeywordEntry("UNTIL",       SUBKEY_UNTIL),
    KeywordEntry("USING",       SUBKEY_USING),
    KeywordEntry("VALUE",       SUBKEY_VALUE),
    KeywordEntry("WHEN",        SUBKEY_WHEN),
    KeywordEntry("WHILE",       SUBKEY_WHILE),
    KeywordEntry("WITH",        SUBKEY_WITH),
};

// table of builtin functions
KeywordEntry RexxToken::builtinFunctions[] =
{
#ifdef EBCDIC
    KeywordEntry("ABBREV",         BUILTIN_ABBREV),
    KeywordEntry("ABS",            BUILTIN_ABS),
    KeywordEntry("ADDRESS",        BUILTIN_ADDRESS),
    KeywordEntry("ARG",            BUILTIN_ARG),
    KeywordEntry("BITAND",         BUILTIN_BITAND),
    KeywordEntry("BITOR",          BUILTIN_BITOR),
    KeywordEntry("BITXOR",         BUILTIN_BITXOR),
    KeywordEntry("B2X",            BUILTIN_B2X),
    KeywordEntry("CENTER",         BUILTIN_CENTER),
    KeywordEntry("CENTRE",         BUILTIN_CENTRE),
    KeywordEntry("CHANGESTR",      BUILTIN_CHANGESTR),
    KeywordEntry("CHARIN",         BUILTIN_CHARIN),
    KeywordEntry("CHAROUT",        BUILTIN_CHAROUT),
    KeywordEntry("CHARS",          BUILTIN_CHARS),
    KeywordEntry("COMPARE",        BUILTIN_COMPARE),
    KeywordEntry("CONDITION",      BUILTIN_CONDITION),
    KeywordEntry("COPIES",         BUILTIN_COPIES),
    KeywordEntry("COUNTSTR",       BUILTIN_COUNTSTR),
    KeywordEntry("C2D",            BUILTIN_C2D),
    KeywordEntry("C2X",            BUILTIN_C2X),
    KeywordEntry("DATATYPE",       BUILTIN_DATATYPE),
    KeywordEntry("DATE",           BUILTIN_DATE),
    KeywordEntry("DELSTR",         BUILTIN_DELSTR),
    KeywordEntry("DELWORD",        BUILTIN_DELWORD),
    KeywordEntry("DIGITS",         BUILTIN_DIGITS),
    KeywordEntry("D2C",            BUILTIN_D2C),
    KeywordEntry("D2X",            BUILTIN_D2X),
    KeywordEntry("ENDLOCAL",       BUILTIN_ENDLOCAL),
    KeywordEntry("ERRORTEXT",      BUILTIN_ERRORTEXT),
    KeywordEntry("FORM",           BUILTIN_FORM),
    KeywordEntry("FORMAT",         BUILTIN_FORMAT),
    KeywordEntry("FUZZ",           BUILTIN_FUZZ),
    KeywordEntry("INSERT",         BUILTIN_INSERT),
    KeywordEntry("LASTPOS",        BUILTIN_LASTPOS),
    KeywordEntry("LEFT",           BUILTIN_LEFT),
    KeywordEntry("LENGTH",         BUILTIN_LENGTH),
    KeywordEntry("LINEIN",         BUILTIN_LINEIN),
    KeywordEntry("LINEOUT",        BUILTIN_LINEOUT),
    KeywordEntry("LINES",          BUILTIN_LINES),
    KeywordEntry("LOWER",          BUILTIN_LOWER),
    KeywordEntry("MAX",            BUILTIN_MAX),
    KeywordEntry("MIN",            BUILTIN_MIN),
    KeywordEntry("OVERLAY",        BUILTIN_OVERLAY),
    KeywordEntry("POS",            BUILTIN_POS),
    KeywordEntry("QUALIFY",        BUILTIN_QUALIFY),
    KeywordEntry("QUEUED",         BUILTIN_QUEUED),
    KeywordEntry("RANDOM",         BUILTIN_RANDOM),
    KeywordEntry("REVERSE",        BUILTIN_REVERSE),
    KeywordEntry("RIGHT",          BUILTIN_RIGHT),
    KeywordEntry("RXFUNCADD",      BUILTIN_RXFUNCADD),
    KeywordEntry("RXFUNCDROP",     BUILTIN_RXFUNCDROP),
    KeywordEntry("RXFUNCQUERY",    BUILTIN_RXFUNCQUERY),
    KeywordEntry("RXQUEUE",        BUILTIN_RXQUEUE),
    KeywordEntry("SETLOCAL",       BUILTIN_SETLOCAL),
    KeywordEntry("SIGN",           BUILTIN_SIGN),
    KeywordEntry("SOURCELINE",     BUILTIN_SOURCELINE),
    KeywordEntry("SPACE",          BUILTIN_SPACE),
    KeywordEntry("STREAM",         BUILTIN_STREAM),
    KeywordEntry("STRIP",          BUILTIN_STRIP),
    KeywordEntry("SUBSTR",         BUILTIN_SUBSTR),
    KeywordEntry("SUBWORD",        BUILTIN_SUBWORD),
    KeywordEntry("SYMBOL",         BUILTIN_SYMBOL),
    KeywordEntry("TIME",           BUILTIN_TIME),
    KeywordEntry("TRACE",          BUILTIN_TRACE),
    KeywordEntry("TRANSLATE",      BUILTIN_TRANSLATE),
    KeywordEntry("TRUNC",          BUILTIN_TRUNC),
    KeywordEntry("UPPER",          BUILTIN_UPPER),
    KeywordEntry("USERID",         BUILTIN_USERID),
    KeywordEntry("VALUE",          BUILTIN_VALUE),
    KeywordEntry("VAR",            BUILTIN_VAR),
    KeywordEntry("VERIFY",         BUILTIN_VERIFY),
    KeywordEntry("WORD",           BUILTIN_WORD),
    KeywordEntry("WORDINDEX",      BUILTIN_WORDINDEX),
    KeywordEntry("WORDLENGTH",     BUILTIN_WORDLENGTH),
    KeywordEntry("WORDPOS",        BUILTIN_WORDPOS),
    KeywordEntry("WORDS",          BUILTIN_WORDS),
    KeywordEntry("XRANGE",         BUILTIN_XRANGE),
    KeywordEntry("X2B",            BUILTIN_X2B),
    KeywordEntry("X2C",            BUILTIN_X2C),
    KeywordEntry("X2D",            BUILTIN_X2D),
#else
    KeywordEntry("ABBREV",         BUILTIN_ABBREV),
    KeywordEntry("ABS",            BUILTIN_ABS),
    KeywordEntry("ADDRESS",        BUILTIN_ADDRESS),
    KeywordEntry("ARG",            BUILTIN_ARG),
    KeywordEntry("B2X",            BUILTIN_B2X),
    KeywordEntry("BITAND",         BUILTIN_BITAND),
    KeywordEntry("BITOR",          BUILTIN_BITOR),
    KeywordEntry("BITXOR",         BUILTIN_BITXOR),
    KeywordEntry("C2D",            BUILTIN_C2D),
    KeywordEntry("C2X",            BUILTIN_C2X),
    KeywordEntry("CENTER",         BUILTIN_CENTER),
    KeywordEntry("CENTRE",         BUILTIN_CENTRE),
    KeywordEntry("CHANGESTR",      BUILTIN_CHANGESTR),
    KeywordEntry("CHARIN",         BUILTIN_CHARIN),
    KeywordEntry("CHAROUT",        BUILTIN_CHAROUT),
    KeywordEntry("CHARS",          BUILTIN_CHARS),
    KeywordEntry("COMPARE",        BUILTIN_COMPARE),
    KeywordEntry("CONDITION",      BUILTIN_CONDITION),
    KeywordEntry("COPIES",         BUILTIN_COPIES),
    KeywordEntry("COUNTSTR",       BUILTIN_COUNTSTR),
    KeywordEntry("D2C",            BUILTIN_D2C),
    KeywordEntry("D2X",            BUILTIN_D2X),
    KeywordEntry("DATATYPE",       BUILTIN_DATATYPE),
    KeywordEntry("DATE",           BUILTIN_DATE),
    KeywordEntry("DELSTR",         BUILTIN_DELSTR),
    KeywordEntry("DELWORD",        BUILTIN_DELWORD),
    KeywordEntry("DIGITS",         BUILTIN_DIGITS),
    KeywordEntry("ENDLOCAL",       BUILTIN_ENDLOCAL),
    KeywordEntry("ERRORTEXT",      BUILTIN_ERRORTEXT),
    KeywordEntry("FORM",           BUILTIN_FORM),
    KeywordEntry("FORMAT",         BUILTIN_FORMAT),
    KeywordEntry("FUZZ",           BUILTIN_FUZZ),
    KeywordEntry("INSERT",         BUILTIN_INSERT),
    KeywordEntry("LASTPOS",        BUILTIN_LASTPOS),
    KeywordEntry("LEFT",           BUILTIN_LEFT),
    KeywordEntry("LENGTH",         BUILTIN_LENGTH),
    KeywordEntry("LINEIN",         BUILTIN_LINEIN),
    KeywordEntry("LINEOUT",        BUILTIN_LINEOUT),
    KeywordEntry("LINES",          BUILTIN_LINES),
    KeywordEntry("LOWER",          BUILTIN_LOWER),
    KeywordEntry("MAX",            BUILTIN_MAX),
    KeywordEntry("MIN",            BUILTIN_MIN),
    KeywordEntry("OVERLAY",        BUILTIN_OVERLAY),
    KeywordEntry("POS",            BUILTIN_POS),
    KeywordEntry("QUALIFY",        BUILTIN_QUALIFY),
    KeywordEntry("QUEUED",         BUILTIN_QUEUED),
    KeywordEntry("RANDOM",         BUILTIN_RANDOM),
    KeywordEntry("REVERSE",        BUILTIN_REVERSE),
    KeywordEntry("RIGHT",          BUILTIN_RIGHT),
    KeywordEntry("RXFUNCADD",      BUILTIN_RXFUNCADD),
    KeywordEntry("RXFUNCDROP",     BUILTIN_RXFUNCDROP),
    KeywordEntry("RXFUNCQUERY",    BUILTIN_RXFUNCQUERY),
    KeywordEntry("RXQUEUE",        BUILTIN_RXQUEUE),
    KeywordEntry("SETLOCAL",       BUILTIN_SETLOCAL),
    KeywordEntry("SIGN",           BUILTIN_SIGN),
    KeywordEntry("SOURCELINE",     BUILTIN_SOURCELINE),
    KeywordEntry("SPACE",          BUILTIN_SPACE),
    KeywordEntry("STREAM",         BUILTIN_STREAM),
    KeywordEntry("STRIP",          BUILTIN_STRIP),
    KeywordEntry("SUBSTR",         BUILTIN_SUBSTR),
    KeywordEntry("SUBWORD",        BUILTIN_SUBWORD),
    KeywordEntry("SYMBOL",         BUILTIN_SYMBOL),
    KeywordEntry("TIME",           BUILTIN_TIME),
    KeywordEntry("TRACE",          BUILTIN_TRACE),
    KeywordEntry("TRANSLATE",      BUILTIN_TRANSLATE),
    KeywordEntry("TRUNC",          BUILTIN_TRUNC),
    KeywordEntry("UPPER",          BUILTIN_UPPER),
    KeywordEntry("USERID",         BUILTIN_USERID),
    KeywordEntry("VALUE",          BUILTIN_VALUE),
    KeywordEntry("VAR",            BUILTIN_VAR),
    KeywordEntry("VERIFY",         BUILTIN_VERIFY),
    KeywordEntry("WORD",           BUILTIN_WORD),
    KeywordEntry("WORDINDEX",      BUILTIN_WORDINDEX),
    KeywordEntry("WORDLENGTH",     BUILTIN_WORDLENGTH),
    KeywordEntry("WORDPOS",        BUILTIN_WORDPOS),
    KeywordEntry("WORDS",          BUILTIN_WORDS),
    KeywordEntry("X2B",            BUILTIN_X2B),
    KeywordEntry("X2C",            BUILTIN_X2C),
    KeywordEntry("X2D",            BUILTIN_X2D),
    KeywordEntry("XRANGE",         BUILTIN_XRANGE),
#endif
};

// Condition name table
KeywordEntry RexxToken::conditionKeywords[] =
{
    KeywordEntry("ANY",            CONDITION_ANY),
    KeywordEntry("ERROR",          CONDITION_ERROR),
    KeywordEntry("FAILURE",        CONDITION_FAILURE),
    KeywordEntry("HALT",           CONDITION_HALT),
    KeywordEntry("LOSTDIGITS",     CONDITION_LOSTDIGITS),
    KeywordEntry("NOMETHOD",       CONDITION_NOMETHOD),
    KeywordEntry("NOSTRING",       CONDITION_NOSTRING),
    KeywordEntry("NOTREADY",       CONDITION_NOTREADY),
    KeywordEntry("NOVALUE",        CONDITION_NOVALUE),
    KeywordEntry("PROPAGATE",      CONDITION_PROPAGATE),
    KeywordEntry("SYNTAX",         CONDITION_SYNTAX),
    KeywordEntry("USER",           CONDITION_USER),
};

// parse option subkeywords
KeywordEntry RexxToken::parseOptions[] =
{
    KeywordEntry("ARG",           SUBKEY_ARG),
    KeywordEntry("CASELESS",      SUBKEY_CASELESS),
    KeywordEntry("LINEIN",        SUBKEY_LINEIN),
    KeywordEntry("LOWER",         SUBKEY_LOWER),
    KeywordEntry("PULL",          SUBKEY_PULL),
    KeywordEntry("SOURCE",        SUBKEY_SOURCE),
    KeywordEntry("UPPER",         SUBKEY_UPPER),
    KeywordEntry("VALUE",         SUBKEY_VALUE),
    KeywordEntry("VAR",           SUBKEY_VAR),
    KeywordEntry("VERSION",       SUBKEY_VERSION),
};

// subkeywords on directive instructions
KeywordEntry RexxToken::subDirectives[] =
{
    KeywordEntry("ABSTRACT",    SUBDIRECTIVE_ABSTRACT),
    KeywordEntry("ALL",         SUBDIRECTIVE_ALL),
    KeywordEntry("ATTRIBUTE",   SUBDIRECTIVE_ATTRIBUTE),
    KeywordEntry("CLASS",       SUBDIRECTIVE_CLASS),
    KeywordEntry("CONDITION",   SUBDIRECTIVE_CONDITION),
    KeywordEntry("CONSTANT",    SUBDIRECTIVE_CONSTANT),
    KeywordEntry("DELEGATE",    SUBDIRECTIVE_DELEGATE),
    KeywordEntry("DIGITS",      SUBDIRECTIVE_DIGITS),
    KeywordEntry("END",         SUBDIRECTIVE_END),
    KeywordEntry("ERROR",       SUBDIRECTIVE_ERROR),
    KeywordEntry("EXTERNAL",    SUBDIRECTIVE_EXTERNAL),
    KeywordEntry("FAILURE",     SUBDIRECTIVE_FAILURE),
    KeywordEntry("FORM",        SUBDIRECTIVE_FORM),
    KeywordEntry("FUZZ",        SUBDIRECTIVE_FUZZ),
    KeywordEntry("GET",         SUBDIRECTIVE_GET),
    KeywordEntry("GUARDED",     SUBDIRECTIVE_GUARDED),
    KeywordEntry("INHERIT",     SUBDIRECTIVE_INHERIT),
    KeywordEntry("LIBRARY",     SUBDIRECTIVE_LIBRARY),
    KeywordEntry("LOSTDIGITS",  SUBDIRECTIVE_LOSTDIGITS),
    KeywordEntry("METACLASS",   SUBDIRECTIVE_METACLASS),
    KeywordEntry("METHOD",      SUBDIRECTIVE_METHOD),
    KeywordEntry("MIXINCLASS",  SUBDIRECTIVE_MIXINCLASS),
    KeywordEntry("NAMESPACE",   SUBDIRECTIVE_NAMESPACE),
    KeywordEntry("NOPROLOG",    SUBDIRECTIVE_NOPROLOG),
    KeywordEntry("NOSTRING",    SUBDIRECTIVE_NOSTRING),
    KeywordEntry("NOTREADY",    SUBDIRECTIVE_NOTREADY),
    KeywordEntry("NOVALUE",     SUBDIRECTIVE_NOVALUE),
    KeywordEntry("PACKAGE",     SUBDIRECTIVE_PACKAGE),
    KeywordEntry("PRIVATE",     SUBDIRECTIVE_PRIVATE),
    KeywordEntry("PROLOG",      SUBDIRECTIVE_PROLOG),
    KeywordEntry("PROTECTED",   SUBDIRECTIVE_PROTECTED),
    KeywordEntry("PUBLIC",      SUBDIRECTIVE_PUBLIC),
    KeywordEntry("ROUTINE",     SUBDIRECTIVE_ROUTINE),
    KeywordEntry("SET",         SUBDIRECTIVE_SET),
    KeywordEntry("SUBCLASS",    SUBDIRECTIVE_SUBCLASS),
    KeywordEntry("SYNTAX",      SUBDIRECTIVE_SYNTAX),
    KeywordEntry("TRACE",       SUBDIRECTIVE_TRACE),
    KeywordEntry("UNGUARDED",   SUBDIRECTIVE_UNGUARDED),
    KeywordEntry("UNPROTECTED", SUBDIRECTIVE_UNPROTECTED),
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
int RexxToken::resolveKeyword(RexxString *token, KeywordEntry *table, size_t tableSize)
{
    const char *name = token->getStringData();
    size_t length = token->getLength();

    // search this table using a binary search

    // NOTE:  We need to use signed numbers for this, otherwise
    // the numbers can wrap
    int lower = 0;                                // set initial lower bound
    int upper = (int)tableSize - 1;               // set the upper bound
    char firstChar = *name;                       // get the first character for fast compares

    // loop until the range converges
    while (lower <= upper)
    {
        // find a new middle location
        int middle = lower + ((upper - lower) / 2);

        // only compare on the name if the first character matches
        if (*table[middle].name == firstChar)
        {
            int rc = memcmp(name, table[middle].name, std::min(length, table[middle].length));
            // if this compared equal, then compare the lengths...if not equal, the longer is
            // the greater of the two.
            if (rc == 0)
            {
                // same length, they are equal
                if (length == table[middle].length)
                {
                    // return the keyword code.
                    return table[middle].keywordCode;
                }
                // not equal, figure out how to divide the range
                else if (length > table[middle].length)
                {
                    // new lower bound
                    lower = middle + 1;
                }
                else
                {
                    // going the other direction
                    upper = middle - 1;
                }
            }
            // did not compare equal, so figure out which way to move
            else if (rc > 0)
            {
                lower = middle + 1;
            }
            else
            {
                upper = middle - 1;
            }
        }
        // still on the first char compares
        else if (*table[middle].name < firstChar)
        {
            lower = middle + 1;
        }
        else
        {
            upper = middle - 1;
        }
    }
    // zero is not found
    return 0;
}


#define tabSize(t) (sizeof(t)/sizeof(t[0]))


/**
 * Resolve a token value to a subkeyword identifier.
 *
 * @return The subkeyword identifier.
 */
InstructionSubKeyword RexxToken::subKeyword()
{
    // not a symbol?  not a keyword
    if (!isSymbol())
    {
        return SUBKEY_NONE;
    }
    return static_cast<InstructionSubKeyword>(resolveKeyword(stringValue, subKeywords, tabSize(subKeywords)));
}

/**
 * Resolve a token to a keyword value.
 *
 * @return The keyword identifier.
 */
InstructionKeyword RexxToken::keyword()
{
    // not a symbol?  not a keyword
    if (!isSymbol())
    {
        return KEYWORD_NONE;
    }
    return static_cast<InstructionKeyword>(resolveKeyword(stringValue, keywordInstructions, tabSize(keywordInstructions)));
}

/**
 * Resolve a token to a builtin identifier.
 *
 * @return The builtin code.
 */
BuiltinCode RexxToken::builtin()
{
    // This can be a symbol or a literal
    if (!isSymbolOrLiteral())
    {
        return NO_BUILTIN;
    }
    return static_cast<BuiltinCode>(resolveKeyword(stringValue, builtinFunctions, tabSize(builtinFunctions)));
}


/**
 * Resolve a condition name to a keyword code.
 *
 * @return The keyword code corresponding to the condition name.
 */
ConditionKeyword RexxToken::condition()
{
    // not a symbol?  not a keyword
    if (!isSymbol())
    {
        return CONDITION_NONE;
    }
    return static_cast<ConditionKeyword>(resolveKeyword(stringValue, conditionKeywords, tabSize(conditionKeywords)));
}

/**
 * Resolve a token to a parse option keyword.
 *
 * @return The corresponding keyword code.
 */
InstructionSubKeyword RexxToken::parseOption()
{
    // not a symbol?  not a keyword
    if (!isSymbol())
    {
        return SUBKEY_NONE;
    }
    return static_cast<InstructionSubKeyword>(resolveKeyword(stringValue, parseOptions, tabSize(parseOptions)));
}

/**
 * Resolve a token to a directve name.
 *
 * @return The code for the directive.
 */
DirectiveKeyword RexxToken::keyDirective()
{
    // not a symbol?  not a keyword
    if (!isSymbol())
    {
        return DIRECTIVE_NONE;
    }
    return static_cast<DirectiveKeyword>(resolveKeyword(stringValue, directives, tabSize(directives)));
}

/**
 * Resolve a token to a subkeyword on a directive instruction.
 *
 * @return The code for the sub keyword.
 */
DirectiveSubKeyword RexxToken::subDirective()
{
    // not a symbol?  not a keyword
    if (!isSymbol())
    {
        return SUBDIRECTIVE_NONE;
    }
    return static_cast<DirectiveSubKeyword>(resolveKeyword(stringValue, subDirectives, tabSize(subDirectives)));
}


/**
 * Resolve a builtin name to a function code.
 *
 * @param value  The value to check.
 *
 * @return The corresponding builtin code.
 */
BuiltinCode RexxToken::resolveBuiltin(RexxString *value)
{
    return static_cast<BuiltinCode>(resolveKeyword(value, builtinFunctions, tabSize(builtinFunctions)));
}
