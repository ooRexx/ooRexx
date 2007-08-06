/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                                  RexxConstants.c     */
/*                                                                            */
/* Global Object REXX constants                                               */
/*                                                                            */
/*   NOTE!!! It is critical for all the following tables to be                */
/*           in ASCII alphabetic order.                                       */
/*                                                                            */
/******************************************************************************/

#define DEFINING
#include "RexxCore.h"
#include "Token.hpp"

/*********************************************************************/
/*                                                                   */
/* Table of directive instructions used for translation              */
/*                                                                   */
/*********************************************************************/

KWDTABLE Directives[] = {              /* language directive table          */
   KWD(CHAR_ATTRIBUTE,   DIRECTIVE_ATTRIBUTE)
   KWD(CHAR_CLASS,       DIRECTIVE_CLASS)
   KWD(CHAR_METHOD,      DIRECTIVE_METHOD)
   KWD(CHAR_REQUIRES,    DIRECTIVE_REQUIRES)
   KWD(CHAR_ROUTINE,     DIRECTIVE_ROUTINE)
};

INT Directivescount =            /* size of directive table     */
    sizeof(Directives)/sizeof(KWDTABLE);


/*********************************************************************/
/*                                                                   */
/* Table of keyword instructions used for translation                */
/*                                                                   */
/*********************************************************************/

KWDTABLE KeywordInstructions[] = {     /* language keyword table     */
   KWD(CHAR_ADDRESS,    KEYWORD_ADDRESS)
   KWD(CHAR_ARG,        KEYWORD_ARG)
   KWD(CHAR_CALL,       KEYWORD_CALL)
   KWD(CHAR_DO,         KEYWORD_DO)
   KWD(CHAR_DROP,       KEYWORD_DROP)
   KWD(CHAR_ELSE,       KEYWORD_ELSE)
   KWD(CHAR_END,        KEYWORD_END)
   KWD(CHAR_EXIT,       KEYWORD_EXIT)
   KWD(CHAR_EXPOSE,     KEYWORD_EXPOSE)
   KWD(CHAR_FORWARD,    KEYWORD_FORWARD)
   KWD(CHAR_GUARD,      KEYWORD_GUARD)
   KWD(CHAR_IF,         KEYWORD_IF)
   KWD(CHAR_INTERPRET,  KEYWORD_INTERPRET)
   KWD(CHAR_ITERATE,    KEYWORD_ITERATE)
   KWD(CHAR_LEAVE,      KEYWORD_LEAVE)
   KWD(CHAR_LOOP,       KEYWORD_LOOP)
   KWD(CHAR_NOP,        KEYWORD_NOP)
   KWD(CHAR_NUMERIC,    KEYWORD_NUMERIC)
   KWD(CHAR_OPTIONS,    KEYWORD_OPTIONS)
   KWD(CHAR_OTHERWISE,  KEYWORD_OTHERWISE)
   KWD(CHAR_PARSE,      KEYWORD_PARSE)
   KWD(CHAR_PROCEDURE,  KEYWORD_PROCEDURE)
   KWD(CHAR_PULL,       KEYWORD_PULL)
   KWD(CHAR_PUSH,       KEYWORD_PUSH)
   KWD(CHAR_QUEUE,      KEYWORD_QUEUE)
   KWD(CHAR_RAISE,      KEYWORD_RAISE)
   KWD(CHAR_REPLY,      KEYWORD_REPLY)
   KWD(CHAR_RETURN,     KEYWORD_RETURN)
   KWD(CHAR_SAY,        KEYWORD_SAY)
   KWD(CHAR_SELECT,     KEYWORD_SELECT)
   KWD(CHAR_SIGNAL,     KEYWORD_SIGNAL)
   KWD(CHAR_THEN,       KEYWORD_THEN)
   KWD(CHAR_TRACE,      KEYWORD_TRACE)
   KWD(CHAR_USE,        KEYWORD_USE)
   KWD(CHAR_WHEN,       KEYWORD_WHEN)
};

INT KeywordInstructionscount =            /* size of instruction table     */
    sizeof(KeywordInstructions)/sizeof(KWDTABLE);

/*********************************************************************/
/*                                                                   */
/* Table of instruction subkeywords used for translation             */
/*                                                                   */
/*********************************************************************/

KWDTABLE SubKeywords[] = {             /* language keyword table     */
   KWD(CHAR_ADDITIONAL,  SUBKEY_ADDITIONAL)
   KWD(CHAR_ARG,         SUBKEY_ARG)
   KWD(CHAR_ARGUMENTS,   SUBKEY_ARGUMENTS)
   KWD(CHAR_ARRAY,       SUBKEY_ARRAY)
   KWD(CHAR_BY,          SUBKEY_BY)
   KWD(CHAR_CLASS,       SUBKEY_CLASS)
   KWD(CHAR_CONTINUE,    SUBKEY_CONTINUE)
   KWD(CHAR_DESCRIPTION, SUBKEY_DESCRIPTION)
   KWD(CHAR_DIGITS,      SUBKEY_DIGITS)
   KWD(CHAR_ENGINEERING, SUBKEY_ENGINEERING)
   KWD(CHAR_EXIT,        SUBKEY_EXIT)
   KWD(CHAR_EXPOSE,      SUBKEY_EXPOSE)
   KWD(CHAR_FALSE,       SUBKEY_FALSE)
   KWD(CHAR_FOR,         SUBKEY_FOR)
   KWD(CHAR_FOREVER,     SUBKEY_FOREVER)
   KWD(CHAR_FORM,        SUBKEY_FORM)
   KWD(CHAR_FUZZ,        SUBKEY_FUZZ)
   KWD(CHAR_LABEL,       SUBKEY_LABEL)
   KWD(CHAR_MESSAGE,     SUBKEY_MESSAGE)
   KWD(CHAR_NAME,        SUBKEY_NAME)
   KWD(CHAR_OFF,         SUBKEY_OFF)
   KWD(CHAR_ON,          SUBKEY_ON)
   KWD(CHAR_OVER,        SUBKEY_OVER)
   KWD(CHAR_RETURN,      SUBKEY_RETURN)
   KWD(CHAR_SCIENTIFIC,  SUBKEY_SCIENTIFIC)
   KWD(CHAR_STRICT,      SUBKEY_STRICT)
   KWD(CHAR_THEN,        SUBKEY_THEN)
   KWD(CHAR_TO,          SUBKEY_TO)
   KWD(CHAR_TRUE,        SUBKEY_TRUE)
   KWD(CHAR_UNTIL,       SUBKEY_UNTIL)
   KWD(CHAR_VALUE,       SUBKEY_VALUE)
   KWD(CHAR_WHEN,        SUBKEY_WHEN)
   KWD(CHAR_WHILE,       SUBKEY_WHILE)
   KWD(CHAR_WITH,        SUBKEY_WITH)
};

INT SubKeywordscount =            /* size of subkeyword table     */
    sizeof(SubKeywords)/sizeof(KWDTABLE);

/*************************************************************************/
/*                                                                   */
/* Table of built-in functions used for translation                  */
/*                                                                   */
/*********************************************************************/

KWDTABLE BuiltinFunctions[] = {        /* built-in function table    */
    KWD(CHAR_ABBREV,      BUILTIN_ABBREV)
    KWD(CHAR_ABS,         BUILTIN_ABS)
    KWD(CHAR_ADDRESS,     BUILTIN_ADDRESS)
    KWD(CHAR_ARG,         BUILTIN_ARG)
    KWD(CHAR_B2X,         BUILTIN_B2X)
    KWD(CHAR_BITAND,      BUILTIN_BITAND)
    KWD(CHAR_BITOR,       BUILTIN_BITOR)
    KWD(CHAR_BITXOR,      BUILTIN_BITXOR)
    KWD(CHAR_C2D,         BUILTIN_C2D)
    KWD(CHAR_C2X,         BUILTIN_C2X)
    KWD(CHAR_CENTER,      BUILTIN_CENTER)
    KWD(CHAR_CENTRE,      BUILTIN_CENTRE)
    KWD(CHAR_CHANGESTR,   BUILTIN_CHANGESTR)
    KWD(CHAR_CHARIN,      BUILTIN_CHARIN)
    KWD(CHAR_CHAROUT,     BUILTIN_CHAROUT)
    KWD(CHAR_CHARS,       BUILTIN_CHARS)
    KWD(CHAR_COMPARE,     BUILTIN_COMPARE)
    KWD(CHAR_CONDITION,   BUILTIN_CONDITION)
    KWD(CHAR_COPIES,      BUILTIN_COPIES)
    KWD(CHAR_COUNTSTR,    BUILTIN_COUNTSTR)
    KWD(CHAR_D2C,         BUILTIN_D2C)
    KWD(CHAR_D2X,         BUILTIN_D2X)
    KWD(CHAR_DATATYPE,    BUILTIN_DATATYPE)
    KWD(CHAR_DATE,        BUILTIN_DATE)
    KWD(CHAR_DBADJUST,    BUILTIN_DBADJUST)
    KWD(CHAR_DBBRACKET,   BUILTIN_DBBRACKET)
    KWD(CHAR_DBCENTER,    BUILTIN_DBCENTER)
    KWD(CHAR_DBLEFT,      BUILTIN_DBLEFT)
    KWD(CHAR_DBRIGHT,     BUILTIN_DBRIGHT)
    KWD(CHAR_DBRLEFT,     BUILTIN_DBRLEFT)
    KWD(CHAR_DBRRIGHT,    BUILTIN_DBRRIGHT)
    KWD(CHAR_DBTODBCS,    BUILTIN_DBTODBCS)
    KWD(CHAR_DBTOSBCS,    BUILTIN_DBTOSBCS)
    KWD(CHAR_DBUNBRACKET, BUILTIN_DBUNBRACKET)
    KWD(CHAR_DBVALIDATE,  BUILTIN_DBVALIDATE)
    KWD(CHAR_DBWIDTH,     BUILTIN_DBWIDTH)
    KWD(CHAR_DELSTR,      BUILTIN_DELSTR)
    KWD(CHAR_DELWORD,     BUILTIN_DELWORD)
    KWD(CHAR_DIGITS,      BUILTIN_DIGITS)
    KWD(CHAR_ERRORTEXT,   BUILTIN_ERRORTEXT)
    KWD(CHAR_FORM,        BUILTIN_FORM)
    KWD(CHAR_FORMAT,      BUILTIN_FORMAT)
    KWD(CHAR_FUZZ,        BUILTIN_FUZZ)
    KWD(CHAR_INSERT,      BUILTIN_INSERT)
    KWD(CHAR_LASTPOS,     BUILTIN_LASTPOS)
    KWD(CHAR_LEFT,        BUILTIN_LEFT)
    KWD(CHAR_LENGTH,      BUILTIN_LENGTH)
    KWD(CHAR_LINEIN,      BUILTIN_LINEIN)
    KWD(CHAR_LINEOUT,     BUILTIN_LINEOUT)
    KWD(CHAR_LINES,       BUILTIN_LINES)
    KWD(CHAR_LOWER,       BUILTIN_LOWER)
    KWD(CHAR_MAX,         BUILTIN_MAX)
    KWD(CHAR_MIN,         BUILTIN_MIN)
    KWD(CHAR_OVERLAY,     BUILTIN_OVERLAY)
    KWD(CHAR_POS,         BUILTIN_POS)
    KWD(CHAR_QUEUED,      BUILTIN_QUEUED)
    KWD(CHAR_RANDOM,      BUILTIN_RANDOM)
    KWD(CHAR_REVERSE,     BUILTIN_REVERSE)
    KWD(CHAR_RIGHT,       BUILTIN_RIGHT)
    KWD(CHAR_SIGN,        BUILTIN_SIGN)
    KWD(CHAR_SOURCELINE,  BUILTIN_SOURCELINE)
    KWD(CHAR_SPACE,       BUILTIN_SPACE)
    KWD(CHAR_STREAM,      BUILTIN_STREAM)
    KWD(CHAR_STRIP,       BUILTIN_STRIP)
    KWD(CHAR_SUBSTR,      BUILTIN_SUBSTR)
    KWD(CHAR_SUBWORD,     BUILTIN_SUBWORD)
    KWD(CHAR_SYMBOL,      BUILTIN_SYMBOL)
    KWD(CHAR_TIME,        BUILTIN_TIME)
    KWD(CHAR_TRACE,       BUILTIN_TRACE)
    KWD(CHAR_TRANSLATE,   BUILTIN_TRANSLATE)
    KWD(CHAR_TRUNC,       BUILTIN_TRUNC)
    KWD(CHAR_UPPER,       BUILTIN_UPPER)
    KWD(CHAR_USERID,      BUILTIN_USERID)
    KWD(CHAR_VALUE,       BUILTIN_VALUE)
    KWD(CHAR_VAR,         BUILTIN_VAR)
    KWD(CHAR_VERIFY,      BUILTIN_VERIFY)
    KWD(CHAR_WORD,        BUILTIN_WORD)
    KWD(CHAR_WORDINDEX,   BUILTIN_WORDINDEX)
    KWD(CHAR_WORDLENGTH,  BUILTIN_WORDLENGTH)
    KWD(CHAR_WORDPOS,     BUILTIN_WORDPOS)
    KWD(CHAR_WORDS,       BUILTIN_WORDS)
    KWD(CHAR_X2B,         BUILTIN_X2B)
    KWD(CHAR_X2C,         BUILTIN_X2C)
    KWD(CHAR_X2D,         BUILTIN_X2D)
    KWD(CHAR_XRANGE,      BUILTIN_XRANGE)
};

INT BuiltinFunctionscount =            /* size of builtin function table     */
    sizeof(BuiltinFunctions)/sizeof(KWDTABLE);

/*********************************************************************/
/*                                                                   */
/* Table of condition keywords used for translation                  */
/*                                                                   */
/*********************************************************************/

KWDTABLE ConditionKeywords[] = {       /* condition option table     */
  KWD(CHAR_ANY,            CONDITION_ANY)
  KWD(CHAR_ERROR,          CONDITION_ERROR)
  KWD(CHAR_FAILURE,        CONDITION_FAILURE)
  KWD(CHAR_HALT,           CONDITION_HALT)
  KWD(CHAR_LOSTDIGITS,     CONDITION_LOSTDIGITS)
  KWD(CHAR_NOMETHOD,       CONDITION_NOMETHOD)
  KWD(CHAR_NOSTRING,       CONDITION_NOSTRING)
  KWD(CHAR_NOTREADY,       CONDITION_NOTREADY)
  KWD(CHAR_NOVALUE,        CONDITION_NOVALUE)
  KWD(CHAR_PROPAGATE,      CONDITION_PROPAGATE)
  KWD(CHAR_SYNTAX,         CONDITION_SYNTAX)
  KWD(CHAR_USER,           CONDITION_USER)
};

INT ConditionKeywordscount =            /* size of condition keyword table     */
    sizeof(ConditionKeywords)/sizeof(KWDTABLE);

/*********************************************************************/
/*                                                                   */
/* Table of parse options used for translation                       */
/*                                                                   */
/*********************************************************************/

KWDTABLE ParseOptions[] = {            /* parse option table         */
  KWD(CHAR_ARG,           SUBKEY_ARG)
  KWD(CHAR_CASELESS,      SUBKEY_CASELESS)
  KWD(CHAR_LINEIN,        SUBKEY_LINEIN)
  KWD(CHAR_LOWER,         SUBKEY_LOWER)
  KWD(CHAR_PULL,          SUBKEY_PULL)
  KWD(CHAR_SOURCE,        SUBKEY_SOURCE)
  KWD(CHAR_UPPER,         SUBKEY_UPPER)
  KWD(CHAR_VALUE,         SUBKEY_VALUE)
  KWD(CHAR_VAR,           SUBKEY_VAR)
  KWD(CHAR_VERSION,       SUBKEY_VERSION)
};

INT ParseOptionscount =            /* size of Parse options table     */
    sizeof(ParseOptions)/sizeof(KWDTABLE);

/*********************************************************************/
/*                                                                   */
/* Table of directive subkeywords used for translation               */
/*                                                                   */
/*********************************************************************/

KWDTABLE SubDirectives[] = {           /* language directive subkeywords    */
   KWD(CHAR_ABSTRACT,    SUBDIRECTIVE_ABSTRACT)
   KWD(CHAR_ATTRIBUTE,   SUBDIRECTIVE_ATTRIBUTE)
   KWD(CHAR_CLASS,       SUBDIRECTIVE_CLASS)
   KWD(CHAR_EXTERNAL,    SUBDIRECTIVE_EXTERNAL)
   KWD(CHAR_GET,         SUBDIRECTIVE_GET)
   KWD(CHAR_GUARDED,     SUBDIRECTIVE_GUARDED)
   KWD(CHAR_INHERIT,     SUBDIRECTIVE_INHERIT)
   KWD(CHAR_METACLASS,   SUBDIRECTIVE_METACLASS)
   KWD(CHAR_MIXINCLASS,  SUBDIRECTIVE_MIXINCLASS)
   KWD(CHAR_PRIVATE,     SUBDIRECTIVE_PRIVATE)
   KWD(CHAR_PROTECTED,   SUBDIRECTIVE_PROTECTED)
   KWD(CHAR_PUBLIC,      SUBDIRECTIVE_PUBLIC)
   KWD(CHAR_SET,         SUBDIRECTIVE_SET)
   KWD(CHAR_SUBCLASS,    SUBDIRECTIVE_SUBCLASS)
   KWD(CHAR_UNGUARDED,   SUBDIRECTIVE_UNGUARDED)
   KWD(CHAR_UNPROTECTED, SUBDIRECTIVE_UNPROTECTED)
};

INT SubDirectivescount =            /* size of function table     */
    sizeof(SubDirectives)/sizeof(KWDTABLE);
