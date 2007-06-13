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
/* REXX Kernel                                                  otpscan.c     */
/*                                                                            */
/* Scanner portion of the REXX Source File Class                              */
/*                                                                            */
/******************************************************************************/
#include <ctype.h>
#include <string.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "SourceFile.hpp"

#include "ASCIIDBCSStrings.hpp"
                                       /* current global settings           */
extern ACTIVATION_SETTINGS *current_settings;

/*********************************************************************/
/*                                                                   */
/* Table of directive instructions used for translation              */
/*                                                                   */
/*********************************************************************/

extern KWDTABLE Directives[];
extern INT      Directivescount;

/*********************************************************************/
/*                                                                   */
/* Table of keyword instructions used for translation                */
/*                                                                   */
/*********************************************************************/

extern KWDTABLE KeywordInstructions[]; /* language keyword table     */
extern INT      KeywordInstructionscount;/* language keyword table size*/

/*********************************************************************/
/*                                                                   */
/* Table of instruction subkeywords used for translation             */
/*                                                                   */
/*********************************************************************/

extern KWDTABLE SubKeywords[];         /* language keyword table     */
extern INT      SubKeywordscount;      /* language keyword table size*/

/*********************************************************************/
/*                                                                   */
/* Table of built-in functions used for translation                  */
/*                                                                   */
/*********************************************************************/

extern KWDTABLE BuiltinFunctions[];    /* built-in function table    */
extern INT      BuiltinFunctionscount; /* builtin function table size*/

/*********************************************************************/
/*                                                                   */
/* Table of condition keywords used for translation                  */
/*                                                                   */
/*********************************************************************/

extern KWDTABLE ConditionKeywords[];   /* condition option table     */
extern INT      ConditionKeywordscount;/* condition option table size*/

/*********************************************************************/
/*                                                                   */
/* Table of parse options used for translation                       */
/*                                                                   */
/*********************************************************************/

extern KWDTABLE ParseOptions[];        /* parse option table         */
extern INT      ParseOptionscount;     /* parse option table size    */

/*********************************************************************/
/*                                                                   */
/* Table of directive subkeywords used for translation               */
/*                                                                   */
/*********************************************************************/

extern KWDTABLE SubDirectives[];/* language directive subkeywords    */
extern INT      SubDirectivescount;    /* language directive         */
                                       /* subkeywords table size     */

/*********************************************************************/
/*                                                                   */
/*   Subroutine Name:   resolve_keyword                              */
/*                                                                   */
/*   Descriptive Name:  search keyword table for a match             */
/*                                                                   */
/*   Function:          look up keyword in table                     */
/*                        return 0 if not found                      */
/*                                                                   */
/*********************************************************************/

INT resolve_keyword(
  PCHAR     Name,                      /* name to search                    */
  size_t    Length,                    /* length of the name                */
  KWDTABLE *Table,                     /* keyword table to use              */
  INT       Table_Size )               /* size of keyword table             */
{
  INT       Upper;                     /* search upper bound         */
  INT       Lower;                     /* search lower bound         */
  INT       Middle;                    /* search middle bound        */
  CHAR      FirstChar;                 /* first search character     */
  INT       rc;                        /* comparison result          */

  Lower = 0;                           /* set initial lower bound    */
  Upper = Table_Size - 1;              /* set the upper bound        */
  FirstChar = *Name;                   /* get the first character    */

  while (Lower <= Upper) {             /* while still a range        */
                                       /* set new middle location    */
    Middle = Lower + ((Upper - Lower) / 2);
                                       /* if first character matches */
    if (*Table[Middle].name == FirstChar) {
      rc = memcmp(Name, Table[Middle].name, min(Length, Table[Middle].length));
      if (!rc) {                       /* compared equal             */
                                       /* lengths equal?             */
        if (Length == Table[Middle].length)
                                       /* return this keyword code   */
          return Table[Middle].keyword_code;
                                       /* have to go up?             */
        else if (Length > Table[Middle].length)
          Lower = Middle + 1;          /* set new lower bound        */
        else                           /* going down                 */
          Upper = Middle - 1;          /* set new upper bound        */
      }
      else if (rc > 0)                 /* name is larger             */
        Lower = Middle + 1;            /* set new lower bound        */
      else                             /* going down                 */
        Upper = Middle - 1;            /* set new upper bound        */
    }
                                       /* name is larger             */
    else if (*Table[Middle].name < FirstChar)
      Lower = Middle + 1;              /* set new lower bound        */
    else                               /* going down                 */
      Upper = Middle - 1;              /* set new upper bound        */
  }
  return 0;                            /* return failure flag        */
}

int RexxSource::subKeyword(
    RexxToken  *token)                 /* token to check                    */
/******************************************************************************/
/* Function:  Return a numeric subkeyword identifier for a token              */
/******************************************************************************/
{
  RexxString *value;                   /* token value                       */

  if (token->classId != TOKEN_SYMBOL)  /* not a symbol?                     */
    return 0;                          /* not a keyword                     */
  else {
    value = token->value;              /* get the token's value             */
                                       /* perform keyword table search      */
    return resolve_keyword((PCHAR)value->stringData, value->length, SubKeywords, SubKeywordscount);
  }
}

int RexxSource::keyword(
    RexxToken  *token)                 /* token to check                    */
/****************************************************************************/
/* Function:  Return a numeric keyword identifier for a token               */
/****************************************************************************/
{
  RexxString *value;                   /* token value                       */

  if (token->classId != TOKEN_SYMBOL)  /* not a symbol?                     */
    return 0;                          /* not a keyword                     */
  else {
    value = token->value;              /* get the token's value             */
                                       /* perform keyword table search      */
    return resolve_keyword((PCHAR)value->stringData, value->length, KeywordInstructions, KeywordInstructionscount);
  }
}

int RexxSource::builtin(
    RexxToken  *token)                 /* token to check                    */
/****************************************************************************/
/* Function:  Return a numeric builtin function identifier for a token      */
/****************************************************************************/
{
  RexxString *value;                   /* token value                       */

  value = token->value;                /* get the token's value             */
                                       /* perform keyword table search      */
  return resolve_keyword((PCHAR)value->stringData, value->length, BuiltinFunctions, BuiltinFunctionscount);
}


int RexxSource::resolveBuiltin(
    RexxString *value)                 /* name to check                     */
/******************************************************************************/
/* Function:  Return a numeric keyword identifier for a string                */
/******************************************************************************/
{
                                       /* perform keyword table search      */
  return resolve_keyword(value->stringData, value->length, BuiltinFunctions, BuiltinFunctionscount);
}

int RexxSource::condition(
    RexxToken  *token)                 /* token to check                    */
/****************************************************************************/
/* Function:  Return a numeric condition identifier for a token             */
/****************************************************************************/
{
  RexxString *value;                   /* token value                       */

  if (token->classId != TOKEN_SYMBOL)  /* not a symbol?                     */
    return 0;                          /* not a keyword                     */
  else {
    value = token->value;              /* get the token's value             */
                                       /* perform keyword table search      */
    return resolve_keyword((PCHAR)value->stringData, value->length, ConditionKeywords, ConditionKeywordscount);
  }
}

int RexxSource::parseOption(
    RexxToken  *token)                 /* token to check                    */
/****************************************************************************/
/* Function:  Return a numeric condition identifier for a token             */
/****************************************************************************/
{
  RexxString *value;                   /* token value                       */

  if (token->classId != TOKEN_SYMBOL)  /* not a symbol?                     */
    return 0;                          /* not a keyword                     */
  else {
    value = token->value;              /* get the token's value             */
                                       /* perform keyword table search      */
    return resolve_keyword((PCHAR)value->stringData, value->length, ParseOptions, ParseOptionscount);
  }
}

int RexxSource::keyDirective(
    RexxToken  *token)                 /* token to check                    */
/****************************************************************************/
/* Function:  Return a numeric directive identifier for a token             */
/****************************************************************************/
{
  RexxString *value;                   /* token value                       */

  if (token->classId != TOKEN_SYMBOL)  /* not a symbol?                     */
    return 0;                          /* not a keyword                     */
  else {
    value = token->value;              /* get the token's value             */
                                       /* perform keyword table search      */
    return resolve_keyword((PCHAR)value->stringData, value->length, Directives, Directivescount);
  }
}

int RexxSource::subDirective(
    RexxToken  *token)                 /* token to check                    */
/****************************************************************************/
/* Function:  Return a numeric directive option identifier for a token      */
/****************************************************************************/
{
  RexxString *value;                   /* token value                       */

  if (token->classId != TOKEN_SYMBOL)  /* not a symbol?                     */
    return 0;                          /* not a keyword                     */
  else {
    value = token->value;              /* get the token's value             */
                                       /* perform keyword table search      */
    return resolve_keyword((PCHAR)value->stringData, value->length, SubDirectives, SubDirectivescount);
  }
}

int RexxSource::precedence(
    RexxToken  *token)                 /* target token                      */
/******************************************************************************/
/* Fucntion:  Determine a token's operator precedence                         */
/******************************************************************************/
{
  INT   precedence;                    /* returned operator precedence      */

  switch (token->subclass) {           /* process based on subclass         */

    default:
      precedence = 0;                  /* this is the bottom of the heap    */
      break;

    case OPERATOR_OR:
    case OPERATOR_XOR:
      precedence = 1;                  /* various OR types are next         */
      break;

    case OPERATOR_AND:
      precedence = 2;                  /* AND operator ahead of ORs         */
      break;

    case OPERATOR_EQUAL:               /* comparisons are all together      */
    case OPERATOR_BACKSLASH_EQUAL:
    case OPERATOR_GREATERTHAN:
    case OPERATOR_BACKSLASH_GREATERTHAN:
    case OPERATOR_LESSTHAN:
    case OPERATOR_BACKSLASH_LESSTHAN:
    case OPERATOR_GREATERTHAN_EQUAL:
    case OPERATOR_LESSTHAN_EQUAL:
    case OPERATOR_STRICT_EQUAL:
    case OPERATOR_STRICT_BACKSLASH_EQUAL:
    case OPERATOR_STRICT_GREATERTHAN:
    case OPERATOR_STRICT_BACKSLASH_GREATERTHAN:
    case OPERATOR_STRICT_LESSTHAN:
    case OPERATOR_STRICT_BACKSLASH_LESSTHAN:
    case OPERATOR_STRICT_GREATERTHAN_EQUAL:
    case OPERATOR_STRICT_LESSTHAN_EQUAL:
    case OPERATOR_LESSTHAN_GREATERTHAN:
    case OPERATOR_GREATERTHAN_LESSTHAN:
      precedence = 3;                  /* concatenates are next             */
      break;

    case OPERATOR_ABUTTAL:
    case OPERATOR_CONCATENATE:
    case OPERATOR_BLANK:
      precedence = 4;                  /* concatenates are next             */
      break;

    case OPERATOR_PLUS:
    case OPERATOR_SUBTRACT:
      precedence = 5;                  /* plus and minus next               */
      break;

    case OPERATOR_MULTIPLY:
    case OPERATOR_DIVIDE:
    case OPERATOR_INTDIV:
    case OPERATOR_REMAINDER:
      precedence = 6;                  /* mulitiply and divide afer simples */
      break;

    case OPERATOR_POWER:
      precedence = 7;                  /* almost the top of the heap        */
      break;

    case OPERATOR_BACKSLASH:
      precedence = 8;                  /* NOT is the top honcho             */
      break;
  }
  return precedence;                   /* return the operator precedence    */
}

RexxSource::RexxSource(
    RexxString *programname,           /* source program name               */
    RexxArray  *source_array )         /* program source array              */
/******************************************************************************/
/* Function:  Initialize a source object                                      */
/******************************************************************************/
{
  ClearObject(this);                   /* start completely clean            */
  this->hashvalue = HASHOREF(this);    /* fill in the hash value            */
                                       /* fill in the name                  */
  OrefSet(this, this->programName, programname);
                                       /* fill in the source array          */
  OrefSet(this, this->sourceArray, source_array);
  if (this->sourceArray) {             /* have an array?                    */
                                       /* fill in the source size           */
    this->line_count = sourceArray->size();
    this->position(1, 0);              /* set position at the first line    */
  }
}

/*********************************************************************
*  The following table detects alphanumeric characters and           *
*  special characters that can be part of an REXX symbol.            *
*  The table also convert lower case letters to upper case.          *
*********************************************************************/
INT lookup[]={
  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  /*   0 -   9 */
  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  /*  10 -  19 */
  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  /*  20 -  29 */
  0, 0, 0,33, 0,  0, 0, 0, 0, 0,  /*  30 -  39 (33 is !) */
  0, 0, 0, 0, 0,  0,46, 0,48,49,  /*  40 -  49 (46 is . 48 is 0) */
 50,51,52,53,54, 55,56,57, 0, 0,  /*  50 -  59 (57 is 9) */
  0, 0, 0,63, 0, 65,66,67,68,69,  /*  60 -  69 (63 is ? 65 is A) */
 70,71,72,73,74, 75,76,77,78,79,  /*  70 -  79 */
 80,81,82,83,84, 85,86,87,88,89,  /*  80 -  89 */
 90, 0, 0, 0, 0, 95, 0,65,66,67,  /*  90 -  99 (95 is _  97 is a and */
                                  /*                     becomes A)  */
 68,69,70,71,72, 73,74,75,76,77,  /* 100 - 109 */
 78,79,80,81,82, 83,84,85,86,87,  /* 110 - 119 */
 88,89,90, 0, 0,  0, 0, 0, 0, 0,  /* 120 - 129 */
  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  /* 130 - 139 */
  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  /* 140 - 149 */
  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  /* 150 - 159 */
  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  /* 160 - 169 */
  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  /* 170 - 179 */
  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  /* 180 - 189 */
  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  /* 190 - 199 */
  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  /* 200 - 209 */
  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  /* 210 - 219 */
  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  /* 220 - 229 */
  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  /* 230 - 239 */
  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  /* 240 - 249 */
  0, 0, 0, 0, 0,  0               /* 250 - 255 */
};

                                       /* some macros for commonly coded    */
                                       /* scanning operations...mostly to   */
                                       /* save some keystrokes and make     */
                                       /* things a little more readable     */
#define GETCHAR()  ((UCHAR)(this->current[this->line_offset]))
#define MORELINE() (this->line_offset < this->current_length)
#define OPERATOR(op) (this->clause->newToken(TOKEN_OPERATOR, OPERATOR_##op, (RexxString *)OREF_##op, &location))
#define CHECK_ASSIGNMENT(op, token) (token->checkAssignment(this, (RexxString *)OREF_ASSIGNMENT_##op))

void RexxSource::endLocation(
  LOCATIONINFO *location )             /* token location information        */
/****************************************************************************/
/* Function:  Record a tokens ending location                               */
/****************************************************************************/
{
                                       /* record the ending line            */
  location->endline = this->line_number;
                                       /* record the ending position        */
  location->endoffset = this->line_offset;
}

BOOL RexxSource::nextSpecial(
  UINT          target,                /* desired target character          */
  LOCATIONINFO *location )             /* token location information        */
/****************************************************************************/
/* Function:  Find the next special character and verify against a target   */
/****************************************************************************/
{
  UINT    inch;                        /* next character                    */
  BOOL   found;                        /* found the target flag             */

  found = FALSE;                       /* default to not found              */
  inch = this->locateToken(OREF_NULL); /* find the next token               */
                                       /* have something else on this line? */
  if (inch != CLAUSEEND_EOF && inch != CLAUSEEND_EOL) {
    if (GETCHAR() == target) {         /* is the next character a match?    */
      this->line_offset++;             /* step over the next                */
      this->endLocation(location);     /* update the end location part      */
      found = TRUE;                    /* got what we need!                 */
    }
  }
  return found;                        /* give back the flag                */
}

void RexxSource::comment()
/****************************************************************************/
/* Function:  Scan source to skip over a nest of comments                   */
/****************************************************************************/
{
  INT    level;                        /* comment nesting level             */
  UINT   inch;                         /* next character                    */
  LONG   startline;                    /* starting line for error reports   */

  if (this->flags&DBCS_scanning) {     /* possible DBCS characters?         */
    this->DBCScomment();               /* use the DBCS version              */
    return;                            /* finished                          */
  }

  level = 1;                           /* start the comment nesting         */
  this->line_offset += 2;              /* step over the comment start       */
  startline = this->line_number;       /* remember the starting position    */
  while (level > 0) {                  /* while still in a comment nest     */
                                       /* hit the end of a line?            */
    if (this->line_offset >= this->current_length) {
      this->nextLine();                /* need to go to the next line       */
                                       /* no more lines?                    */
      if (this->line_number > this->line_count) {
                                       /* record current position in clause */
        this->clause->setEnd(this->line_count, this->line_offset);
                                       /* error, must report                */
        report_error1(Error_Unmatched_quote_comment, new_integer(startline));
      }
      continue;                        /* go loop around                    */
    }
    inch = GETCHAR();                  /* get the next character            */
    this->line_offset++;               /* step past the character           */
                                       /* is this the end delimeter?        */
    if (inch == '*' && GETCHAR() == '/') {
      level--;                         /* reduce the nesting level          */
      this->line_offset++;             /* step the pointer over the close   */
    }
                                       /* start of a new comment?           */
    else if (inch == '/' && GETCHAR() == '*') {
      level++;                         /* increment the level               */
      this->line_offset++;             /* step the pointer over new start   */
    }
  }
}

void RexxSource::DBCScomment()
/****************************************************************************/
/* Function:  Scan source to skip over a nest of comments                   */
/****************************************************************************/
{
  INT    level;                        /* comment nesting level             */
  UINT   inch;                         /* next character                    */
  LONG   startline;                    /* starting line for error reports   */

  level = 1;                           /* start the comment nesting         */
  this->line_offset += 2;              /* step over the comment start       */
  startline = this->line_number;       /* remember the starting position    */
  while (level > 0) {                  /* while still in a comment nest     */
                                       /* hit the end of a line?            */
    if (this->line_offset >= this->current_length) {
      this->nextLine();                /* need to go to the next line       */
                                       /* no more lines?                    */
      if (this->line_number > this->line_count) {
                                       /* record current position in clause */
        this->clause->setEnd(this->line_number, this->line_offset);
                                       /* error, must report                */
        report_error1(Error_Unmatched_quote_comment, new_integer(startline));
      }
      continue;                        /* go loop around                    */
    }
    inch = GETCHAR();                  /* get the next character            */
    this->line_offset++;               /* step past the character           */
                                       /* is this the end delimeter?        */
    if (inch == '*' && GETCHAR() == '/') {
      level--;                         /* reduce the nesting level          */
      this->line_offset++;             /* step the pointer over the close   */
    }
                                       /* start of a new comment?           */
    else if (inch == '/' && GETCHAR() == '*') {
      level++;                         /* increment the level               */
      this->line_offset++;             /* step the pointer over new start   */
    }
    else if (IsDBCS(inch)) {           /* got a DBCS character?             */
      this->line_offset++;             /* step to the next character        */
      if (!MORELINE()) {               /* end of the line?                  */
                                       /* record current position in clause */
        this->clause->setEnd(this->line_number, this->line_offset);
                                       /* raise the appropriate error       */
        report_error(Error_Invalid_character_string_DBCS);
      }
    }
  }
}

UINT RexxSource::locateToken(
  RexxToken *previous )                /* previous token                    */
/****************************************************************************/
/* Function:  Locate next significant token in source, skipping extra       */
/*            blanks and comments.                                          */
/****************************************************************************/
{
 UINT    inch;                         /* current input character           */
 UINT    inch2;                        /* secondary input character         */
 LONG    startline;                    /* backward reset line number        */
 LONG    startoffset;                  /* backward reset offset             */
 UINT    character;                    /* returned character type           */
 BOOL    blanks;                       /* are blanks significant?           */

 character = 0;                        /* no specific character type yet    */
                                       /* check if blanks should be returned*/
 if (previous != OREF_NULL &&          /* no previous token, or             */
                                       /* have a symbol, literal, right     */
                                       /* paren or right square bracket     */
     (previous->classId == TOKEN_SYMBOL ||
      previous->classId == TOKEN_LITERAL ||
      previous->classId == TOKEN_RIGHT ||
      previous->classId == TOKEN_SQRIGHT))
   blanks = TRUE;                      /* blanks are significant here       */
 else
   blanks = FALSE;                     /* not looking for blanks            */

                                       /* no more lines in file?            */
 if (this->line_number > this->line_count)
   character = CLAUSEEND_EOF;          /* return an end-of-file             */
 else if (!MORELINE())                 /* reached the line end?             */
   character = CLAUSEEND_EOL;          /* return an end-of-line             */
 else {
                                       /* while more program to scan        */
   while (this->line_offset < this->current_length) {
     inch = GETCHAR();                 /* get the next character            */
     if (inch==' ' || inch=='\t') {    /* blank or tab?                     */
       if (blanks) {                   /* is this significant?              */
         character = TOKEN_BLANK;      /* have a blank character            */
         break;                        /* got what we need                  */
       }
       else {
         this->line_offset++;          /* step the position                 */
         continue;                     /* go around again                   */
       }
     }
                                       /* possible continuation character?  */
     else if (inch == ',' || inch == '-') {
                                       /* line comment?                     */
       if (inch == '-' && this->line_offset + 1 < this->current_length &&
                          this->current[this->line_offset + 1] == '-') {
         this->line_offset = this->current_length;
         break;
       }

       character = inch;               /* assume for now real character     */
       /* we check for EOL (possibly following blanks and comments)         */
       startoffset = this->line_offset;/* remember the location             */
       startline = this->line_number;  /* remember the line position        */
       this->line_offset++;            /* step the position                 */

                                       /* skip blanks and comments          */
       while (this->line_offset < this->current_length) {
         inch2 = GETCHAR();            /* pick up the next character        */
                                       /* comment level start?              */
         if (inch2 == '/' && (this->line_offset + 1 < this->current_length) &&
             this->current[this->line_offset + 1] == '*') {
           this->comment();            /* go skip over the comment          */
           continue;                   /* and continue scanning             */
         }
                                       /* line comment?                     */
         if (inch2 == '-' && (this->line_offset + 1 < this->current_length) &&
             this->current[this->line_offset + 1] == '-') {
                                       /* go skip overto the end of line    */
           this->line_offset = this->current_length;
           break;
         }
                                       /* non-blank outside comment         */
         if (inch2 != ' ' && inch2 != '\t')
           break;                      /* done scanning                     */
         this->line_offset++;          /* step over this character          */
       }
                                       /* found an EOL?                     */
       if (this->line_offset >= this->current_length) {
                                       /* more lines in file?               */
         if (this->line_number < this->line_count) {
//                                     /* this is an error                  */
//         report_error(Error_Unexpected_comma_comma);
//       else {
           this->nextLine();           /* step to the next line             */
           if (blanks) {               /* blanks allowed?                   */
             character = TOKEN_BLANK;  /* make this a blank token           */
             break;                    /* finished here                     */
           }
         }
       }
       else {                          /* reset to the starting position    */
         this->position(startline, startoffset);
         character = inch;             /* this is a real character          */
         break;                        /* other non-blank, done scanning    */
       }
     }
                                       /* comment level start?              */
     else if (inch == '/' && (this->line_offset + 1 < this->current_length) &&
                              this->current[this->line_offset + 1] == '*')
       this->comment();                /* go skip over the comment          */
//                                     /* line comment?                     */
//   else if (inch == '-' && (this->line_offset + 1 < this->current_length) &&
//            this->current[this->line_offset + 1] == '-') {
//                                     /* go skip overto the end of line    */
//     this->line_offset = this->current_length;
//     break;
//   }
     else {                            /* got the character                 */
       character = inch;               /* this is a good character          */
       break;                          /* done looping                      */
     }
   }
   if (!MORELINE())                    /* fallen off the end of the line?   */
     character = CLAUSEEND_EOL;        /* this is an end of clause          */
 }
 return character;                     /* return the character              */
}

RexxString *RexxSource::packLiteral(
  INT        start,                    /* start of the literal in line      */
  INT        length,                   /* length of the literal to reduce   */
  INT        type )                    /* type of literal to process        */
/****************************************************************************/
/* Function:  Convert and check a hex or binary constant, packing it down   */
/*            into a string object.                                         */
/****************************************************************************/
{
  INT    first;                        /* switch to mark first group        */
  INT    blanks;                       /* switch to say if scanning blanks  */
  INT    count;                        /* count for group                   */
  INT    i;                            /* loop counter                      */
  INT    j;                            /* loop counter                      */
  INT    k;                            /* loop counter                      */
  LONG   m;                            /* temporary integer                 */
  INT    byte;                         /* individual byte of literal        */
  INT    nibble;                       /* individual nibble of literal      */
  INT    oddhex;                       /* odd number of characters in first */
  INT    inpointer;                    /* current input position            */
  INT    outpointer;                   /* current output pointer            */
  RexxString *value;                   /* reduced value                     */
  INT    real_length;                  /* real number of digits in string   */
  CHAR   error_output[2];              /* used for formatting error         */

 first = TRUE;                         /* initialize group flags and        */
 count = 0;                            /* counters                          */
 blanks = FALSE;
 error_output[1] = '\0';               /* terminate string                  */
                                       /* set initial input/output positions*/
 inpointer = start;                    /* get initial starting position     */

 if (!length)                          /* hex or binary null string?        */
   value = OREF_NULLSTRING;            /* this is a null string             */
 else {                                /* data to reduce                    */
  /* first scan is to check REXX rules for validity of grouping             */
  /* and to remove blanks                                                   */

  real_length = length;                /* pick up the string length         */
  for (i = 0; i < length; i++) {       /* loop through entire string        */
                                       /* got a blank?                      */
   if (this->current[inpointer] == ' ' || this->current[inpointer] == '\t') {
     blanks = TRUE;                    /* remember scanning blanks          */
    /* don't like initial blanks or groups after the first                  */
    /* which are not in twos (hex) or fours (binary)                        */
     if (i == 0 ||                     /* if at the beginning               */
         (!first &&                    /* or past first group and not the   */
                                       /* correct size                      */
         (((count&1) && type == LITERAL_HEX) ||
          ((count&3) && type == LITERAL_BIN)))) {
       m = i+1;                        /* place holder for new_integer invocation */
       if (type == LITERAL_HEX)        /* hex string?                       */
                                       /* report correct error              */
         report_error1(Error_Invalid_hex_hexblank, new_integer(m));
       else                            /* need the binary message           */
         report_error1(Error_Invalid_hex_binblank, new_integer(m));
     }
     count = 0;                        /* this starts a new group           */
     real_length--;                    /* this shortens the value           */

   }
   else {
     if (blanks)                       /* had a blank group?                */
       first = FALSE;                  /* no longer on the lead grouping    */
     blanks = FALSE;                   /* not processing blanks now         */
     count++;                          /* count this significant character  */
   }
   inpointer++;                        /* step the input position           */
  }

  if (blanks ||                        /* trailing blanks or                */
      (!first &&                       /* last group isn't correct count?   */
       (((count&1) && type == LITERAL_HEX) ||
       ((count&3) && type == LITERAL_BIN)))) {
    m = i-1;                           /* place holder for new_integer invocation */
    if (type == LITERAL_HEX)           /* hex string?                       */
                                       /* report correct error              */
      report_error1(Error_Invalid_hex_hexblank, new_integer(m));
    else                               /* need the binary message           */
      report_error1(Error_Invalid_hex_binblank, new_integer(m));
  }

  /* second scan is to create the string value determined by the            */
  /* hex or binary constant.                                                */

  i = real_length;                     /* get the adjusted length           */
                                       /* reset the scan pointers           */
  inpointer = start;                   /* reset the scan pointer            */
  outpointer = 0;                      /* set the position a start          */
  if (type == LITERAL_HEX) {           /* hex literal?                      */
    oddhex = i&1;                      /* get any odd count                 */
    i >>= 1;                           /* divide by 2 ... and               */
    i += oddhex;                       /* add in the odd count              */
    value = raw_string(i);             /* get the final value               */

    for (j = 0; j < i; j++) {          /* loop for the appropriate count    */
      byte = 0;                        /* current byte is zero              */
      for (k = oddhex; k < 2; k++) {   /* loop either 1 or 2 times          */
                                       /* get the next nibble               */
        nibble = this->current[inpointer];
        inpointer++;                   /* step to the next character        */
        while (nibble == ' ' || nibble == '\t') {   /* step over any inter-nibble blanks */
                                       /* get the next nibble               */
          nibble = this->current[inpointer];
          inpointer++;                 /* step to the next character        */
        }
                                       /* real digit?                       */
        if (nibble >= '0' && nibble <= '9')
          nibble -= '0';               /* make base zero                    */
                                       /* lowercase hex digit?              */
        else if (nibble >= 'a' && nibble <= 'f') {
          nibble -= 'a';               /* subtract lowest and               */
          nibble += 10;                /* add 10 to digit                   */
        }                              /* uppercase hex digit?              */
        else if (nibble >= 'A' && nibble <= 'F') {
          nibble -= 'A';               /* subtract lowest and               */
          nibble += 10;                /* add 10 to digit                   */
        }
        else {
          error_output[0] = nibble;    /* copy the error character          */
                                       /* report the invalid character      */
          report_error1(Error_Invalid_hex_invhex, new_cstring(&error_output[0]));
        }
        byte <<= 4;                    /* shift the last nibble over        */
        byte += nibble;                /* add in the next nibble            */
      }
      oddhex = 0;                      /* remainder are full bytes          */
      value->putChar(outpointer, byte);/* store this in the output position */
      outpointer++;                    /* step to the next position         */
    }
    value->generateHash();             /* rebuild the hash value            */
    value = this->commonString(value); /* now force to a common string      */
  }
  else {                               /* convert to binary                 */
    oddhex = i&7;                      /* get the leading byte count        */
    if (oddhex)                        /* incomplete byte?                  */
      oddhex = 8 - oddhex;             /* get the padding count             */
    i += oddhex;                       /* and add that into total           */
    i >>= 3;                           /* get the byte count                */
    value = raw_string(i);             /* get the final value               */

    for (j = 0; j < i; j++) {          /* loop through the entire string    */
      byte = 0;                        /* zero the byte                     */
      for (k = oddhex; k < 8; k++) {   /* loop through each byte segment    */
                                       /* get the next bit                  */
        nibble = this->current[inpointer];
        inpointer++;                   /* step to the next character        */
        while (nibble == ' ' || nibble == '\t') {  /* step over any inter-nibble blanks */
                                       /* get the next nibble               */
          nibble = this->current[inpointer];
          inpointer++;                 /* step to the next character        */
        }
        byte <<= 1;                    /* shift the accumulator             */
        if (nibble == '1')             /* got a one bit?                    */
          byte++;                      /* add in the bit                    */
        else if (nibble != '0') {      /* not a '0' either?                 */
          error_output[0] = nibble;    /* copy the error character          */
                                       /* report the invalid character      */
          report_error1(Error_Invalid_hex_invbin, new_cstring(&error_output[0]));
        }
      }
      oddhex = 0;                      /* use 8 bits for the remaining group*/
      value->putChar(outpointer, byte);/* store this in the output position */
      outpointer++;                    /* step to the next position         */
    }
    value->generateHash();             /* rebuild the hash value            */
    value = this->commonString(value); /* now force to a common string      */
  }
 }
 return value;                         /* return newly created string       */
}

RexxToken *RexxSource::sourceNextToken(
    RexxToken *previous )              /* previous token scanned off        */
/*********************************************************************/
/* Extract a token from the source and create a new token object.    */
/* The token type and sub-type are set in the token, and any string  */
/* value extracted.                                                  */
/*********************************************************************/
{
 RexxToken  *token;                    /* working token                     */
 RexxString *value;                    /* associate string value            */
 UINT   inch;                          /* working input character           */
 LONG   eoffset;                       /* location of exponential           */
 INT    state;                         /* state of symbol scanning          */
 LONG   start;                         /* scan start location               */
 LONG   litend;                        /* end of literal data               */
 LONG   length;                        /* length of extracted token         */
 INT    dot_count;                     /* count of periods in symbol        */
 UINT   literal_delimiter;             /* literal string delimiter          */
 INT    type;                          /* type of literal token             */
 LONG   i;                             /* loop counter                      */
 LONG   j;                             /* loop counter                      */
 INT    subclass;                      /* sub type of the token             */
 INT    numeric;                       /* numeric type flag                 */
 LOCATIONINFO  location;               /* token location information        */
 CHAR   tran;                          /* translated character              */
 CHAR   badchar[4];                    /* working buffer for errors         */
 CHAR   hexbadchar[4];                 /* working buffer for errors         */

 /* definitions of states of exponential numeric scan */
#define EXP_START    0
#define EXP_EXCLUDED 1
#define EXP_DIGIT    2
#define EXP_SPOINT   3
#define EXP_POINT    4
#define EXP_E        5
#define EXP_ESIGN    6
#define EXP_EDIGIT   7

  for (;;) {                           /* loop until we find a significant  */
                                       /* token                             */
    inch = this->locateToken(previous);/* locate the next token position    */
                                       /* save the starting point and the   */
                                       /* default ending point              */
    location.line = this->line_number; /* record the starting line          */
                                       /* record the offset position        */
    location.offset = this->line_offset;
                                       /* record the ending line            */
    location.endline = this->line_number;
                                       /* record the ending position        */
    location.endoffset = this->line_offset + 1;
    if (inch == CLAUSEEND_EOF) {       /* reach the end of the source?      */
      token = OREF_NULL;               /* no token to return                */
      break;                           /* finished                          */
    }
    else if (inch == CLAUSEEND_EOL) {  /* some other end-of-clause          */
                                       /* make end the end of the line      */
      location.endoffset = this->current_length;
                                       /* return a clause terminator        */
      token = this->clause->newToken(TOKEN_EOC, CLAUSEEND_EOL, OREF_NULL, &location);
      this->nextLine();                /* step to the next line             */
      break;                           /* have something to return          */
    }
    else if (inch == TOKEN_BLANK ) {   /* some sort of white space?         */
                                       /* now go ahead to the next token    */
      inch = this->locateToken(OREF_NULL);
                                       /* is this blank significant?        */
      if (inch != CLAUSEEND_EOL  &&    /* not at the end                    */
         (lookup[inch] ||              /* and next is a symbol token        */
          inch == '\"' ||              /* or start of a " quoted literal    */
          inch == '\'' ||              /* or start of a ' quoted literal    */
          inch == '('  ||              /* or a left parenthesis             */
          inch == '[')) {              /* or a left square bracket          */
                                       /* return blank token                */
        token = this->clause->newToken(TOKEN_BLANK, OPERATOR_BLANK, (RexxString *)OREF_BLANK, &location);
      }
      else                             /* non-significant blank             */
        continue;                      /* just loop around again            */
    }
    else {                             /* non-special token type            */
                                       /* process different token types     */
      tran = lookup[inch];             /* do the table mapping              */
      if (tran != 0) {                 /* have a symbol character?          */
        state = EXP_START;             /* in a clean state now              */
        eoffset = 0;                   /* no exponential sign yet           */
        start = this->line_offset;     /* remember token start position     */
        dot_count = 0;                 /* no periods yet                    */
        for (;;) {                     /* loop through the token            */
          if (inch == '.')             /* have a period?                    */
            dot_count++;               /* remember we saw this one          */

   /* finite state machine to establish numeric constant (with possible     */
   /* included sign in exponential form)                                    */

          switch (state) {             /* process based on current state    */

            case EXP_START:            /* beginning of scan                 */
                                       /* have a digit at the start?        */
              if (inch >= '0' && inch <= '9')
                state = EXP_DIGIT;     /* now scanning digits               */
              else if (inch == '.')    /* start with a decimal point?       */
                state = EXP_SPOINT;    /* now scanning after the decimal    */
              else                     /* must be a non-numeric character   */
                state = EXP_EXCLUDED;  /* no longer a number                */
              break;                   /* go process the next character     */

            case EXP_DIGIT:            /* have at least one digit mantissa  */
              if (inch=='.')           /* decimal point?                    */
                state = EXP_POINT;     /* we've hit a decimal point         */
              else if (tran=='E')      /* start of exponential?             */
                state = EXP_E;         /* remember we've had the 'E' form   */
                                       /* non-digit?                        */
              else if (inch < '0' && inch > '9')
                state = EXP_EXCLUDED;  /* no longer scanning a number       */
    /* a digit leaves the state unchanged at EXP_DIGIT                      */
              break;                   /* go get the next character         */

            case EXP_SPOINT:           /* leading decimal point             */
                                       /* not a digit?                      */
              if (inch < '0' || inch > '9')
                state = EXP_EXCLUDED;  /* not a number                      */
              else                     /* digit character                   */
                state = EXP_POINT;     /* processing a decimal number       */
              break;                   /* go process the next character     */

            case EXP_POINT:            /* have a decimal point              */
              if (tran == 'E')         /* found the exponential?            */
                state = EXP_E;         /* set exponent state                */
                                       /* non-digit found?                  */
              else if (inch < '0' || inch > '9')
                state = EXP_EXCLUDED;  /* can't be a number                 */
        /* a digit leaves the state unchanged at EXP_POINT                  */
              break;                   /* go get another character          */

            case EXP_E:                /* just had an exponent              */
                                       /* next one a digit?                 */
              if (inch >= '0' && inch <= '9')
                state = EXP_EDIGIT;    /* now looking for exponent digits   */
       /* a sign will be collected by the apparent end of symbol code below */
              break;                   /* finished                          */

            case EXP_ESIGN:            /* just had a signed exponent        */
                                       /* got a digit?                      */
              if (inch >= '0' && inch <= '9')
                state = EXP_EDIGIT;    /* now looking for the exponent      */
              else
                state = EXP_EXCLUDED;  /* can't be a number                 */
              break;                   /* go get the next digits            */

            case EXP_EDIGIT:           /* processing the exponent digits    */
                                       /* not a digit?                      */
              if (inch < '0' || inch > '9')
                state = EXP_EXCLUDED;  /* can't be a number                 */
              break;                   /* go get the next character         */

                   /* once EXP_EXCLUDED is reached the state doesn't change */
          }
          this->line_offset++;         /* step the source pointer           */
                                       /* had a bad exponent part?          */
          if (eoffset && state == EXP_EXCLUDED) {
                                       /* back up the scan pointer          */
            this->line_offset = eoffset;
            break;                     /* and we're finished with this      */
          }
          if (!MORELINE())             /* reached the end of the line?      */
            break;                     /* done processing                   */

          inch = GETCHAR();            /* get the next character            */
          tran = lookup[inch];         /* translate the next character      */
          if (tran != 0)               /* good symbol character?            */
            continue;                  /* loop through the state machine    */
                                       /* check for sign in correct state   */
          if (state == EXP_E && (inch == '+' || inch == '-')) {
                                       /* remember current position         */
            eoffset = this->line_offset;
            state = EXP_ESIGN;         /* now looking for the exponent      */
            this->line_offset++;       /* step past the sign                */
            if (!MORELINE()) {         /* reached the end of the line?      */
              state = EXP_EXCLUDED;    /* can't be a number                 */
              break;                   /* quit looping                      */
            }
            inch = GETCHAR();          /* get the next character            */
            tran = lookup[inch];       /* translate the next character      */
            if (tran != 0)             /* good character?                   */
              continue;                /* loop around                       */
            else {                     /* bad character                     */
              state = EXP_EXCLUDED;    /* not a number                      */
              break;                   /* break out of here                 */
            }
          }
          else
            break;                     /* reached a non-symbol character    */
        }
   /* this must be the end of the symbol - check whether we have too much   */
                                       /* need to step backward?            */
        if (eoffset && state != EXP_EDIGIT)
          this->line_offset = eoffset; /* restore the source pointer        */
                                       /* get the token length              */
        length = this->line_offset - start;
        value = raw_string(length);    /* get the final value               */
        numeric = 0;                   /* not a numeric constant yet        */
        for (i = 0; i < length; i++) { /* copy over and translate the value */
                                       /* copy over the symbol value        */
                                       /* (translating to uppercase         */
                                       /* get the next character            */
          inch = this->current[start + i];
          if (lookup[inch] != 0)       /* normal symbol character (not +/-) */
            inch = lookup[inch];       /* translate to uppercase            */
          value->putChar(i, inch);
        }
        value->setUpperOnly();         /* only contains uppercase           */
        value->generateHash();         /* rebuild the has value             */
                                       /* now force to a common string      */
        value = this->commonString(value);
                                       /* record current position in clause */
        this->clause->setEnd(this->line_number, this->line_offset);
        if (length > MAX_SYMBOL_LENGTH)/* result too long?                  */
                                       /* report the error                  */
          report_error1(Error_Name_too_long_name, value);
        inch = this->current[start];   /* get the first character           */
        if (length == 1 && inch == '.')/* have a solo period?               */
          subclass = SYMBOL_DUMMY;     /* this is the place holder          */
                                       /* have a digit?                     */
        else if (inch >= '0' && inch <= '9') {
          subclass = SYMBOL_CONSTANT;  /* have a constant symbol            */
                                       /* can we optimize to an integer?    */
          if (state == EXP_DIGIT && length < DEFAULT_DIGITS) {
                                       /* no leading zero or only zero?     */
            if (inch != '0' || length == 1)
                                       /* we can make this an integer object*/
              numeric = INTEGER_CONSTANT;
          }
        }
        else if (inch == '.') {        /* may have an environmental symbol  */
                                       /* get the second character          */
          inch = this->current[start + 1];
                                       /* have a digit?                     */
          if (inch >= '0' && inch <= '9')
            subclass = SYMBOL_CONSTANT;/* have a constant symbol            */
          else
                                       /* this is an environment symbol     */
            subclass = SYMBOL_DOTSYMBOL;
        }
        else {                         /* variable type symbol              */
                                       /* set the default extended type     */
          subclass = SYMBOL_VARIABLE;
          if (dot_count > 0) {         /* have a period in the name?        */
                                       /* end in a dot?                     */
            if (dot_count == 1 && value->getChar(length-1) == '.')
                                       /* this is a stem variable           */
              subclass = SYMBOL_STEM;
            else                       /* have a compound variable          */
              subclass = SYMBOL_COMPOUND;
          }
        }
        this->endLocation(&location);  /* record the end position           */
                                       /* get a symbol token                */
        token = this->clause->newToken(TOKEN_SYMBOL, subclass, value, &location);
        token->setNumeric(numeric);    /* record any numeric side info      */
      }
                                       /* start of a quoted string?         */
      else if (inch=='\'' || inch=='\"') {
        literal_delimiter = inch;      /* save the starting character       */
        start = this->line_offset + 1; /* save the starting point           */
        dot_count = 0;                 /* no doubled quotes yet             */
        type = 0;                      /* working with a straight literal   */
                                       /* need to process in DBCS mode?     */
        if (this->flags&DBCS_scanning) {
          for (;;) {                   /* spin through the string           */
            this->line_offset++;       /* step the pointer                  */
            if (!MORELINE()) {         /* reached the end of the line?      */
                                       /* record current position in clause */
              this->clause->setEnd(this->line_number, this->line_offset);
              if (literal_delimiter == '\'')
                                       /* raise the appropriate error       */
                report_error(Error_Unmatched_quote_single);
              else
                                       /* must be a double quote            */
                report_error(Error_Unmatched_quote_double);
            }
            inch = GETCHAR();          /* get the next character            */
                                       /* is this the delimiter?            */
            if (literal_delimiter == inch) {
                                       /* remember end location             */
              litend = this->line_offset - 1;
              this->line_offset++;     /* step to the next character        */
              if (!MORELINE())         /* end of the line?                  */
                break;                 /* we're finished                    */
              inch = GETCHAR();        /* get the next character            */
                                       /* not a doubled quote?              */
              if (inch != literal_delimiter)
                break;                 /* got the end                       */
              dot_count++;             /* remember count of doubled quotes  */
            }
            else if (IsDBCS(inch)) {   /* found a DBCS first character?     */
              this->line_offset++;     /* step to the next character        */
              if (!MORELINE()) {       /* end of the line?                  */
                                       /* record current position in clause */
                this->clause->setEnd(this->line_number, this->line_offset);
                                       /* raise the appropriate error       */
                report_error(Error_Invalid_character_string_DBCS);
              }
            }
          }
        }
        else {
          for (;;) {                   /* spin through the string           */
            this->line_offset++;       /* step the pointer                  */
            if (!MORELINE()) {         /* reached the end of the line?      */
                                       /* record current position in clause */
              this->clause->setEnd(this->line_number, this->line_offset);
              if (literal_delimiter == '\'')
                                       /* raise the appropriate error       */
                report_error(Error_Unmatched_quote_single);
              else
                                       /* must be a double quote            */
                report_error(Error_Unmatched_quote_double);
            }
            inch = GETCHAR();          /* get the next character            */
                                       /* is this the delimiter?            */
            if (literal_delimiter == inch) {
                                       /* remember end location             */
              litend = this->line_offset - 1;
              this->line_offset++;     /* step to the next character        */
              if (!MORELINE())         /* end of the line?                  */
                break;                 /* we're finished                    */
              inch = GETCHAR();        /* get the next character            */
                                       /* not a doubled quote?              */
              if (inch != literal_delimiter)
                break;                 /* got the end                       */
              dot_count++;             /* remember count of doubled quotes  */
            }
          }
        }
        if (MORELINE()) {              /* have more on this line?           */
          inch = GETCHAR();            /* get the next character            */
                                       /* potentially a hex string?         */
          if (inch == 'x' || inch == 'X') {
            this->line_offset++;       /* step to the next character        */
                                       /* the end of the line, or           */
                                       /* have another symbol character     */
            if (MORELINE() && lookup[GETCHAR()] != 0)
              this->line_offset--;     /* step back to the X                */
            else
              type = LITERAL_HEX;      /* set the appropriate type          */
          }
                                       /* potentially a binary string?      */
          else if (inch == 'b' || inch == 'B') {
            this->line_offset++;       /* step to the next character        */
                                       /* the end of the line, or           */
                                       /* have another symbol character     */
            if (MORELINE() && lookup[GETCHAR()] != 0)
              this->line_offset--;     /* step back to the B                */
            else
              type = LITERAL_BIN;      /* set the appropriate type          */
          }
        }
        length = litend - start + 1;   /* calculate the literal length      */
                                       /* record current position in clause */
        this->clause->setEnd(this->line_number, this->line_offset);
        if (type)                      /* need to pack a literal?           */
                                       /* compress into packed form         */
          value = this->packLiteral(start, litend - start + 1, type) ;
        else {
          length = litend - start + 1; /* get length of literal data        */
                                       /* get the final value string        */
          value = raw_string(length - dot_count);
                                       /* copy over and translate the value */
          for (i = 0, j = 0; j < length; i++, j++) {
                                       /* get the next character            */
            inch = this->current[start + j];
                                       /* same as our delimiter?            */
            if (inch == literal_delimiter)
              j++;                     /* step one extra                    */
            value->putChar(i, inch);   /* copy over the literal data        */
          }
          value->generateHash();       /* rebuild the has value             */
                                       /* now force to a common string      */
          value = this->commonString(value);
        }
        this->endLocation(&location);  /* record the end position           */
                                       /* get a string token                */
        token = this->clause->newToken(TOKEN_LITERAL, 0, value, &location);
      }
      else {                           /* other special character           */
       this->line_offset++;            /* step past it                      */

       switch (inch) {                 /* process operators and punctuation */

         case ')':                     /* right parenthesis?                */
                                       /* this is a special character class */
           token = this->clause->newToken(TOKEN_RIGHT, 0, OREF_NULL, &location);
           break;

         case ']':                     /* right square bracket              */
                                       /* this is a special character class */
           token = this->clause->newToken(TOKEN_SQRIGHT, 0, OREF_NULL, &location);
           break;

         case '(':                     /* left parenthesis                  */
                                       /* this is a special character class */
           token = this->clause->newToken(TOKEN_LEFT, 0, OREF_NULL, &location);
           break;

         case '[':                     /* left square bracket               */
                                       /* this is a special character class */
           token = this->clause->newToken(TOKEN_SQLEFT, 0, OREF_NULL, &location);
           break;

         case ',':                     /* comma                             */
                                       /* this is a special character class */
           token = this->clause->newToken(TOKEN_COMMA, 0, OREF_NULL, &location);
           break;

         case ';':                     /* semicolon                         */
                                       /* this is a special character class */
           token = this->clause->newToken(TOKEN_EOC, CLAUSEEND_SEMICOLON, OREF_NULL, &location);
           break;

         case ':':                     /* colon                             */
                                       /* next one a colon also?            */
           if (this->nextSpecial(':', &location))
                                       /* this is a special character class */
             token = this->clause->newToken(TOKEN_DCOLON, 0, OREF_NULL, &location);
           else
                                       /* this is a special character class */
             token = this->clause->newToken(TOKEN_COLON, 0, OREF_NULL, &location);
           break;

         case '~':                     /* message send?                     */
                                       /* next one a tilde also?            */
           if (this->nextSpecial('~', &location))
                                       /* this is a special character class */
             token = this->clause->newToken(TOKEN_DTILDE, 0, OREF_NULL, &location);
           else
                                       /* this is a special character class */
             token = this->clause->newToken(TOKEN_TILDE, 0, OREF_NULL, &location);
           break;

         case '+':                     /* plus sign                         */
                                       /* addition operator                 */
           token = OPERATOR(PLUS);     /* this is an operator class         */
           CHECK_ASSIGNMENT(PLUS, token); // this is allowed as an assignment shortcut
           break;

         case '-':                     /* minus sign                        */
                                       /* subtraction operator              */
           token = OPERATOR(SUBTRACT); /* this is an operator class         */
           CHECK_ASSIGNMENT(SUBTRACT, token); // this is allowed as an assignment shortcut
           break;

         case '%':                     /* percent sign                      */
                                       /* integer divide operator           */
           token = OPERATOR(INTDIV);   /* this is an operator class         */
           CHECK_ASSIGNMENT(INTDIV, token);  // this is allowed as an assignment shortcut
           break;

         case '/':                     /* forward slash                     */
                                       /* this is division                  */
                                       /* next one a slash also?            */
           if (this->nextSpecial('/', &location))
           {

               token = OPERATOR(REMAINDER);
               CHECK_ASSIGNMENT(REMAINDER, token);  // this is allowed as an assignment shortcut
           }
                                       /* this is an operator class         */
           else
           {
               token = OPERATOR(DIVIDE); /* this is an operator class         */
               CHECK_ASSIGNMENT(DIVIDE, token);  // this is allowed as an assignment shortcut
           }
           break;

         case '*':                     /* asterisk?                         */
                                       /* this is multiply                  */
                                       /* next one a star also?             */
           if (this->nextSpecial('*', &location))
           {
               token = OPERATOR(POWER);  /* this is an operator class         */
               CHECK_ASSIGNMENT(POWER, token);  // this is allowed as an assignment shortcut
           }
           else                        /* this is an operator class         */
           {

               token = OPERATOR(MULTIPLY);
               CHECK_ASSIGNMENT(MULTIPLY, token);  // this is allowed as an assignment shortcut
           }
           break;

         case '&':                     /* ampersand?                        */
                                       /* this is the and operator          */
                                       /* next one an ampersand also?       */
           if (this->nextSpecial('&', &location))
           {

               token = OPERATOR(XOR);    /* this is an operator class         */
               CHECK_ASSIGNMENT(XOR, token);  // this is allowed as an assignment shortcut
           }
           else                        /* this is an operator class         */
           {
               token = OPERATOR(AND);
               CHECK_ASSIGNMENT(AND, token);  // this is allowed as an assignment shortcut
           }
           break;

         case '|':                     /* vertical bar?                     */
                                       /* this is an or operator            */
                                       /* next one a vertical bar also?     */
           if (this->nextSpecial('|', &location))
           {
                                       /* this is a concatenation           */
               token = OPERATOR(CONCATENATE);
               CHECK_ASSIGNMENT(CONCATENATE, token);  // this is allowed as an assignment shortcut
           }
           else                        /* this is an operator class         */
           {

               token = OPERATOR(OR);     /* this is the OR operator           */
               CHECK_ASSIGNMENT(OR, token);  // this is allowed as an assignment shortcut
           }
           break;

         case '=':                     /* equal sign?                       */
                                       /* set this an an equal              */
                                       /* next one an equal sign also?      */
           if (this->nextSpecial('=', &location))
                                       /* this is an operator class         */
             token = OPERATOR(STRICT_EQUAL);
           else                        /* this is an operator class         */
             token = OPERATOR(EQUAL);
           break;

         case '<':                     /* less than sign?                   */
                                       /* next one a less than also?        */
           if (this->nextSpecial('<', &location)) {
                                       /* have an equal sign after that?    */
             if (this->nextSpecial('=', &location))
                                       /* this is an operator class         */
               token = OPERATOR(STRICT_LESSTHAN_EQUAL);
             else                      /* this is an operator class         */
               token = OPERATOR(STRICT_LESSTHAN);
           }
                                       /* next one an equal sign?           */
           else if (this->nextSpecial('=', &location))
                                       /* this is the <= operator           */
             token = OPERATOR(LESSTHAN_EQUAL);
                                       /* next one a greater than sign?     */
           else if (this->nextSpecial('>', &location))
                                       /* this is the <> operator           */
             token = OPERATOR(LESSTHAN_GREATERTHAN);
           else                        /* this simply the < operator        */
             token = OPERATOR(LESSTHAN);
           break;

         case '>':                     /* greater than sign?                */
                                       /* next one a greater than also?     */
           if (this->nextSpecial('>', &location)) {
                                       /* have an equal sign after that?    */
             if (this->nextSpecial('=', &location))
                                       /* this is the >>= operator          */
               token = OPERATOR(STRICT_GREATERTHAN_EQUAL);
             else                      /* this is the >> operator           */
               token = OPERATOR(STRICT_GREATERTHAN);
           }
                                       /* next one an equal sign?           */
           else if (this->nextSpecial('=', &location))
                                       /* this is the >= operator           */
             token = OPERATOR(GREATERTHAN_EQUAL);
                                       /* next one a less than sign?        */
           else if (this->nextSpecial('<', &location))
                                       /* this is the <> operator           */
             token = OPERATOR(GREATERTHAN_LESSTHAN);
           else                        /* this simply the > operator        */
             token = OPERATOR(GREATERTHAN);
           break;

         case '\\':                    /* backslash                         */
                                       /* next one an equal sign?           */
           if (this->nextSpecial('=', &location)) {
                                       /* have an equal sign after that?    */
             if (this->nextSpecial('=', &location))
                                       /* this is the \== operator          */
               token = OPERATOR(STRICT_BACKSLASH_EQUAL);
             else                      /* this is the \= operator           */
               token = OPERATOR(BACKSLASH_EQUAL);
           }
                                       /* next one a greater than sign?     */
           else if (this->nextSpecial('>', &location)) {
                                       /* have another greater than next?   */
             if (this->nextSpecial('>', &location))
                                       /* this is the \>> operator          */
               token = OPERATOR(STRICT_BACKSLASH_GREATERTHAN);
             else                      /* this is the \> operator           */
               token = OPERATOR(BACKSLASH_GREATERTHAN);
           }
                                       /* next one a less than sign?        */
           else if (this->nextSpecial('<', &location)) {
                                       /* have another less than next?      */
             if (this->nextSpecial('<', &location))
                                       /* this is the \<< operator          */
               token = OPERATOR(STRICT_BACKSLASH_LESSTHAN);
             else                      /* this is the \< operator           */
               token = OPERATOR(BACKSLASH_LESSTHAN);
           }
           else                        /* this is just the NOT operator     */
             token = OPERATOR(BACKSLASH);
           break;

         case (UCHAR)'ª':              /* logical not                       */
                                       /* next one an equal sign?           */
           if (this->nextSpecial('=', &location)) {
                                       /* have an equal sign after that?    */
             if (this->nextSpecial('=', &location))
                                       /* this is the \== operator          */
               token = OPERATOR(STRICT_BACKSLASH_EQUAL);
             else                      /* this is the \= operator           */
               token = OPERATOR(BACKSLASH_EQUAL);
           }
                                       /* next one a greater than sign?     */
           else if (this->nextSpecial('>', &location)) {
                                       /* have another greater than next?   */
             if (this->nextSpecial('>', &location))
                                       /* this is the \>> operator          */
               token = OPERATOR(STRICT_BACKSLASH_GREATERTHAN);
             else                      /* this is the \> operator           */
               token = OPERATOR(BACKSLASH_GREATERTHAN);
           }
                                       /* next one a less than sign?        */
           else if (this->nextSpecial('<', &location)) {
                                       /* have another less than next?      */
             if (this->nextSpecial('<', &location))
                                       /* this is the \<< operator          */
               token = OPERATOR(STRICT_BACKSLASH_LESSTHAN);
             else                      /* this is the \< operator           */
               token = OPERATOR(BACKSLASH_LESSTHAN);
           }                           /* this is just the BACKSLASH operator     */
           else
             token = OPERATOR(BACKSLASH);
           break;

         default:                      /* something else found              */
                                       /* record current position in clause */
           this->clause->setEnd(this->line_number, this->line_offset);
           sprintf(badchar, "%c", inch);
           sprintf(hexbadchar, "%2.2X", inch);
                                       /* report the error                  */
           report_error2(Error_Invalid_character_char, new_cstring(badchar), new_cstring(hexbadchar));
           break;
       }
      }
    }
    break;                             /* have a token now                  */
  }
  return token;                        /* return the next token             */
}
