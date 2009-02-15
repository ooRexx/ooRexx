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
/* REXX Kernel                                                  Token.hpp  */
/*                                                                            */
/* Primitive Translator Token Class Definitions                               */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxToken
#define Included_RexxToken

#include "SourceLocation.hpp"


#define   TERM_EOC     0x00000001u     /* terminate on end of clause        */
#define   TERM_RIGHT   0x00000002u     /* terminate on left paren           */
#define   TERM_SQRIGHT 0x00000004u     /* terminate on left square bracket  */
#define   TERM_TO      0x00000008u     /* terminate on TO keyword           */
#define   TERM_BY      0x00000010u     /* terminate on BY keyword           */
#define   TERM_FOR     0x00000020u     /* terminate on FOR keyword          */
#define   TERM_WHILE   0x00000040u     /* terminate on WHILE/UNTIL keywords */
#define   TERM_COMMA   0x00000080u     /* terminate on comma                */
#define   TERM_WITH    0x00000100u     /* terminate on WITH keyword         */
#define   TERM_THEN    0x00000200u     /* terminate on THEN keyword         */
#define   TERM_KEYWORD 0x10000000u     /* perform keyword terminator checks */
                                       /* terminate on DO keywords          */
#define   TERM_CONTROL (TERM_KEYWORD | TERM_TO | TERM_BY | TERM_FOR | TERM_WHILE | TERM_EOC)
                                       /* terminate on DO conditionals      */
#define   TERM_COND    (TERM_KEYWORD | TERM_WHILE | TERM_EOC)

#define   TERM_IF      (TERM_KEYWORD | TERM_THEN | TERM_EOC)

/* token types */
#define TOKEN_NULL        1201
#define TOKEN_BLANK       TOKEN_NULL      + 1
#define TOKEN_SYMBOL      TOKEN_BLANK     + 1
#define TOKEN_LITERAL     TOKEN_SYMBOL    + 1
#define TOKEN_OPERATOR    TOKEN_LITERAL   + 1
#define TOKEN_EOC         TOKEN_OPERATOR  + 1
#define TOKEN_COMMA       TOKEN_EOC       + 1
#define TOKEN_PREFIX      TOKEN_COMMA     + 1
#define TOKEN_LEFT        TOKEN_PREFIX    + 1
#define TOKEN_RIGHT       TOKEN_LEFT      + 1
#define TOKEN_POINT       TOKEN_RIGHT     + 1
#define TOKEN_COLON       TOKEN_POINT     + 1
#define TOKEN_TILDE       TOKEN_COLON     + 1
#define TOKEN_DTILDE      TOKEN_TILDE     + 1
#define TOKEN_SQLEFT      TOKEN_DTILDE    + 1
#define TOKEN_SQRIGHT     TOKEN_SQLEFT    + 1
#define TOKEN_DCOLON      TOKEN_SQRIGHT   + 1
#define TOKEN_CONTINUE    TOKEN_DCOLON    + 1
#define TOKEN_ASSIGNMENT  TOKEN_CONTINUE  + 1

/* token extended types - symbols */
#define SYMBOL_CONSTANT    1251
#define SYMBOL_VARIABLE    1252
#define SYMBOL_NAME        1253
#define SYMBOL_COMPOUND    1254
#define SYMBOL_STEM        1255
#define SYMBOL_DUMMY       1256
#define SYMBOL_DOTSYMBOL   1257
#define INTEGER_CONSTANT   1258
#define LITERAL_HEX        1259
#define LITERAL_BIN        1260

/* token extended types - operators */
#define OPERATOR_PLUS                           1
#define OPERATOR_SUBTRACT                       OPERATOR_PLUS                          + 1
#define OPERATOR_MULTIPLY                       OPERATOR_SUBTRACT                      + 1
#define OPERATOR_DIVIDE                         OPERATOR_MULTIPLY                      + 1
#define OPERATOR_INTDIV                         OPERATOR_DIVIDE                        + 1
#define OPERATOR_REMAINDER                      OPERATOR_INTDIV                        + 1
#define OPERATOR_POWER                          OPERATOR_REMAINDER                     + 1
#define OPERATOR_ABUTTAL                        OPERATOR_POWER                         + 1
#define OPERATOR_CONCATENATE                    OPERATOR_ABUTTAL                       + 1
#define OPERATOR_BLANK                          OPERATOR_CONCATENATE                   + 1
#define OPERATOR_EQUAL                          OPERATOR_BLANK                         + 1
#define OPERATOR_BACKSLASH_EQUAL                OPERATOR_EQUAL                         + 1
#define OPERATOR_GREATERTHAN                    OPERATOR_BACKSLASH_EQUAL               + 1
#define OPERATOR_BACKSLASH_GREATERTHAN          OPERATOR_GREATERTHAN                   + 1
#define OPERATOR_LESSTHAN                       OPERATOR_BACKSLASH_GREATERTHAN         + 1
#define OPERATOR_BACKSLASH_LESSTHAN             OPERATOR_LESSTHAN                      + 1
#define OPERATOR_GREATERTHAN_EQUAL              OPERATOR_BACKSLASH_LESSTHAN            + 1
#define OPERATOR_LESSTHAN_EQUAL                 OPERATOR_GREATERTHAN_EQUAL             + 1
#define OPERATOR_STRICT_EQUAL                   OPERATOR_LESSTHAN_EQUAL                + 1
#define OPERATOR_STRICT_BACKSLASH_EQUAL         OPERATOR_STRICT_EQUAL                  + 1
#define OPERATOR_STRICT_GREATERTHAN             OPERATOR_STRICT_BACKSLASH_EQUAL        + 1
#define OPERATOR_STRICT_BACKSLASH_GREATERTHAN   OPERATOR_STRICT_GREATERTHAN            + 1
#define OPERATOR_STRICT_LESSTHAN                OPERATOR_STRICT_BACKSLASH_GREATERTHAN  + 1
#define OPERATOR_STRICT_BACKSLASH_LESSTHAN      OPERATOR_STRICT_LESSTHAN               + 1
#define OPERATOR_STRICT_GREATERTHAN_EQUAL       OPERATOR_STRICT_BACKSLASH_LESSTHAN     + 1
#define OPERATOR_STRICT_LESSTHAN_EQUAL          OPERATOR_STRICT_GREATERTHAN_EQUAL      + 1
#define OPERATOR_LESSTHAN_GREATERTHAN           OPERATOR_STRICT_LESSTHAN_EQUAL         + 1
#define OPERATOR_GREATERTHAN_LESSTHAN           OPERATOR_LESSTHAN_GREATERTHAN          + 1
#define OPERATOR_AND                            OPERATOR_GREATERTHAN_LESSTHAN          + 1
#define OPERATOR_OR                             OPERATOR_AND                           + 1
#define OPERATOR_XOR                            OPERATOR_OR                            + 1
#define OPERATOR_BACKSLASH                      OPERATOR_XOR                           + 1


/* token extended types - instruction keywords */
#define KEYWORD_ADDRESS            1
#define KEYWORD_ARG                KEYWORD_ADDRESS           + 1
#define KEYWORD_CALL               KEYWORD_ARG               + 1
#define KEYWORD_DO                 KEYWORD_CALL              + 1
#define KEYWORD_DROP               KEYWORD_DO                + 1
#define KEYWORD_EXIT               KEYWORD_DROP              + 1
#define KEYWORD_IF                 KEYWORD_EXIT              + 1
#define KEYWORD_INTERPRET          KEYWORD_IF                + 1
#define KEYWORD_ITERATE            KEYWORD_INTERPRET         + 1
#define KEYWORD_LEAVE              KEYWORD_ITERATE           + 1
#define KEYWORD_METHOD             KEYWORD_LEAVE             + 1
#define KEYWORD_NOP                KEYWORD_METHOD            + 1
#define KEYWORD_NUMERIC            KEYWORD_NOP               + 1
#define KEYWORD_OPTIONS            KEYWORD_NUMERIC           + 1
#define KEYWORD_PARSE              KEYWORD_OPTIONS           + 1
#define KEYWORD_PROCEDURE          KEYWORD_PARSE             + 1
#define KEYWORD_PULL               KEYWORD_PROCEDURE         + 1
#define KEYWORD_PUSH               KEYWORD_PULL              + 1
#define KEYWORD_QUEUE              KEYWORD_PUSH              + 1
#define KEYWORD_REPLY              KEYWORD_QUEUE             + 1
#define KEYWORD_RETURN             KEYWORD_REPLY             + 1
#define KEYWORD_SAY                KEYWORD_RETURN            + 1
#define KEYWORD_SELECT             KEYWORD_SAY               + 1
#define KEYWORD_SIGNAL             KEYWORD_SELECT            + 1
#define KEYWORD_TRACE              KEYWORD_SIGNAL            + 1
#define KEYWORD_VAR                KEYWORD_TRACE             + 1
#define KEYWORD_GUARD              KEYWORD_VAR               + 1
#define KEYWORD_USE                KEYWORD_GUARD             + 1
#define KEYWORD_INITPROC           KEYWORD_USE               + 1
#define KEYWORD_EXPOSE             KEYWORD_INITPROC          + 1
#define KEYWORD_RAISE              KEYWORD_EXPOSE            + 1
#define KEYWORD_ELSE               KEYWORD_RAISE             + 1
#define KEYWORD_THEN               KEYWORD_ELSE              + 1
#define KEYWORD_END                KEYWORD_THEN              + 1
#define KEYWORD_OTHERWISE          KEYWORD_END               + 1
#define KEYWORD_IFTHEN             KEYWORD_OTHERWISE         + 1
#define KEYWORD_WHENTHEN           KEYWORD_IFTHEN            + 1
#define KEYWORD_WHEN               KEYWORD_WHENTHEN          + 1
#define KEYWORD_ASSIGNMENT         KEYWORD_WHEN              + 1
#define KEYWORD_COMMAND            KEYWORD_ASSIGNMENT        + 1
#define KEYWORD_MESSAGE            KEYWORD_COMMAND           + 1
#define KEYWORD_LABEL              KEYWORD_MESSAGE           + 1
#define KEYWORD_ENDIF              KEYWORD_LABEL             + 1
#define KEYWORD_BLOCK              KEYWORD_ENDIF             + 1
#define KEYWORD_FIRST              KEYWORD_BLOCK             + 1
#define KEYWORD_LAST               KEYWORD_FIRST             + 1
#define KEYWORD_ENDELSE            KEYWORD_LAST              + 1
#define KEYWORD_ENDTHEN            KEYWORD_ENDELSE           + 1
#define KEYWORD_ENDWHEN            KEYWORD_ENDTHEN           + 1
#define KEYWORD_REQUIRES           KEYWORD_ENDWHEN           + 1
#define KEYWORD_CLASS              KEYWORD_REQUIRES          + 1
#define KEYWORD_INSTRUCTION        KEYWORD_CLASS             + 1
#define KEYWORD_FORWARD            KEYWORD_INSTRUCTION       + 1
#define KEYWORD_LOOP               KEYWORD_FORWARD           + 1
#define KEYWORD_LIBRARY            KEYWORD_LOOP              + 1

/* token extended types - instruction option keywords */
#define SUBKEY_ARG         1
#define SUBKEY_BY          SUBKEY_ARG         + 1
#define SUBKEY_DIGITS      SUBKEY_BY          + 1
#define SUBKEY_END         SUBKEY_DIGITS      + 1
#define SUBKEY_ELSE        SUBKEY_END         + 1
#define SUBKEY_ENGINEERING SUBKEY_ELSE        + 1
#define SUBKEY_EXPOSE      SUBKEY_ENGINEERING + 1
#define SUBKEY_FOR         SUBKEY_EXPOSE      + 1
#define SUBKEY_FOREVER     SUBKEY_FOR         + 1
#define SUBKEY_FORM        SUBKEY_FOREVER     + 1
#define SUBKEY_FUZZ        SUBKEY_FORM        + 1
#define SUBKEY_LINEIN      SUBKEY_FUZZ        + 1
#define SUBKEY_LOWER       SUBKEY_LINEIN      + 1
#define SUBKEY_CASELESS    SUBKEY_LOWER       + 1
#define SUBKEY_NAME        SUBKEY_CASELESS    + 1
#define SUBKEY_NOVALUE     SUBKEY_NAME        + 1
#define SUBKEY_OFF         SUBKEY_NOVALUE     + 1
#define SUBKEY_ON          SUBKEY_OFF         + 1
#define SUBKEY_OTHERWISE   SUBKEY_ON          + 1
#define SUBKEY_OVER        SUBKEY_OTHERWISE   + 1
#define SUBKEY_PULL        SUBKEY_OVER        + 1
#define SUBKEY_SCIENTIFIC  SUBKEY_PULL        + 1
#define SUBKEY_SOURCE      SUBKEY_SCIENTIFIC  + 1
#define SUBKEY_THEN        SUBKEY_SOURCE      + 1
#define SUBKEY_TO          SUBKEY_THEN        + 1
#define SUBKEY_UNTIL       SUBKEY_TO          + 1
#define SUBKEY_UPPER       SUBKEY_UNTIL       + 1
#define SUBKEY_VALUE       SUBKEY_UPPER       + 1
#define SUBKEY_VAR         SUBKEY_VALUE       + 1
#define SUBKEY_VERSION     SUBKEY_VAR         + 1
#define SUBKEY_WHEN        SUBKEY_VERSION     + 1
#define SUBKEY_WHILE       SUBKEY_WHEN        + 1
#define SUBKEY_WITH        SUBKEY_WHILE       + 1
#define SUBKEY_DESCRIPTION SUBKEY_WITH        + 1
#define SUBKEY_ADDITIONAL  SUBKEY_DESCRIPTION + 1
#define SUBKEY_RESULT      SUBKEY_ADDITIONAL  + 1
#define SUBKEY_ARRAY       SUBKEY_RESULT      + 1
#define SUBKEY_RETURN      SUBKEY_ARRAY       + 1
#define SUBKEY_EXIT        SUBKEY_RETURN      + 1
#define SUBKEY_CONTINUE    SUBKEY_EXIT        + 1
#define SUBKEY_CLASS       SUBKEY_CONTINUE    + 1
#define SUBKEY_MESSAGE     SUBKEY_CLASS       + 1
#define SUBKEY_ARGUMENTS   SUBKEY_MESSAGE     + 1
#define SUBKEY_LABEL       SUBKEY_ARGUMENTS   + 1
#define SUBKEY_STRICT      SUBKEY_LABEL       + 1
#define SUBKEY_TRUE        SUBKEY_STRICT      + 1
#define SUBKEY_FALSE       SUBKEY_TRUE        + 1

/* token extended types - end of clause */
#define CLAUSEEND_EOF         2301
#define CLAUSEEND_SEMICOLON   2302
#define CLAUSEEND_EOL         2303
#define CLAUSEEND_NULL        2304

/* directive types */
#define DIRECTIVE_REQUIRES  2401
#define DIRECTIVE_CLASS     2402
#define DIRECTIVE_METHOD    2403
#define DIRECTIVE_ROUTINE   2404
#define DIRECTIVE_ATTRIBUTE 2405
#define DIRECTIVE_CONSTANT  2406

/* directive sub-keywords */
#define SUBDIRECTIVE_PUBLIC      2501
#define SUBDIRECTIVE_METACLASS   2502
#define SUBDIRECTIVE_INHERIT     2503
#define SUBDIRECTIVE_PRIVATE     2504
#define SUBDIRECTIVE_GUARDED     2505
#define SUBDIRECTIVE_CLASS       2506
#define SUBDIRECTIVE_EXTERNAL    2507
#define SUBDIRECTIVE_SUBCLASS    2508
#define SUBDIRECTIVE_UNGUARDED   2509
#define SUBDIRECTIVE_MIXINCLASS  2510
#define SUBDIRECTIVE_ATTRIBUTE   2511
#define SUBDIRECTIVE_PROTECTED   2512
#define SUBDIRECTIVE_ABSTRACT    2513
#define SUBDIRECTIVE_UNPROTECTED 2514
#define SUBDIRECTIVE_GET         2515
#define SUBDIRECTIVE_SET         2516
#define SUBDIRECTIVE_LIBRARY     2517


/* condition keywords */
#define CONDITION_ANY        2601
#define CONDITION_ERROR      2602
#define CONDITION_FAILURE    2603
#define CONDITION_HALT       2604
#define CONDITION_NOMETHOD   2605
#define CONDITION_NOSTRING   2606
#define CONDITION_NOTREADY   2607
#define CONDITION_NOVALUE    2608
#define CONDITION_PROPAGATE  2609
#define CONDITION_SYNTAX     2610
#define CONDITION_USER       2611
#define CONDITION_LOSTDIGITS 2612

/* builtin function codes */
#define  NO_BUILTIN                0   /* builtin function not found        */

#define  BUILTIN_ABBREV            1
#define  BUILTIN_ABS               BUILTIN_ABBREV           + 1
#define  BUILTIN_ADDRESS           BUILTIN_ABS              + 1
#define  BUILTIN_ARG               BUILTIN_ADDRESS          + 1
#define  BUILTIN_B2X               BUILTIN_ARG              + 1
#define  BUILTIN_BITAND            BUILTIN_B2X              + 1
#define  BUILTIN_BITOR             BUILTIN_BITAND           + 1
#define  BUILTIN_BITXOR            BUILTIN_BITOR            + 1
#define  BUILTIN_C2D               BUILTIN_BITXOR           + 1
#define  BUILTIN_C2X               BUILTIN_C2D              + 1
#define  BUILTIN_CENTER            BUILTIN_C2X              + 1
#define  BUILTIN_CENTRE            BUILTIN_CENTER           + 1
#define  BUILTIN_CHANGESTR         BUILTIN_CENTRE           + 1
#define  BUILTIN_CHARIN            BUILTIN_CHANGESTR        + 1
#define  BUILTIN_CHAROUT           BUILTIN_CHARIN           + 1
#define  BUILTIN_CHARS             BUILTIN_CHAROUT          + 1
#define  BUILTIN_COMPARE           BUILTIN_CHARS            + 1
#define  BUILTIN_CONDITION         BUILTIN_COMPARE          + 1
#define  BUILTIN_COPIES            BUILTIN_CONDITION        + 1
#define  BUILTIN_COUNTSTR          BUILTIN_COPIES           + 1
#define  BUILTIN_D2C               BUILTIN_COUNTSTR         + 1
#define  BUILTIN_D2X               BUILTIN_D2C              + 1
#define  BUILTIN_DATATYPE          BUILTIN_D2X              + 1
#define  BUILTIN_DATE              BUILTIN_DATATYPE         + 1
#define  BUILTIN_DELSTR            BUILTIN_DATE             + 1
#define  BUILTIN_DELWORD           BUILTIN_DELSTR           + 1
#define  BUILTIN_DIGITS            BUILTIN_DELWORD          + 1
#define  BUILTIN_ERRORTEXT         BUILTIN_DIGITS           + 1
#define  BUILTIN_FORM              BUILTIN_ERRORTEXT        + 1
#define  BUILTIN_FORMAT            BUILTIN_FORM             + 1
#define  BUILTIN_FUZZ              BUILTIN_FORMAT           + 1
#define  BUILTIN_INSERT            BUILTIN_FUZZ             + 1
#define  BUILTIN_LASTPOS           BUILTIN_INSERT           + 1
#define  BUILTIN_LEFT              BUILTIN_LASTPOS          + 1
#define  BUILTIN_LENGTH            BUILTIN_LEFT             + 1
#define  BUILTIN_LINEIN            BUILTIN_LENGTH           + 1
#define  BUILTIN_LINEOUT           BUILTIN_LINEIN           + 1
#define  BUILTIN_LINES             BUILTIN_LINEOUT          + 1
#define  BUILTIN_MAX               BUILTIN_LINES            + 1
#define  BUILTIN_MIN               BUILTIN_MAX              + 1
#define  BUILTIN_OVERLAY           BUILTIN_MIN              + 1
#define  BUILTIN_POS               BUILTIN_OVERLAY          + 1
#define  BUILTIN_QUEUED            BUILTIN_POS              + 1
#define  BUILTIN_RANDOM            BUILTIN_QUEUED           + 1
#define  BUILTIN_REVERSE           BUILTIN_RANDOM           + 1
#define  BUILTIN_RIGHT             BUILTIN_REVERSE          + 1
#define  BUILTIN_SIGN              BUILTIN_RIGHT            + 1
#define  BUILTIN_SOURCELINE        BUILTIN_SIGN             + 1
#define  BUILTIN_SPACE             BUILTIN_SOURCELINE       + 1
#define  BUILTIN_STREAM            BUILTIN_SPACE            + 1
#define  BUILTIN_STRIP             BUILTIN_STREAM           + 1
#define  BUILTIN_SUBSTR            BUILTIN_STRIP            + 1
#define  BUILTIN_SUBWORD           BUILTIN_SUBSTR           + 1
#define  BUILTIN_SYMBOL            BUILTIN_SUBWORD          + 1
#define  BUILTIN_TIME              BUILTIN_SYMBOL           + 1
#define  BUILTIN_TRACE             BUILTIN_TIME             + 1
#define  BUILTIN_TRANSLATE         BUILTIN_TRACE            + 1
#define  BUILTIN_TRUNC             BUILTIN_TRANSLATE        + 1
#define  BUILTIN_VALUE             BUILTIN_TRUNC            + 1
#define  BUILTIN_VAR               BUILTIN_VALUE            + 1
#define  BUILTIN_VERIFY            BUILTIN_VAR              + 1
#define  BUILTIN_WORD              BUILTIN_VERIFY           + 1
#define  BUILTIN_WORDINDEX         BUILTIN_WORD             + 1
#define  BUILTIN_WORDLENGTH        BUILTIN_WORDINDEX        + 1
#define  BUILTIN_WORDPOS           BUILTIN_WORDLENGTH       + 1
#define  BUILTIN_WORDS             BUILTIN_WORDPOS          + 1
#define  BUILTIN_X2B               BUILTIN_WORDS            + 1
#define  BUILTIN_X2C               BUILTIN_X2B              + 1
#define  BUILTIN_X2D               BUILTIN_X2C              + 1
#define  BUILTIN_XRANGE            BUILTIN_X2D              + 1
// not sorted in alphabetically for security reasons...
// WARNING: this makes tokenized scripts incomptabile with
//          previous tokenized scripts (new tokenized scripts
//          cannot be run on older versions if USERID is used)
//          the magic number in the tokenized format is increased
//          for that reason
#define  BUILTIN_USERID            BUILTIN_XRANGE           + 1
#define  BUILTIN_LOWER             BUILTIN_USERID           + 1
#define  BUILTIN_UPPER             BUILTIN_LOWER            + 1
#define  BUILTIN_RXFUNCADD         BUILTIN_UPPER            + 1
#define  BUILTIN_RXFUNCDROP        BUILTIN_RXFUNCADD        + 1
#define  BUILTIN_RXFUNCQUERY       BUILTIN_RXFUNCDROP       + 1
#define  BUILTIN_ENDLOCAL          BUILTIN_RXFUNCQUERY      + 1
#define  BUILTIN_SETLOCAL          BUILTIN_ENDLOCAL         + 1
#define  BUILTIN_QUEUEEXIT         BUILTIN_SETLOCAL         + 1
#define  BUILTIN_QUALIFY           BUILTIN_QUEUEEXIT        + 1



class RexxToken : public RexxInternalObject {
 public:
  void      *operator new(size_t);
  inline void      *operator new(size_t size, void *ptr) {return ptr;};
  inline void  operator delete(void *) { ; }
  inline void  operator delete(void *, void *) { ; }

  RexxToken(int, int, RexxString *, SourceLocation &);;
  inline RexxToken(RESTORETYPE restoreType) { ; };
  void       live(size_t);
  void       liveGeneral(int reason);
  void       flatten(RexxEnvelope *);
  inline void setStart(size_t l, size_t o) { tokenLocation.setStart(l, o); }
  inline void setEnd(size_t l, size_t o) { tokenLocation.setEnd(l, o); }

  inline bool       isVariable() { return (this->subclass == SYMBOL_VARIABLE || this->subclass == SYMBOL_STEM || this->subclass == SYMBOL_COMPOUND); };
  inline bool       isLiteral()  { return (this->classId == TOKEN_LITERAL); };
  inline bool       isSymbolOrLiteral()  { return (this->classId == TOKEN_LITERAL) || (this->classId == TOKEN_SYMBOL); };
  inline bool       isConstant()  { return (this->classId == TOKEN_SYMBOL && this->subclass != SYMBOL_VARIABLE && this->subclass != SYMBOL_STEM && this->subclass != SYMBOL_COMPOUND); };
  inline bool       isSymbol() { return (this->classId == TOKEN_SYMBOL); };
  inline bool       isOperator() { return (this->classId == TOKEN_OPERATOR); }
  inline bool       isEndOfClause() { return this->classId == TOKEN_EOC; }
  inline void       setNumeric(int v)   { this->numeric = v; };
  inline const SourceLocation &getLocation() { return tokenLocation; }
  inline void  setLocation(SourceLocation &l) { tokenLocation = l; }
         void       checkAssignment(RexxSource *source, RexxString *newValue);

  SourceLocation tokenLocation;        /* token source location             */
  RexxString *value;                   /* token string value                */
  int         classId;                 /* class of token                    */
  int         subclass;                /* specialized type of token         */
  int         numeric;                 /* even further specialization       */
};


inline RexxArray *new_arrayOfTokens(size_t n) { return memoryObject.newObjects(sizeof(RexxToken), n, T_Token); }
inline RexxToken *new_token(int c, int s, RexxString *v, SourceLocation &l) { return new RexxToken (c, s, v, l); }

#endif
