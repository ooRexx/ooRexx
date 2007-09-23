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
/* REXX Translator                                    BuiltinFunctions.c      */
/*                                                                            */
/* Builtin Function Execution Stubs                                           */
/*                                                                            */
/******************************************************************************/

#include <stdlib.h>
#include <ctype.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "DirectoryClass.hpp"
#include "ArrayClass.hpp"
#include "RexxVariableDictionary.hpp"
#include "RexxActivation.hpp"
#include "RexxActivity.hpp"
#include "ExpressionBaseVariable.hpp"
#include "SourceFile.hpp"
#include "BuiltinFunctions.hpp"
#include "RexxDateTime.hpp"
#include "Numerics.hpp"


/* checks if pad is a single character string */
void checkPadArgument(char *pFuncName, RexxObject *position, RexxString *pad)
{
  if (pad == OREF_NULL) return;
  if (pad->length != 1)
    report_exception3(Error_Incorrect_call_pad, new_cstring(pFuncName), position, pad);
}

#define CENTER_MIN 2
#define CENTER_MAX 3
#define CENTER_string 1
#define CENTER_length 2
#define CENTER_pad    3

BUILTIN(CENTER) {
  RexxString  *string;                 /* target centered string            */
  RexxInteger *length;                 /* target string length              */
  RexxString  *pad;                    /* optional pad character            */

  fix_args(CENTER);                    /* check on required number of args  */
                                       /* force first argument to a string  */
  string = required_string(CENTER, string);
                                       /* this is a required length         */
  length = required_integer(CENTER, length);
  pad = optional_string(CENTER, pad);  /* the pad character must be one too */
  checkPadArgument(CHAR_CENTER, IntegerThree, pad);
  return string->center(length, pad);  /* do the center function            */
}

#define CENTRE_MIN 2
#define CENTRE_MAX 3
#define CENTRE_string 1
#define CENTRE_length 2
#define CENTRE_pad    3

BUILTIN(CENTRE) {
  RexxString  *string;                 /* target string                     */
  RexxInteger *length;                 /* target string length              */
  RexxString  *pad;                    /* optional pad character            */

  fix_args(CENTRE);                    /* check on required number of args  */
                                       /* force first argument to a string  */
  string = required_string(CENTRE, string);
                                       /* this is a required length         */
  length = required_integer(CENTRE, length);
  pad = optional_string(CENTRE, pad);  /* the pad character must be one too */
  checkPadArgument(CHAR_CENTRE, IntegerThree, pad);
  return string->center(length, pad);  /* do the center function            */
}

#define DELSTR_MIN 2
#define DELSTR_MAX 3
#define DELSTR_string 1
#define DELSTR_n      2
#define DELSTR_length 3

BUILTIN(DELSTR) {
  RexxString  *string;                 /* target string                     */
  RexxInteger *n;                      /* start position                    */
  RexxInteger *length;                 /* target string length              */

  fix_args(DELSTR);                    /* check on required number of args  */
                                       /* must have the first argument      */
  string = required_string(DELSTR, string);
  n = required_integer(DELSTR, n);     /* need a delete position            */
                                       /* length is optional                */
  length = optional_integer(DELSTR, length);
  return string->delstr(n, length);    /* do the delstr function            */
}

#define DELWORD_MIN 2
#define DELWORD_MAX 3
#define DELWORD_string 1
#define DELWORD_n      2
#define DELWORD_length 3

BUILTIN(DELWORD) {
  RexxString  *string;                 /* target string                     */
  RexxInteger *n;                      /* start position                    */
  RexxInteger *length;                 /* target string length              */

  fix_args(DELWORD);                   /* check on required number of args  */
                                       /* must have the first argument      */
  string = required_string(DELWORD, string);
  n = required_integer(DELWORD, n);    /* need a delete position            */
                                       /* length is optional                */
  length = optional_integer(DELWORD, length);
  return string->delWord(n, length);   /* do the delword function           */
}

#define INSERT_MIN 2
#define INSERT_MAX 5
#define INSERT_new    1
#define INSERT_target 2
#define INSERT_n      3
#define INSERT_length 4
#define INSERT_pad    5

BUILTIN(INSERT) {
  RexxString  *newString;              /* new string to insert              */
  RexxString  *target;                 /* target string                     */
  RexxInteger *n;                      /* start position                    */
  RexxInteger *length;                 /* target string length              */
  RexxString  *pad;                    /* optional pad character            */

  fix_args(INSERT);                    /* check on require number of args   */
                                       /* get string for new                */
  newString = required_string(INSERT, new);
                                       /* get string for target             */
  target = required_string(INSERT, target);
  n = optional_integer(INSERT, n);     /* insert position is optional       */
                                       /* length is optional                */
  length = optional_integer(INSERT, length);
  pad = optional_string(INSERT, pad);  /* get string for pad                */
                                       /* go perform the insert function    */
  checkPadArgument(CHAR_INSERT, IntegerFour, pad);
  return target->insert(newString, n, length, pad);
}

#define LEFT_MIN 2
#define LEFT_MAX 3
#define LEFT_string 1
#define LEFT_length 2
#define LEFT_pad    3

BUILTIN(LEFT) {
  RexxString  *string;                 /* target string                     */
  RexxInteger *length;                 /* target string length              */
  RexxString  *pad;                    /* optional pad character            */

  fix_args(LEFT);                      /* check on required number of args  */
                                       /* must have the first argument      */
  string = required_string(LEFT, string);
                                       /* length is optional                */
  length = optional_integer(LEFT, length);
  pad = optional_string(LEFT, pad);    /* pad must be a string also         */
  checkPadArgument(CHAR_LEFT, IntegerThree, pad);
  return string->left(length, pad);    /* do the substr function            */
}

#define OVERLAY_MIN 2
#define OVERLAY_MAX 5
#define OVERLAY_new    1
#define OVERLAY_target 2
#define OVERLAY_n      3
#define OVERLAY_length 4
#define OVERLAY_pad    5

BUILTIN(OVERLAY) {
  RexxString  *newString;              /* new string to overlay             */
  RexxString  *target;                 /* target string                     */
  RexxInteger *n;                      /* start position                    */
  RexxInteger *length;                 /* target string length              */
  RexxString  *pad;                    /* optional pad character            */

  fix_args(OVERLAY);                   /* check on require number of args   */
                                       /* get string for new                */
  newString = required_string(OVERLAY, new);
                                       /* get string for target             */
  target = required_string(OVERLAY, target);
  n = optional_integer(OVERLAY, n);    /* overlay position is optional      */
                                       /* length is optional                */
  length = optional_integer(OVERLAY, length);
  pad = optional_string(OVERLAY, pad); /* get string for pad                */
                                       /* go perform the overlay function   */
  checkPadArgument(CHAR_OVERLAY, IntegerFive, pad);
  return target->overlay(newString, n, length, pad);
}

#define POS_MIN 2
#define POS_MAX 3
#define POS_needle   1
#define POS_haystack 2
#define POS_start    3

BUILTIN(POS) {
  RexxString  *needle;                 /* search needle                     */
  RexxString  *haystack;               /* string to search in               */
  RexxInteger *start;                  /* the start position                */

  fix_args(POS);                       /* check on require number of args   */
                                       /* get string for new                */
  needle = required_string(POS, needle);
                                       /* get string for target             */
  haystack = required_string(POS, haystack);
  start = optional_integer(POS, start);/* start position is optional        */
                                       /* go perform the pos function       */
  return haystack->posRexx(needle, start);
}

#define LASTPOS_MIN 2
#define LASTPOS_MAX 3
#define LASTPOS_needle   1
#define LASTPOS_haystack 2
#define LASTPOS_start    3

BUILTIN(LASTPOS) {
  RexxString  *needle;                 /* search needle                     */
  RexxString  *haystack;               /* string to search in               */
  RexxInteger *start;                  /* the start position                */

  fix_args(LASTPOS);                   /* check on require number of args   */
                                       /* get string for new                */
  needle = required_string(LASTPOS, needle);
                                       /* get string for target             */
  haystack = required_string(LASTPOS, haystack);
                                       /* start position is optional        */
  start = optional_integer(LASTPOS, start);
                                       /* go perform the lastpos function   */
  return haystack->lastPosRexx(needle, start);
}

#define REVERSE_MIN 1
#define REVERSE_MAX 1
#define REVERSE_string 1

BUILTIN(REVERSE) {
  RexxString  *string;                 /* target string                     */

  fix_args(REVERSE);                   /* check on require number of args   */
                                       /* get string for string             */
  string = required_string(REVERSE, string);
  return string->reverse();            /* go perform the reverse function   */
}

#define RIGHT_MIN 2
#define RIGHT_MAX 3
#define RIGHT_string 1
#define RIGHT_length 2
#define RIGHT_pad    3

BUILTIN(RIGHT) {
  RexxString  *string;                 /* target string                     */
  RexxInteger *length;                 /* target string length              */
  RexxString  *pad;                    /* optional pad character            */

  fix_args(RIGHT);                     /* check on required number of args  */
                                       /* must have the first argument      */
  string = required_string(RIGHT, string);
                                       /* length is optional                */
  length = optional_integer(RIGHT, length);
  pad = optional_string(RIGHT, pad);   /* pad must be a string also         */
  checkPadArgument(CHAR_RIGHT, IntegerThree, pad);
  return string->right(length, pad);   /* do the substr function            */
}

#define STRIP_MIN 1
#define STRIP_MAX 3
#define STRIP_string 1
#define STRIP_option 2
#define STRIP_char   3

BUILTIN(STRIP) {
  RexxString  *string;                 /* target string                     */
  RexxString  *option;                 /* function option                   */
  RexxString  *character;              /* character to strip                */

  fix_args(STRIP);                     /* check on required number of args  */
                                       /* must have the first argument      */
  string = required_string(STRIP, string);
                                       /* option must be a string too       */
  option = optional_string(STRIP, option);
                                       /* as is char as well                */
  character = optional_string(STRIP, char);
                                       /* do the strip function             */
  return string->strip(option, character);
}

#define SPACE_MIN 1
#define SPACE_MAX 3
#define SPACE_string 1
#define SPACE_n      2
#define SPACE_pad    3


BUILTIN(SPACE) {
  RexxString  *string;                 /* target string                     */
  RexxInteger *n;                      /* inter-word spaces                 */
  RexxString  *pad;                    /* optional pad character            */

  fix_args(SPACE);                     /* check on required number of args  */
                                       /* must have the first argument      */
  string = required_string(SPACE, string);
  n = optional_integer(SPACE, n);      /* spacing is an optional integer    */
  pad = optional_string(SPACE, pad);   /* pad must be a string also         */
  checkPadArgument(CHAR_SPACE, IntegerThree, pad);
  return string->space(n, pad);        /* do the space function             */
}

#define SUBSTR_MIN 2
#define SUBSTR_MAX 4
#define SUBSTR_string 1
#define SUBSTR_n      2
#define SUBSTR_length 3
#define SUBSTR_pad    4


BUILTIN(SUBSTR) {
  RexxString  *string;                 /* target string                     */
  RexxInteger *n;                      /* start position                    */
  RexxInteger *length;                 /* target string length              */
  RexxString  *pad;                    /* optional pad character            */

  fix_args(SUBSTR);                    /* check on required number of args  */
                                       /* must have the first argument      */
  string = required_string(SUBSTR, string);
  n = required_integer(SUBSTR, n);     /* position is required              */
                                       /* length is optional                */
  length = optional_integer(SUBSTR, length);
  pad = optional_string(SUBSTR, pad);  /* pad must be a string also         */
                                       /* do the substr function            */
  checkPadArgument(CHAR_SUBSTR, IntegerFour, pad);
  return string->substr(n, length, pad);
}


#define LOWER_MIN 1
#define LOWER_MAX 3
#define LOWER_string 1
#define LOWER_n      2
#define LOWER_length 3


BUILTIN(LOWER) {
  RexxString  *string;                 /* target string                     */
  RexxInteger *n;                      /* start position                    */
  RexxInteger *length;                 /* target string length              */

  fix_args(LOWER);                     /* check on required number of args  */
                                       /* must have the first argument      */
  string = required_string(LOWER, string);
  n = optional_integer(LOWER, n);      /* position is optional              */
                                       /* length is optional                */
  length = optional_integer(LOWER, length);
                                       /* do the LOWER function            */
  return string->lowerRexx(n, length);
}


#define UPPER_MIN 1
#define UPPER_MAX 3
#define UPPER_string 1
#define UPPER_n      2
#define UPPER_length 3


BUILTIN(UPPER) {
  RexxString  *string;                 /* target string                     */
  RexxInteger *n;                      /* start position                    */
  RexxInteger *length;                 /* target string length              */

  fix_args(UPPER);                     /* check on required number of args  */
                                       /* must have the first argument      */
  string = required_string(UPPER, string);
  n = optional_integer(UPPER, n);       /* position is optional              */
                                       /* length is optional                */
  length = optional_integer(UPPER, length);
                                       /* do the UPPER function            */
  return string->upperRexx(n, length);
}


#define SUBWORD_MIN 2
#define SUBWORD_MAX 3
#define SUBWORD_string 1
#define SUBWORD_n      2
#define SUBWORD_length 3

BUILTIN(SUBWORD) {
  RexxString  *string;                 /* target string                     */
  RexxInteger *n;                      /* required start                    */
  RexxInteger *length;                 /* target string length              */

  fix_args(SUBWORD);                   /* check on required number of args  */
                                       /* must have the first argument      */
  string = required_string(SUBWORD, string);
  n = required_integer(SUBWORD, n);    /* position is required              */
                                       /* length is optional                */
  length = optional_integer(SUBWORD, length);
  return string->subWord(n, length);   /* do the subword function           */
}

#define WORD_MIN 2
#define WORD_MAX 2
#define WORD_string 1
#define WORD_n      2

BUILTIN(WORD) {
  RexxString  *string;                 /* target string                     */
  RexxInteger *n;                      /* start position                    */

  fix_args(WORD);                      /* check on required number of args  */
                                       /* must have the first argument      */
  string = required_string(WORD, string);
  n = required_integer(WORD, n);       /* position is required              */
  return string->word(n);              /* do the word function              */
}

#define WORDINDEX_MIN 2
#define WORDINDEX_MAX 2
#define WORDINDEX_string 1
#define WORDINDEX_n      2

BUILTIN(WORDINDEX) {
  RexxString  *string;                 /* target string                     */
  RexxInteger *n;                      /* word number                       */

  fix_args(WORDINDEX);                 /* check on required number of args  */
                                       /* must have the first argument      */
  string = required_string(WORDINDEX, string);
  n = required_integer(WORDINDEX, n);  /* position is required              */
  return string->wordIndex(n);         /* do the wordindex function         */
}

#define WORDLENGTH_MIN 2
#define WORDLENGTH_MAX 2
#define WORDLENGTH_string 1
#define WORDLENGTH_n      2

BUILTIN(WORDLENGTH) {
  RexxString  *string;                 /* target string                     */
  RexxInteger *n;                      /* start position                    */

  fix_args(WORDLENGTH);                /* check on required number of args  */
                                       /* must have the first argument      */
  string = required_string(WORDLENGTH, string);
  n = required_integer(WORDLENGTH, n); /* position is required              */
  return string->wordLength(n);        /* do the wordlength function        */
}

#define COPIES_MIN 2
#define COPIES_MAX 2
#define COPIES_string 1
#define COPIES_n      2

BUILTIN(COPIES) {
  RexxString  *string;                 /* target string                     */
  RexxInteger *n;                      /* number of copies                  */

  fix_args(COPIES);                    /* check on required number of args  */
                                       /* must have the first argument      */
  string = required_string(COPIES, string);
  n = required_integer(COPIES, n);     /* position is required              */
  return string->copies(n);            /* do the copies function            */
}

#define WORDPOS_MIN 2
#define WORDPOS_MAX 3
#define WORDPOS_phrase 1
#define WORDPOS_string 2
#define WORDPOS_start  3

BUILTIN(WORDPOS) {
  RexxString  *string;                 /* target string                     */
  RexxString  *phrase;                 /* search phrase                     */
  RexxInteger *start;                  /* start position                    */

  fix_args(WORDPOS);                   /* check on required number of args  */
                                       /* must have a phrase string         */
  phrase = required_string(WORDPOS, phrase);
                                       /* must have the string argument     */
  string = required_string(WORDPOS, string);
                                       /* start position is optional        */
  start = optional_integer(WORDPOS, start);
                                       /* do the wordpos function           */
  return string->wordPos(phrase, start);
}

#define WORDS_MIN 1
#define WORDS_MAX 1
#define WORDS_string 1

BUILTIN(WORDS) {
  RexxString  *string;                 /* target string                     */

  fix_args(WORDS);                     /* check on required number of args  */
                                       /* must have the string argument     */
  string = required_string(WORDS, string);
  return string->words();              /* do the words function             */
}

#define ABBREV_MIN 2
#define ABBREV_MAX 3
#define ABBREV_information 1
#define ABBREV_info        2
#define ABBREV_length      3

BUILTIN(ABBREV) {
  RexxString  *information;            /* information to check              */
  RexxString  *info;                   /* target abbreviation               */
  RexxInteger *length;                 /* target string length              */

  fix_args(ABBREV);                    /* check on required number of args  */
                                       /* information must be a string arg  */
  information = required_string(ABBREV, information);
  info = required_string(ABBREV, info);/* info must also be a string        */
                                       /* length is optional                */
  length = optional_integer(ABBREV, length);
                                       /* check on the abbreviation         */
  return information->abbrev(info, length);
}

#define BITAND_MIN 1
#define BITAND_MAX 3
#define BITAND_string1 1
#define BITAND_string2 2
#define BITAND_pad     3

BUILTIN(BITAND) {
  RexxString  *string1;                /* first string                      */
  RexxString  *string2;                /* second string                     */
  RexxString  *pad;                    /* optional pad character            */

  fix_args(BITAND);                    /* check on required number of args  */
                                       /* must have the first string        */
  string1 = required_string(BITAND, string1);
                                       /* second string is optional         */
  string2 = optional_string(BITAND, string2);
  pad = optional_string(BITAND, pad);  /* pad is optional also              */
  checkPadArgument(CHAR_BITAND, IntegerThree, pad);
  return string1->bitAnd(string2, pad);/* do the bitand function            */
}

#define BITOR_MIN 1
#define BITOR_MAX 3
#define BITOR_string1 1
#define BITOR_string2 2
#define BITOR_pad     3

BUILTIN(BITOR) {
  RexxString  *string1;                /* first string                      */
  RexxString  *string2;                /* second string                     */
  RexxString  *pad;                    /* optional pad character            */

  fix_args(BITOR);                     /* check on required number of args  */
                                       /* must have the first string        */
  string1 = required_string(BITOR, string1);
                                       /* second string is optional         */
  string2 = optional_string(BITOR, string2);
  pad = optional_string(BITOR, pad);   /* pad is optional also              */
  checkPadArgument(CHAR_BITOR, IntegerThree, pad);
  return string1->bitOr(string2, pad); /* do the bitor function             */
}

#define BITXOR_MIN 1
#define BITXOR_MAX 3
#define BITXOR_string1 1
#define BITXOR_string2 2
#define BITXOR_pad     3

BUILTIN(BITXOR) {
  RexxString  *string1;                /* first string                      */
  RexxString  *string2;                /* second string                     */
  RexxString  *pad;                    /* optional pad character            */

  fix_args(BITXOR);                    /* check on required number of args  */
                                       /* must have the first string        */
  string1 = required_string(BITXOR, string1);
                                       /* second string is optional         */
  string2 = optional_string(BITXOR, string2);
  pad = optional_string(BITXOR, pad);  /* pad is optional also              */
  checkPadArgument(CHAR_BITXOR, IntegerThree, pad);
  return string1->bitXor(string2, pad);/* do the bitxor function            */
}

#define B2X_MIN 1
#define B2X_MAX 1
#define B2X_string  1

BUILTIN(B2X) {
  RexxString  *string;                 /* target string                     */

  fix_args(B2X);                       /* check on required number of args  */
                                       /* must have the first string        */
  string = required_string(B2X, string);
  return string->b2x();                /* do the b2x function               */
}

#define X2B_MIN 1
#define X2B_MAX 1
#define X2B_string  1

BUILTIN(X2B) {
  RexxString  *string;                 /* target string                     */

  fix_args(X2B);                       /* check on required number of args  */
                                       /* must have the first string        */
  string = required_string(X2B, string);
  return string->x2b();                /* do the x2b function               */
}

#define C2X_MIN 1
#define C2X_MAX 1
#define C2X_string  1

BUILTIN(C2X) {
  RexxString  *string;                 /* target string                     */

  fix_args(C2X);                       /* check on required number of args  */
                                       /* must have the first string        */
  string = required_string(C2X, string);
  return string->c2x();                /* do the c2x function               */
}

#define X2C_MIN 1
#define X2C_MAX 1
#define X2C_string  1

BUILTIN(X2C) {
  RexxString  *string;                 /* target string                     */

  fix_args(X2C);                       /* check on required number of args  */
                                       /* must have the first string        */
  string = required_string(X2C, string);
  return string->x2c();                /* do the x2c function               */
}

#define C2D_MIN 1
#define C2D_MAX 2
#define C2D_string  1
#define C2D_n       2

BUILTIN(C2D) {
  RexxString  *string;                 /* target string                     */
  RexxInteger *n;                      /* start position                    */

  fix_args(C2D);                       /* check on required number of args  */
                                       /* must have the first string        */
  string = required_string(C2D, string);
  n = optional_integer(C2D, n);        /* length is optional                */
  return string->c2d(n);               /* do the c2d function               */
}

#define TRUNC_MIN 1
#define TRUNC_MAX 2
#define TRUNC_number  1
#define TRUNC_n       2

BUILTIN(TRUNC) {
  RexxString  *number;                 /* number to truncate                */
  RexxInteger *n;                      /* digits to use                     */

  fix_args(TRUNC);                     /* check on required number of args  */
                                       /* must have the first string        */
  number = required_string(TRUNC, number);
  n = optional_integer(TRUNC, n);      /* length is optional                */
  return number->trunc(n);             /* do the trunc function             */
}

#define X2D_MIN 1
#define X2D_MAX 2
#define X2D_string  1
#define X2D_n       2

BUILTIN(X2D) {
  RexxString  *string;                 /* target string                     */
  RexxInteger *n;                      /* start position                    */

  fix_args(X2D);                       /* check on required number of args  */
                                       /* must have the first string        */
  string = required_string(X2D, string);
  n = optional_integer(X2D, n);        /* length is optional                */
  return string->x2d(n);               /* do the x2d function               */
}

#define D2X_MIN 1
#define D2X_MAX 2
#define D2X_string  1
#define D2X_n       2

BUILTIN(D2X) {
  RexxString  *string;                 /* target string                     */
  RexxInteger *n;                      /* conversion length                 */

  fix_args(D2X);                       /* check on required number of args  */
                                       /* must have the first string        */
  string = required_string(D2X, string);
  n = optional_integer(D2X, n);        /* length is optional                */
  return string->d2x(n);               /* do the x2d function               */
}

#define D2C_MIN 1
#define D2C_MAX 2
#define D2C_string  1
#define D2C_n       2

BUILTIN(D2C) {
  RexxString  *string;                 /* target string                     */
  RexxInteger *n;                      /* start position                    */

  fix_args(D2C);                       /* check on required number of args  */
                                       /* must have the first string        */
  string = required_string(D2C, string);
  n = optional_integer(D2C, n);        /* length is optional                */
  return string->d2c(n);               /* do the x2d function               */
}

#define COMPARE_MIN 2
#define COMPARE_MAX 3
#define COMPARE_string1 1
#define COMPARE_string2 2
#define COMPARE_pad     3

BUILTIN(COMPARE) {
  RexxString  *string1;                /* first comparison string           */
  RexxString  *string2;                /* second comparison string          */
  RexxString  *pad;                    /* optional pad character            */

  fix_args(COMPARE);                   /* check on required number of args  */
                                       /* must have the first string        */
  string1 = required_string(COMPARE, string1);
                                       /* and the second string also        */
  string2 = required_string(COMPARE, string2);
  pad = optional_string(COMPARE, pad); /* padding is optional               */
                                       /* do the comparison                 */
  checkPadArgument(CHAR_COMPARE, IntegerThree, pad);
  return string1->compare(string2, pad);
}

#define LENGTH_MIN 1
#define LENGTH_MAX 1
#define LENGTH_string  1

BUILTIN(LENGTH) {
  RexxString  *target;                 /* target of the operation           */

  fix_args(LENGTH);                    /* check on required number of args  */
                                       /* must have a string                */
  target = required_string(LENGTH, string);
  return target->lengthRexx();         /* get the length                    */
}

#define TRANSLATE_MIN 1
#define TRANSLATE_MAX 4
#define TRANSLATE_string  1
#define TRANSLATE_tableo  2
#define TRANSLATE_tablei  3
#define TRANSLATE_pad     4

BUILTIN(TRANSLATE) {
  RexxString  *string;                 /* target string                     */
  RexxString  *tableo;                 /* output table                      */
  RexxString  *tablei;                 /* input table                       */
  RexxString  *pad;                    /* optional pad character            */

  fix_args(TRANSLATE);                 /* check on required number of args  */
                                       /* must have a string                */
  string = required_string(TRANSLATE, string);
                                       /* output table is optional          */
  tableo = optional_string(TRANSLATE, tableo);
                                       /* input table is optional           */
  tablei = optional_string(TRANSLATE, tablei);
                                       /* pad is also optional              */
  pad = optional_string(TRANSLATE, pad);
                                       /* perform the translate             */
  checkPadArgument(CHAR_TRANSLATE, IntegerFour, pad);
  return string->translate(tableo, tablei, pad);
}

#define VERIFY_MIN 2
#define VERIFY_MAX 4
#define VERIFY_string    1
#define VERIFY_reference 2
#define VERIFY_option    3
#define VERIFY_start     4

BUILTIN(VERIFY) {
  RexxString  *string;                 /* target string                     */
  RexxString  *reference;              /* reference characters              */
  RexxString  *option;                 /* function option                   */
  RexxInteger *start;                  /* start position                    */

  fix_args(VERIFY);                    /* check on required number of args  */
                                       /* must have a string                */
  string = required_string(VERIFY, string);
                                       /* reference is also required        */
  reference = required_string(VERIFY, reference);
                                       /* the options are optional          */
  option = optional_string(VERIFY, option);
                                       /* start is optional                 */
  start = optional_integer(VERIFY, start);
                                       /* do the verify function            */
  return string->verify(reference, option, start);
}

#define DATATYPE_MIN 1
#define DATATYPE_MAX 2
#define DATATYPE_string    1
#define DATATYPE_type      2

BUILTIN(DATATYPE) {
  RexxString  *string;                 /* target string                     */
  RexxString  *type;                   /* type to check against             */

  fix_args(DATATYPE);                  /* check on required number of args  */
                                       /* must have a string                */
  string = required_string(DATATYPE, string);
                                       /* type must also be a string        */
  type = optional_string(DATATYPE, type);
  return string->dataType(type);       /* call the datatype method          */
}

#define ADDRESS_MIN 0
#define ADDRESS_MAX 0

BUILTIN(ADDRESS) {
  check_args(ADDRESS);                 /* check on required number of args  */
  return context->getAddress();        /* return the current address setting*/
}

#define DIGITS_MIN 0
#define DIGITS_MAX 0

BUILTIN(DIGITS) {
  LONG  tempDigits;

  check_args(DIGITS);                  /* check on required number of args  */
  tempDigits = context->digits();      /* get the number of digits          */
  return new_integer(tempDigits);      /* return as an option               */
}

#define FUZZ_MIN 0
#define FUZZ_MAX 0

BUILTIN(FUZZ) {
  LONG  tempFuzz;

  check_args(FUZZ);                    /* check on required number of args  */
  tempFuzz = context->fuzz();          /* get the fuzz value                */
  return new_integer(tempFuzz);        /* return as an integer object       */
}

#define FORM_MIN 0
#define FORM_MAX 0

BUILTIN(FORM) {
  check_args(FORM);                    /* check on required number of args  */
                                       /* return the current form setting   */
  return context->form() == FORM_SCIENTIFIC ? OREF_SCIENTIFIC : OREF_ENGINEERING;
}

#define USERID_MIN 0
#define USERID_MAX 0

BUILTIN(USERID) {
  check_args(USERID);
  return SysUserid();
}

#define ERRORTEXT_MIN 1
#define ERRORTEXT_MAX 1
#define ERRORTEXT_n   1

BUILTIN(ERRORTEXT) {
  INT    error_number;                 /* requested error number            */
  RexxString *result;                  /* function result                   */

  check_args(ERRORTEXT);               /* check on required number of args  */
                                       /* get the error number              */
  error_number = (required_integer(ERRORTEXT, n))->value;
                                       /* outside allowed range?            */
  if (error_number < 0 || error_number > 99)
                                       /* this is an error                  */
    report_exception3(Error_Incorrect_call_range, new_cstring(CHAR_ERRORTEXT), IntegerOne, new_integer(error_number));
                                       /* retrieve the major error message  */
  result = (RexxString *)SysMessageText(error_number * 1000);
  if (result == OREF_NULL)             /* not found?                        */
    result = OREF_NULLSTRING;          /* this is a null string result      */
  return result;                       /* finished                          */
}

#define ARG_MIN 0
#undef ARG_MAX                      /* In AIX already defined            */
#define ARG_MAX 2
#define ARG_n      1
#define ARG_option 2

BUILTIN(ARG) {
  RexxString  *option;                 /* function option                   */
  RexxInteger *n;                      /* arg position count                */
  RexxObject  *result;                 /* function result                   */
  RexxObject **arglist;                /* activation argument list          */
  size_t position;                     /* position argument                 */
  size_t size;                         /* array size                        */

  fix_args(ARG);                       /* expand arguments to full value    */
  n = optional_integer(ARG, n);        /* get the position info             */
                                       /* get the option string             */
  option = optional_string(ARG, option);
  /* get the argument array            */
  arglist = context->getMethodArgumentList();
  size = context->getMethodArgumentCount();
                                       /* have an option but no position?   */
  if (n == OREF_NULL) {                /* no position specified?            */
    if (option != OREF_NULL)           /* have an option with no position   */
                                       /* raise an error                    */
      report_exception2(Error_Incorrect_call_noarg, new_cstring(CHAR_ARG), IntegerOne);
    /* return the count as an object */
    result = new_integer(size);
  }
  else if (option == OREF_NULL) {      /* just looking for a specific arg?  */
    position = n->value;               /* get the integer value             */
                                       /* must be a positive integer        */
    positive_integer(position, ARG, IntegerOne);
                                       /* bigger than argument list size?   */
    if (size < position)
      result = OREF_NULLSTRING;        /* just return a null string         */
    else {
      result = arglist[position - 1];  /* get actual value from arglist     */
      if (result == OREF_NULL)         /* argument wasn't there?            */
        result = OREF_NULLSTRING;      /* this too is a null string         */
    }
  }
                                       /* have a null string?               */
  else if (option->length == 0)
                                       /* this is an error                  */
    report_exception4(Error_Incorrect_call_list, new_cstring(CHAR_ARG), IntegerTwo, new_string("AENO", 4), option);
  else {                               /* need to process an option         */
    position = n->value;               /* get the integer value             */
                                       /* must be a positive integer        */
    positive_integer(position, ARG, IntegerOne);

    switch (option->getChar(0)) {      /* process the option character      */

      case 'A':                        /* return argument array             */
      case 'a':                        /* return argument array             */
        if (position == 1) {           /* want it all?                      */
            /* create an array result for the return */
            result = new (size, arglist) RexxArray;
        }
        else if (position > size)      /* beyond bounds of argument list?   */
                                       /* this is a zero size array         */
            result = TheNullArray->copy();
        else {                         /* need to extract a sub array       */
            result = new (size - position + 1, &arglist[position - 1]) RexxArray;
        }
        break;

      case 'E':                        /* argument 'E'xist?                 */
      case 'e':                        /* argument 'E'xist?                 */
        if (position > size)           /* too big for argument list?        */
          result = TheFalseObject;     /* can't be true                     */
                                       /* have a real argument?             */
        else if (arglist[position - 1] == OREF_NULL)
          result = TheFalseObject;     /* nope, this is false also          */
        else
          result = TheTrueObject;      /* have a real argument              */
        break;

      case 'O':                        /* argument 'O'mitted?               */
      case 'o':                        /* argument 'O'mitted?               */
        if (position > size)           /* too big for argument list?        */
          result = TheTrueObject;      /* must be omitted                   */
                                       /* have a real argument?             */
        else if (arglist[position - 1] == OREF_NULL)
          result = TheTrueObject;      /* this is omitted also              */
        else
          result = TheFalseObject;     /* have a real argument              */
        break;

      case 'N':                        /* 'N'ormal processing?              */
      case 'n':                        /* 'N'ormal processing?              */
        if (position > size)           /* bigger than argument list size?   */
          result = OREF_NULLSTRING;    /* just return a null string         */
        else {                         /* get actual value from arglist     */
          result = arglist[position - 1];
          if (result == OREF_NULL)     /* argument wasn't there?            */
            result = OREF_NULLSTRING;  /* this too is a null string         */
        }
        break;

      default:                         /* unknown option                    */
                                       /* this is an error                  */
        report_exception4(Error_Incorrect_call_list, new_cstring(CHAR_ARG), IntegerTwo, new_string("AENO", 4), option);
        break;
    }
  }
  return result;                       /* all finished                      */
}


#define DATE_MIN 0
#define DATE_MAX 5
#define DATE_option  1
#define DATE_indate  2
#define DATE_option2 3
#define DATE_osep    4
#define DATE_isep    5

BUILTIN(DATE) {
    char  work[30];                      /* temporary work                    */

    fix_args(DATE);                      /* expand arguments to full value    */

    // get the arguments
    RexxString *option = optional_string(DATE, option);
    RexxString *indate = optional_string(DATE, indate);
    RexxString *option2 = optional_string(DATE, option2);
    RexxString *osep = optional_string(DATE, osep);
    RexxString *isep = optional_string(DATE, isep);

    // get a copy of the current activation time.  This will ensure
    // a consistent timestamp across calls
    RexxDateTime current = context->getTime();
    // by default, we operator off of the current time.  We may end up
    // overwriting this
    RexxDateTime timestamp = current;

    // default for both input and output styles is 'N'ormal
    int style = 'N';
    int style2 = 'N';

    // now process the various option specifiers
    if (option != OREF_NULL)             /* just using default format?        */
    {
        if (option->length == 0)        /* have a null string?               */
        {
                                             /* this is an error                  */
            report_exception4(Error_Incorrect_call_list, new_cstring(CHAR_DATE), IntegerOne, new_string("BDEFLMNOSTUW", 10), option);
        }
        else                                 /* need to process an option         */
        {
                                             /* option is first character         */
            style = toupper(option->getChar(0));
        }
    }

    /* opt2 or isep w/o date?            */
    if (indate == OREF_NULL && (option2 != OREF_NULL || isep != OREF_NULL))
    {
        /* this is an error                  */
        report_exception2(Error_Incorrect_call_noarg, new_cstring(CHAR_DATE), IntegerTwo);
    }

    if (option2 != OREF_NULL)            /* just using default format?        */
    {
        if (option2->length == 0)            /* have a null string?               */
        {
                                             /* this is an error                  */
            report_exception4(Error_Incorrect_call_list, new_cstring(CHAR_DATE), IntegerThree, new_string("BDEFNOSTU", 7), option2);
        }
        else                                 /* need to process an option         */
        {
                                             /* option is first character         */
            style2 = toupper(option2->getChar(0));
        }
    }

    char *outputSeparator = NULL;            // each format has it's own default

    // validate the output separator is only used with supported styles
    if (osep != OREF_NULL)
    {
        // only certain styles support this option
        if (strchr("BDMWL", style) != NULL)
        {
            report_exception4(Error_Incorrect_call_format_incomp_sep, new_cstring(CHAR_DATE), IntegerOne, new_string((PCHAR)&style, 1), IntegerFour);
        }
        if (osep->length > 1 || (osep->length == 1 && strchr(ALPHANUM, osep->getChar(0)) != NULL))
        {
            report_exception3(Error_Incorrect_call_parm_wrong_sep, new_cstring(CHAR_DATE), IntegerFour, new_string(osep->stringData, osep->length));
        }
        // string objects are null terminated, so we can point directly at what will
        // be either 1 or 0 characters of data.
        outputSeparator = osep->getStringData();
    }

    if (indate != OREF_NULL)             /* given a time stamp?               */
    {
        bool valid = true;                 /* assume have a good stamp          */

        char *separator = NULL;            // different formats will override this
                                           /* begin addition                    */
        // if we have a separator, perform validation here
        if (isep != OREF_NULL)
        {
            if (strchr("BDMWL", style2) != NULL)
            {
                report_exception4(Error_Incorrect_call_format_incomp_sep, new_cstring(CHAR_DATE), IntegerThree, new_string((PCHAR)&style2, 1), IntegerFive);
            }
            // explicitly specified delimiter, we need to validate this first
            if (isep->length > 1 || (isep->length == 1 && strchr(ALPHANUM, isep->getChar(0)) != NULL))
            {
                // the field delimiter must be a single character and NOT
                // alphanumeric, or a null character
                report_exception3(Error_Incorrect_call_parm_wrong_sep, new_cstring(CHAR_DATE), IntegerFive, new_string(isep->stringData, isep->length));
            }
            // string objects are null terminated, so we can point directly at what will
            // be either 1 or 0 characters of data.
            separator = isep->getStringData();
        }

        /* clear the time stamp              */
        timestamp.clear();
        switch (style2)
        {                  /* convert to usable form per option2*/

            case 'N':                        /* 'N'ormal: default style           */
                valid = timestamp.parseNormalDate(indate->getStringData(), separator);
                break;

            case 'B':                        /* 'B'asedate                        */
            {
                /*convert the value                  */
                int basedays = indate->longValue(9);
                /* bad value?                        */
                if (basedays == NO_LONG || !timestamp.setBaseDate(basedays))
                {
                    report_exception3(Error_Incorrect_call_format_invalid, new_cstring(CHAR_DATE), indate, new_string((PCHAR)&style2, 1));
                }
                break;
            }

            case 'F':                        /* 'F'ull datetime stamp            */
            {
                /*convert the value                  */
                int64_t basetime;
                if (!Numerics::objectToInt64(indate, basetime) || !timestamp.setBaseTime(basetime))
                {
                    report_exception3(Error_Incorrect_call_format_invalid, new_cstring(CHAR_DATE), indate, new_string((PCHAR)&style2, 1));
                }
                break;
            }

            case 'T':                        /* 'T'icks datetime stamp            */
            {
                /*convert the value                  */
                int64_t basetime;
                if (!Numerics::objectToInt64(indate, basetime) || !timestamp.setUnixTime(basetime))
                {
                    report_exception3(Error_Incorrect_call_format_invalid, new_cstring(CHAR_DATE), indate, new_string((PCHAR)&style2, 1));
                }
                break;
            }

            case 'D':                        /* 'D'ay of year                     */
            {
                /*convert the value                  */
                int yearday = indate->longValue(9);
                /* bad value?                        */
                if (yearday == NO_LONG || yearday < 0 || yearday > YEAR_DAYS + 1 ||
                    (yearday > YEAR_DAYS && !LeapYear(current.year)))
                {
                    report_exception3(Error_Incorrect_call_format_invalid, new_cstring(CHAR_DATE), indate, new_string((PCHAR)&style2, 1));
                }
                // set the date directly
                timestamp.setDate(current.year, yearday);
                break;
            }

            case 'E':                        /* 'E'uropean format: days-month-year*/
                valid = timestamp.parseEuropeanDate(indate->getStringData(), separator, current.year);
                break;

            case 'O':                        /* 'O'rdered format: year-month-day  */
                valid = timestamp.parseOrderedDate(indate->getStringData(), separator, current.year);
                break;

            case 'S':                        /* 'S'tandard format (ISO date)      */
                valid = timestamp.parseStandardDate(indate->getStringData(), separator);
                break;

            case 'U':                        /* 'U'SA format: month-day-year      */
                valid = timestamp.parseUsaDate(indate->getStringData(), separator, current.year);
                break;

            default:
                report_exception4(Error_Incorrect_call_list, new_cstring(CHAR_DATE), IntegerThree, new_string("BDEFNOTSU", 7), new_string((PCHAR)&style2, 1));
                break;
        }
        // if there's a formatting error
        if (!valid)
        {
            // different error message depending on whether a separator was specified, or not.
            if (isep != OREF_NULL)
            {
                report_exception4(Error_Incorrect_call_format_incomp_sep, new_cstring(CHAR_DATE), IntegerTwo, indate, IntegerFive);
            }
            else
            {
                report_exception3(Error_Incorrect_call_format_invalid, new_cstring(CHAR_DATE), indate, new_string((PCHAR)&style2, 1));
            }
        }
    }
    else
    {
        // just copy the current time stamp
        timestamp = current;
    }

    int day = timestamp.day;          /* get various date parts            */
    int month = timestamp.month;
    int year = timestamp.year;
    int separator = ' ';              // each format with a separator has it's own default

    switch (style)
    {                     /* process the various styles        */

        case 'B':                          /* 'B'asedate                        */
            timestamp.formatBaseDate(work);
            break;

        case 'F':                          /* 'F'asedate                        */
            timestamp.formatBaseTime(work);
            break;

        case 'T':                          /* 'F'asedate                        */
            timestamp.formatUnixTime(work);
            break;

        case 'D':                          /* 'D'ays                            */
            timestamp.formatDays(work);
            break;

        case 'E':                          /* 'E'uropean                        */
            timestamp.formatEuropeanDate(work, outputSeparator);
            break;

        case 'L':                          /* 'L'ocal                           */
        {
            /* get the month name                */
            RexxString *month_name = (RexxString *)SysMessageText(Message_Translations_January + month - 1);
            /* format as a date                  */
            sprintf(work, "%u %s %4.4u", day, month_name->getStringData(), year);
            break;

        }

        case 'M':                          /* 'M'onth                           */
            timestamp.formatMonthName(work);
            break;

        case 'N':                          /* 'N'ormal -- default format        */
            timestamp.formatNormalDate(work, outputSeparator);
            break;

        case 'O':                          /* 'O'rdered                         */
            timestamp.formatOrderedDate(work, outputSeparator);
            break;

        case 'S':                          /* 'S'tandard format (ISO dates)     */
            timestamp.formatStandardDate(work, outputSeparator);
            break;

        case 'U':                          /* 'U'SA                             */
            timestamp.formatUsaDate(work, outputSeparator);
            break;

        case 'W':                          /* 'W'eekday                         */
            timestamp.formatWeekDay(work);
            break;

        default:                           /* unrecognized                      */
            work[0] = style;                 /* copy over the character           */
            report_exception4(Error_Incorrect_call_list, new_cstring(CHAR_DATE), IntegerOne, new_string("BDEFLMNOSTUW", 10), new_string(work, 1));
            break;
    }
    /* now create a string object        */
    return new_cstring(work);
}


#define TIME_MIN 0
#define TIME_MAX 3
#define TIME_option  1
#define TIME_intime  2
#define TIME_option2 3

BUILTIN(TIME) {
    char  work[30];                      /* temporary work                    */

    fix_args(TIME);                      /* expand arguments to full value    */
                                         /* get the option string             */
    RexxString *option = optional_string(TIME, option);
    /* the input date                    */
    RexxString *intime = optional_string(TIME, intime);
    /* input date format                 */
    RexxString *option2 = optional_string(TIME, option2);
    RexxDateTime current = context->getTime();        /* get the current activation time   */
    RexxDateTime timestamp = current;                 // and by default we work off of that time
    int style = 'N';                     // get the default style

    // do we have a style option specified?  Validate, and retrieve
    if (option != OREF_NULL)
    {
        // null strings not allowed as an option character
        if (option->length == 0)
        {
            report_exception4(Error_Incorrect_call_list, new_cstring(CHAR_TIME), IntegerOne, new_string("CEHLMNRS", 8), option);
        }
        // we only use the first character
        style = toupper(option->getChar(0));
    }

    // now repeat with the second style
    int style2 = 'N';

    // has the input style been specified?
    if (option2 != OREF_NULL)
    {
        // the second option requires an input date
        if (intime == OREF_NULL)
        {
            report_exception2(Error_Incorrect_call_noarg, new_cstring(CHAR_TIME), IntegerTwo);
        }
        // again, must be at least one character, of which we only use the first
        if (option2->length == 0)
        {
            report_exception4(Error_Incorrect_call_list, new_cstring(CHAR_TIME), IntegerThree, new_string("CHLMNS", 6), option2);
        }
        style2 = toupper(option2->getChar(0));
    }


    if (intime != OREF_NULL)
    {
        // the input timestamp is not valid with the elapsed time options
        if (style == 'R' || style == 'E')
        {
            report_exception2(Error_Incorrect_call_invalid_conversion, new_cstring(CHAR_TIME), new_string((PCHAR)&style, 1));
        }
        bool valid = true;                 // assume this is a good timestamp
        timestamp.clear();                 // clear everything out

        switch (style2)
        {
            // default style, 01:23:45 format (24 hour)
            case 'N':
                valid = timestamp.parseNormalTime(intime->getStringData());
                break;

            // 'C'ivil time, 1:23pm format (12-hour, no zero)
            case 'C':
                valid = timestamp.parseCivilTime(intime->getStringData());
                break;

            // 'L'ong time, full 24-hour, plus fractional
            case 'L':
                valid = timestamp.parseLongTime(intime->getStringData());
                break;

            case 'H':                        /* 'H'ours format                    */
            {
                int i = intime->longValue(9);      /* convert the value                 */
                valid = i != NO_LONG && timestamp.setHours(i);
                break;
            }

            case 'S':                        /* 'S'econds format                  */
            {
                int i = intime->longValue(9);      /* convert the value                 */
                valid = i != NO_LONG && timestamp.setSeconds(i);
                break;
            }

            case 'M':                        /* 'M'inutes format                  */
            {
                int i = intime->longValue(9);      /* convert the value                 */
                valid = i != NO_LONG && timestamp.setMinutes(i);
                break;
            }

            case 'F':                        /* 'F'ull datetime stamp            */
            {
                /*convert the value                  */
                int64_t basetime;
                if (!Numerics::objectToInt64(intime, basetime) || !timestamp.setBaseTime(basetime))
                {
                    report_exception3(Error_Incorrect_call_format_invalid, new_cstring(CHAR_TIME), intime, new_string((PCHAR)&style2, 1));
                }
                break;
            }

            case 'T':                        /* 'T'icks datetime stamp            */
            {
                /*convert the value                  */
                int64_t basetime;
                if (!Numerics::objectToInt64(intime, basetime) || !timestamp.setUnixTime(basetime))
                {
                    report_exception3(Error_Incorrect_call_format_invalid, new_cstring(CHAR_TIME), intime, new_string((PCHAR)&style2, 1));
                }
                break;
            }

            default:
                work[0] = style2;              /* copy over the character           */
                report_exception4(Error_Incorrect_call_list, new_cstring(CHAR_TIME), IntegerThree, new_string("CFHLMNS", 6), new_string(work, 1));
                break;
        }
        if (!valid)                        /* not convert cleanly?              */
        {
            report_exception3(Error_Incorrect_call_format_invalid, new_cstring(CHAR_TIME), intime, new_string((PCHAR)&style2, 1) );
        }
    }

    switch (style)
    {                     /* process the styles                */

        case 'E':                         /* 'E'lapsed time                    */
        case 'R':                         /* 'R'eset elapsed time              */
        {
            /* get the current elapsed time      */
            int64_t startTime = context->getElapsed();
            // substract the time values
            int64_t threshold = current.getBaseTime() - startTime;
            if (threshold < 0)
            {
                strcpy(work, "0");            /* just return zero                  */
                context->resetElapsed();      /* reset the clock for next time     */
            }                                 /* times equal?                      */
            else if (threshold == 0)
            {
                strcpy(work, "0");            /* just return zero                  */
            }
            else
            {
                // format as a long time
                sprintf(work, "%lu.%06lu", (int)(threshold / (int64_t)MICROSECONDS), (int)(threshold % (int64_t)MICROSECONDS));
            }
                /* format the result                 */
            if (style == 'R')               /* is this a reset call?             */
            {
                context->resetElapsed();      /* reset the clock for next time     */
            }
            break;
        }

        case 'C':                         /* 'C'ivil time                      */
            timestamp.formatCivilTime(work);
            break;

        case 'H':                         /* 'Hours'                           */
            timestamp.formatHours(work);
            break;

        case 'L':                         /* 'L'ong format                     */
            timestamp.formatLongTime(work);
            break;

        case 'M':                         /* 'M'inutes format                  */
            timestamp.formatMinutes(work);
            break;

        case 'N':                         /* 'N'ormal format...the default     */
            timestamp.formatNormalTime(work);
            break;

        case 'S':                         /* 'S'econds format...total seconds  */
            timestamp.formatSeconds(work);
            break;

        case 'F':                          /* 'F'ull                            */
            timestamp.formatBaseTime(work);
            break;

        case 'T':                          /* 'T'icks                           */
            timestamp.formatUnixTime(work);
            break;

        default:                          /* unknown format                    */
            work[0] = style;                /* copy over the character           */
            report_exception4(Error_Incorrect_call_list, new_cstring(CHAR_TIME), IntegerOne, new_string("CEFHLMNRST", 8), new_string(work, 1));
            break;
    }
    /* now create a string object        */
    return new_cstring(work);
}

#define RANDOM_MIN 0
#define RANDOM_MAX 3
#define RANDOM_minimum 1
#define RANDOM_maximum 2
#define RANDOM_seed    3

BUILTIN(RANDOM) {
  RexxInteger *minimum;                /* RANDOM minimum value              */
  RexxInteger *maximum;                /* RANDOM maximum value              */
  RexxInteger *seed;                   /* RANDOM seed value                 */

  fix_args(RANDOM);                    /* expand arguments to full value    */
  // we need a special case here.  the interpretation of Random is such that
  // random() is NOT the same as Random(,).
  if (argcount == 2 && arg_omitted(RANDOM, minimum) && arg_omitted(RANDOM, maximum))
  {
      minimum = IntegerZero;
      maximum = new_integer(999);
  }
  else
  {
                                           /* get the minimum value             */
      minimum = optional_integer(RANDOM, minimum);
                                           /* get the maximum value             */
      maximum = optional_integer(RANDOM, maximum);
  }
                                       /* get the seed value                */
  seed    = optional_integer(RANDOM, seed);
                                       /* have the activation generate      */
  return context->random(minimum, maximum, seed);
}

#define XRANGE_MIN 0
#define XRANGE_MAX 2
#define XRANGE_start   1
#define XRANGE_end     2

BUILTIN(XRANGE) {
  RexxString *start;                   /* starting position                 */
  RexxString *end;                     /* ending position                   */
  RexxString *result;                  /* XRANGE result                     */
  UCHAR  startchar;                    /* starting character                */
  UCHAR  endchar;                      /* ending character                  */
  LONG   length;                       /* length of result                  */
  LONG   i;                            /* loop counter                      */

  fix_args(XRANGE);                    /* expand arguments to full value    */
  startchar = 0;                       /* set default start position        */
  endchar = 0xff;                      /* set default end position          */

                                       /* get the starting string           */
  start = optional_string(XRANGE, start);
  end = optional_string(XRANGE, end);  /* get the ending string             */

  if (start != OREF_NULL) {            /* have a start position             */
    if (start->length != 1)            /* not a single character?           */
                                       /* have an error                     */
      report_exception3(Error_Incorrect_call_pad, new_cstring(CHAR_XRANGE), IntegerOne, start);
    startchar = start->getChar(0);     /* get the new start position        */
  }
  if (end != OREF_NULL) {              /* have an end position              */
    if (end->length != 1)              /* not a single character?           */
                                       /* have an error                     */
      report_exception3(Error_Incorrect_call_pad, new_cstring(CHAR_XRANGE), IntegerTwo, end);
    endchar = end->getChar(0);         /* get the new end position          */
  }
                                       /* calculate result size             */
  length = ((endchar < startchar) ? (256 - startchar) + endchar : (endchar - startchar)) + 1;
  result = raw_string(length);         /* get a result string               */
  for (i = 0; i < length; i++)         /* loop through result length        */
    result->putChar(i, startchar++);   /* inserting each character          */
  result->generateHash();              /* rebuild the hash value            */
  return result;                       /* finished                          */
}

#define SYMBOL_MIN 1
#define SYMBOL_MAX 1
#define SYMBOL_name    1

BUILTIN(SYMBOL) {
  RexxVariableBase *variable;          /* specified variable name           */
  RexxString *result;                  /* function result                   */

  fix_args(SYMBOL);                    /* expand arguments to full value    */
                                       /* get the variable name             */
  result = required_string(SYMBOL, name);
                                       /* get a variable retriever          */
  variable = context->getVariableRetriever(result);
  if (variable == OREF_NULL)           /* invalid variable name?            */
                                       /* return the 'BAD' result           */
    result = (RexxString *)new_cstring(CHAR_BAD);
  else if (OTYPE(String, variable))    /* directly returned a string?       */
                                       /* this is a literal value           */
    result = (RexxString *)new_cstring(CHAR_LIT);
  else {                               /* need to perform lookup            */
                                       /* see if variable has a value       */
    if (!variable->exists(context))
                                       /* this is a literal value           */
      result = (RexxString *)new_cstring(CHAR_LIT);
    else
                                       /* this is a variable value          */
      result = (RexxString *)new_cstring(CHAR_VAR);
  }
  return result;                       /* return the indicator              */
}

#define VAR_MIN 1
#define VAR_MAX 1
#define VAR_name    1

BUILTIN(VAR) {
  RexxString       *variable;          /* specified variable name           */
  RexxVariableBase *retriever;         /* variable retriever                */
  RexxObject       *result;            /* function result                   */

  fix_args(VAR);                       /* expand arguments to full value    */
                                       /* get the variable name             */
  variable = required_string(VAR, name);
                                       /* get a variable retriever          */
  retriever = context->getVariableRetriever(variable);
  if (retriever == OREF_NULL)          /* invalid variable name?            */
    result = TheFalseObject;           /* return the 'BAD' result           */
  else if (OTYPE(String, retriever))   /* directly returned a string?       */
    result = TheFalseObject;           /* this doesn't exist either         */
  else {                               /* need to perform lookup            */
                                       /* get the variable value            */
    result = retriever->exists(context) ? TheTrueObject : TheFalseObject;
  }
  return result;                       /* return the indicator              */
}

#define VALUE_MIN 1
#define VALUE_MAX 3
#define VALUE_name     1
#define VALUE_newValue 2
#define VALUE_selector 3

BUILTIN(VALUE) {
  RexxString  *variable;               /* specified variable name           */
  RexxVariableBase *retriever;         /* variable retriever                */
  RexxObject  *newvalue;               /* new variable value                */
  RexxObject  *result;                 /* function result                   */
  RexxString  *selector;               /* variable pool selector            */

  fix_args(VALUE);                     /* expand arguments to full value    */
                                       /* get the variable name             */
  variable = required_string(VALUE, name);
                                       /* get the new value                 */
  newvalue = optional_argument(VALUE, newValue);
                                       /* and the selector                  */
  selector = optional_string(VALUE, selector);
  // get the variable type
  int variableType = variable->isSymbol();
  bool assignable = variableType == STRING_NAME || variableType == STRING_STEM || variableType == STRING_COMPOUND_NAME;

  if (selector == OREF_NULL) {         /* have a selector?                  */
                                       /* get a variable retriever          */
    retriever = context->getVariableRetriever(variable);
    // this could an invalid name, or we might be trying to assign a value to a non-variable
    // symbol.
    if (retriever == OREF_NULL || (newvalue != OREF_NULL && !assignable))
    {
        report_exception3(Error_Incorrect_call_symbol, new_cstring(CHAR_VALUE), IntegerOne, variable);
    }
    else {                             /* need to perform lookup            */
                                       /* get the variable value            */
      result = retriever->getValue(context);
      if (newvalue != OREF_NULL)       /* have a new value to assign?       */
                                       /* do the assignment                 */
        retriever->assign(context, stack, newvalue);
    }
  }
  else if (selector->length == 0) {    /* null string selector?             */
                                       /* get the existing value            */
    result = TheEnvironment->entry(variable);
    if (result == OREF_NULL)           /* not in the environment?           */
                                       /* turn into ".VARIABLE" as value    */
      result = ((RexxString *)OREF_PERIOD)->concat(variable->upper());
    if (newvalue != OREF_NULL)         /* have a new value?                 */
                                       /* do the set also                   */
      TheEnvironment->setEntry(variable, newvalue);
  }
  else {                               /* external value function           */
                                       /* need to go external on this       */
    result = (RexxObject *)SysValue(variable, newvalue, selector);
  }
  return result;                       /* return the indicator              */
}

#define ABS_MIN 1
#define ABS_MAX 1
#define ABS_n   1

BUILTIN(ABS) {
    fix_args(ABS);                       /* check on required number of args  */
    /* get the argument in question      */
    RexxObject *argument = get_arg(ABS, n);
    if (OTYPE(Integer, argument)) {      /* integer object already?           */
        /* we can process this without conversion */
        return ((RexxInteger *)argument)->abs();
    }
    else if (OTYPE(NumberString, argument)) { /* how about already numeric?        */
        /* we can process this without conversion */
        return ((RexxNumberString *)argument)->abs();
    }
    /* force to a string object          */
    RexxString *n = required_string(ABS, n);
    return n->abs();                   /* invoke the string ABS function    */
}

#define SIGN_MIN 1
#define SIGN_MAX 1
#define SIGN_n   1

BUILTIN(SIGN) {
    fix_args(SIGN);                       /* check on required number of args  */
    /* get the argument in question      */
    RexxObject *argument = get_arg(SIGN, n);
    if (OTYPE(Integer, argument)) {       /* integer object already?           */
        /* we can process this without conversion */
        return ((RexxInteger *)argument)->sign();
    }
    else if (OTYPE(NumberString, argument)) { /* how about already numeric?        */
        /* we can process this without conversion */
        return ((RexxNumberString *)argument)->Sign();
    }
    /* force to a string object          */
    RexxString *n = required_string(SIGN, n);
    return n->sign();                     /* invoke the string SIGN function    */
}

#define FORMAT_MIN 1
#define FORMAT_MAX 5
#define FORMAT_number 1
#define FORMAT_before 2
#define FORMAT_after  3
#define FORMAT_expp   4
#define FORMAT_expt   5

BUILTIN(FORMAT) {
  RexxString  *number;                 /* number to format                  */
  RexxInteger *before;                 /* before part                       */
  RexxInteger *after;                  /* after digits                      */
  RexxInteger *expp;                   /* expp value                        */
  RexxInteger *expt;                   /* expt value                        */

  fix_args(FORMAT);                    /* check on required number of args  */
                                       /* force to a string object          */
  number = required_string(FORMAT, number);
                                       /* before value is optional          */
  before = optional_integer(FORMAT, before);
                                       /* after value is optional           */
  after = optional_integer(FORMAT, after);
                                       /* expp value is optional            */
  expp = optional_integer(FORMAT, expp);
                                       /* expt value is optional            */
  expt = optional_integer(FORMAT, expt);
                                       /* invoke the string FORMAT function   */
  return number->format(before, after, expp, expt);
}

#define MAX_MIN 1
#define MAX_MAX argcount
#define MAX_target 1

BUILTIN(MAX) {
  check_args(MAX);                     /* check on required args            */
  /* get the argument in question      */
  RexxObject *argument = get_arg(MAX, target);
  if (OTYPE(NumberString, argument)) { /* how about already numeric?        */
      /* we can process this without conversion */
      return ((RexxNumberString *)argument)->Max(stack->arguments(argcount - 1), argcount - 1);
  }
                                       /* get the target string             */
  RexxString *target = required_string(MAX, target);
  /* go perform the MIN function       */
  return target->Max(stack->arguments(argcount - 1), argcount - 1);
}

#define MIN_MIN 1
#define MIN_MAX argcount
#define MIN_target 1

BUILTIN(MIN) {
  check_args(MIN);                     /* check on required args            */
  /* get the argument in question      */
  RexxObject *argument = get_arg(MIN, target);
  if (OTYPE(NumberString, argument)) { /* how about already numeric?        */
      /* we can process this without conversion */
      return ((RexxNumberString *)argument)->Min(stack->arguments(argcount - 1), argcount - 1);
  }
                                       /* get the target string             */
  RexxString *target = required_string(MIN, target);
  /* go perform the MIN function       */
  return target->Min(stack->arguments(argcount - 1), argcount - 1);
}

#define SOURCELINE_MIN 0
#define SOURCELINE_MAX 1
#define SOURCELINE_n   1

BUILTIN(SOURCELINE) {
  LONG   line_number;                  /* requested error number            */
  RexxObject  *result;                 /* function result                   */
  RexxSource  *source;                 /* current program source            */
  LONG   size;                         /* size of source program            */

  fix_args(SOURCELINE);                /* check on required number of args  */
  source = context->getSource();       /* get current source object         */
  size = source->sourceSize();         /* get the program size              */
  if (argcount == 1) {                 /* asking for a specific line?       */
                                       /* get the line number               */
    line_number = required_integer(SOURCELINE, n)->value;
                                       /* must be a positive integer        */
    positive_integer(line_number, SOURCELINE, IntegerOne);
    if (line_number > size)            /* larger than program source?       */
                                       /* this is an error too?             */
      report_exception2(Error_Incorrect_call_sourceline, new_integer(line_number), new_integer(size));
                                       /* get the specific line             */
    result = (RexxObject *)source->get(line_number);
  }
  else
                                       /* just return the source size       */
    result = (RexxObject *)new_integer(size);
  return result;                       /* finished                          */
}

#define TRACE_MIN 0
#define TRACE_MAX 1
#define TRACE_setting 1

BUILTIN(TRACE) {
  RexxString  *result;                 /* returned result                   */
  RexxString  *setting;                /* new trace setting                 */
  INT    newsetting;                   /* new trace setting                 */
  INT    debug;                        /* new debug setting                 */

  fix_args(TRACE);                     /* check required arguments          */
                                       /* get the trace setting             */
  setting = optional_string(TRACE, setting);
  result = context->traceSetting();    /* get the existing trace setting    */
  if (setting != OREF_NULL) {          /* have a new setting?               */
    context->source->parseTraceSetting(setting, &newsetting, &debug);
                                       /* now change the setting            */
    context->setTrace(newsetting, debug);
  }
  return result;                       /* return old trace setting          */
}

RexxObject *resolve_stream(            /* resolve a stream name             */
  RexxString          *name,           /* name of the stream                */
  RexxActivation      *context,        /* current activation context        */
  RexxExpressionStack *stack,          /* current expression stack          */
  BOOL                 input,          /* input or an output stream         */
  RexxString          **fullName,      /* qualified name of stream          */
  BOOL                *added)          /* not found -> TRUE                 */
/******************************************************************************/
/* Function:  Convert a stream name into a stream object                      */
/******************************************************************************/
{
  RexxObject    *stream;               /* associated stream                 */
  RexxDirectory *streamTable;          /* current set of open streams       */
  RexxObject    *streamClass;          /* current stream class              */
  RexxString    *qualifiedName;        /* qualified file name               */
  RexxDirectory *securityArgs;         /* security check arguments          */

  if (added) *added = FALSE;           /* when caller requires stream table entry then initialize */
  streamTable = context->getStreams(); /* get the current stream set        */
  if (fullName)                        /* fullName requested?               */
    *fullName = name;                  /* initialize to name                */
  /* if length of name is 0, then it's the same as omitted */
  if (name == OREF_NULL || name->length == 0) { /* no name?                 */
    if (input) {                       /* input operation?                  */
                                       /* get the default output stream     */
      stream = CurrentActivity->local->at(OREF_INPUT);
    }
    else {
                                       /* get the default output stream     */
      stream = CurrentActivity->local->at(OREF_OUTPUT);
    }
  }
                                       /* standard input stream?            */
  else if (name->strICompare(CHAR_STDIN) || name->strICompare(CHAR_CSTDIN))
                                       /* get the default output stream     */
    stream = CurrentActivity->local->at(OREF_INPUT);
                                       /* standard output stream?           */
  else if (name->strICompare(CHAR_STDOUT) || name->strICompare(CHAR_CSTDOUT))
                                       /* get the default output stream     */
    stream = CurrentActivity->local->at(OREF_OUTPUT);
                                       /* standard error stream?            */
  else if (name->strICompare(CHAR_STDERR) || name->strICompare(CHAR_CSTDERR))
                                       /* get the default output stream     */
    stream = CurrentActivity->local->at(OREF_ERRORNAME);
  else {
//  stream = streamTable->at(name);    /* first try supplied name           */
//  if (stream != OREF_NULL)           /* get one?                          */
//    return stream;                   /* get out of here                   */
                                       /* go get the qualified name         */
    qualifiedName = (RexxString *)SysQualifyFileSystemName(name);
    if (fullName)                      /* fullName requested?               */
      *fullName = qualifiedName;       /* provide qualified name            */
    stack->push(qualifiedName);        /* Protect from GC.                  */
    /* Note: stream name is pushed to the stack to be protected from GC;    */
    /* e.g. it is used by the caller to remove stream from stream table.    */
    /* The stack will be reset after the function was executed and the      */
    /* protection is released                                               */
                                       /* see if we've already opened this  */
    stream = streamTable->at(qualifiedName);
    if (stream == OREF_NULL) {         /* not open                          */
                                       /* need to secure this?              */
      if (context->hasSecurityManager()) {
        securityArgs = new_directory();/* get the information directory     */
                                       /* put the name in the directory     */
        securityArgs->put(qualifiedName, OREF_NAME);
        if (context->callSecurityManager(OREF_STREAM, securityArgs)) {
          stream = securityArgs->fastAt(OREF_STREAM);
          if (stream == OREF_NULL)     /* in an expression and need a result*/
                                       /* need to raise an exception        */
            report_exception1(Error_No_result_object_message, OREF_STREAM);
                                       /* add to the streams table          */
          streamTable->put(stream, qualifiedName);
          return stream;               /* return the stream object          */
        }
      }
//    stack->push(qualifiedName);      /* Protect from GC;moved up          */
                                       /* get the stream class              */
      streamClass = TheEnvironment->at(OREF_STREAM);
                                       /* create a new stream object        */
      stream = send_message1(streamClass, OREF_NEW, name);

      if (added) {                     /* open the stream?   begin          */
                                       /* add to the streams table          */
        streamTable->put(stream, qualifiedName);
        *added = TRUE;                 /* mark it as added to stream table  */
      }
//    streamTable->put(stream, name);  /* under both names                  */
    }
  }

  return stream;                       /* return the stream object          */
}

BOOL check_queue(                      /* check to see if stream is to queue*/
  RexxString *name)                    /* namve of the stream               */
/******************************************************************************/
/* Function:  Check to see if a stream name is a queue                        */
/******************************************************************************/
{
  if (name != OREF_NULL)               /* non-default name?                 */
    return name->strICompare("QUEUE:");/* compare against the queue         */
  else return FALSE;                   /* not the queue                     */
}

#define LINEIN_MIN 0
#define LINEIN_MAX 3
#define LINEIN_name   1
#define LINEIN_line   2
#define LINEIN_count  3

BUILTIN(LINEIN) {
  RexxObject   *stream;                /* target stream                     */
  RexxInteger  *line;                  /* target line                       */
  RexxInteger  *count;                 /* count of lines                    */
  RexxString   *name;                  /* stream name                       */
  RexxString   *result;                /* linein result                     */
  BOOL          added;                 /* add to stream table               */

  fix_args(LINEIN);                    /* check required arguments          */

  name = optional_string(LINEIN, name);/* get the string name               */
                                       /* get the line position             */
  line = optional_integer(LINEIN, line);
                                       /* and the optional count of lines   */
  count = optional_integer(LINEIN, count);
  if (check_queue(name)) {             /* is this "QUEUE:"                  */
                                       /* if exit declines call             */
    if (CurrentActivity->sysExitMsqPll(context, &result)) {
                                       /* get the default output stream     */
        stream = CurrentActivity->local->at(OREF_REXXQUEUE);
                                       /* pull from the queue               */
        result = (RexxString *)send_message0(stream, OREF_LINEIN);
    }
  }
  else {
                                       /* get a stream for this name        */
    stream = resolve_stream(name, context, stack, TRUE, NULL, &added);
    switch (argcount) {                /* process according to argcount     */
      case 0:                          /* no name                           */
      case 1:                          /* name only                         */
        result = (RexxString *)send_message0(stream, OREF_LINEIN);
        break;
      case 2:                          /* name and start                    */
        result = (RexxString *)send_message1(stream, OREF_LINEIN, line);
        break;
      case 3:                          /* name, start and count             */
        result = (RexxString *)send_message2(stream, OREF_LINEIN, line, count);
        break;
    }
  }
  return result;                       /* all finished                      */
}

#define CHARIN_MIN 0
#define CHARIN_MAX 3
#define CHARIN_name   1
#define CHARIN_start  2
#define CHARIN_count  3

BUILTIN(CHARIN) {
  RexxObject   *stream;                /* target stream                     */
  RexxInteger  *position;              /* target position                   */
  RexxInteger  *count;                 /* count of lines                    */
  RexxString   *name;                  /* stream name                       */
  RexxString   *result;                /* linein result                     */
  BOOL          added;                 /* add to stream table               */

  fix_args(CHARIN);                    /* check required arguments          */
                                       /* get the string name               */
  name = optional_string(CHARIN, name);
                                       /* get the line position             */
  position = optional_integer(CHARIN, start);
                                       /* and the optional count of chars   */
  count = optional_integer(CHARIN, count);
  if (check_queue(name))               /* is this "QUEUE:"                  */
                                       /* this isn't allowed                */
    report_exception1(Error_Incorrect_call_queue_no_char, OREF_CHARIN);

                                       /* get a stream for this name        */
  stream = resolve_stream(name, context, stack, TRUE, NULL, &added);
  switch (argcount) {                  /* process according to argcount     */
    case 0:                            /* no name                           */
    case 1:                            /* name only                         */
      result = (RexxString *)send_message0(stream, OREF_CHARIN);
      break;
    case 2:                            /* name and string                   */
      result = (RexxString *)send_message1(stream, OREF_CHARIN, position);
      break;
    case 3:                            /* name, string and line             */
      result = (RexxString *)send_message2(stream, OREF_CHARIN, position, count);
      break;
  }
  return result;                       /* return final result               */
}

#define LINEOUT_MIN 0
#define LINEOUT_MAX 3
#define LINEOUT_name   1
#define LINEOUT_string 2
#define LINEOUT_line   3

BUILTIN(LINEOUT) {
  RexxObject   *stream;                /* target stream                     */
  RexxInteger  *line;                  /* target position                   */
  RexxString   *name;                  /* stream name                       */
  RexxString   *result;                /* linein result                     */
  RexxString   *string;                /* target string                     */
  RexxString   *fullName;              /* fully qual'd stream name          */
  BOOL          added;                 /* add to stream table               */

  fix_args(LINEOUT);                   /* check required arguments          */
                                       /* get the string name               */
  name = optional_string(LINEOUT, name);
                                       /* get the output string             */
  string = optional_string(LINEOUT, string);
                                       /* get the line position             */
  line = optional_integer(LINEOUT, line);
  if (check_queue(name)) {             /* is this "QUEUE:"                  */
                                       /* if exit declines call             */
    if (CurrentActivity->sysExitMsqPsh(context, string, QUEUE_FIFO)) {
      if (string != OREF_NULL) {       /* have an actual string to write?   */
                                       /* get the default output stream     */
        stream = CurrentActivity->local->at(OREF_REXXQUEUE);
                                       /* push onto the queue               */
        result = (RexxString *)send_message1(stream, OREF_QUEUENAME, string);
      }
      else
                                       /* always a zero residual            */
        result = (RexxString *)IntegerZero;
    }
  }
  else {
                                       /* get a stream for this name        */
    stream = resolve_stream(name, context, stack, FALSE, &fullName, &added);
    switch (argcount) {                /* process according to argcount     */
      case 0:                          /* no name                           */
      case 1:                          /* name only                         */
        result = (RexxString *)send_message0(stream, OREF_LINEOUT);
        break;
      case 2:                          /* name and string                   */
        result = (RexxString *)send_message1(stream, OREF_LINEOUT, string);
        break;
      case 3:                          /* name, string and line             */
        result = (RexxString *)send_message2(stream, OREF_LINEOUT, string, line);
        break;
    }
  }
  return result;                       /* all finished                      */
}

#define CHAROUT_MIN 0
#define CHAROUT_MAX 3
#define CHAROUT_name   1
#define CHAROUT_string 2
#define CHAROUT_start  3

BUILTIN(CHAROUT) {
  RexxObject   *stream;                /* target stream                     */
  RexxInteger  *position;              /* target position                   */
  RexxString   *name;                  /* stream name                       */
  RexxString   *result;                /* linein result                     */
  RexxString   *string;                /* target string                     */
  RexxString   *fullName;              /* fully qual'd stream name          */
  BOOL          added;                 /* add to stream table               */

  fix_args(CHAROUT);                   /* check required arguments          */
                                       /* get the string name               */
  name = optional_string(CHAROUT, name);
                                       /* get the output string             */
  string = optional_string(CHAROUT, string);
                                       /* get the line position             */
  position = optional_integer(CHAROUT, start);
  if (check_queue(name))               /* is this "QUEUE:"                  */
                                       /* this isn't allowed                */
    report_exception1(Error_Incorrect_call_queue_no_char, OREF_CHAROUT);
                                       /* get a stream for this name        */
  stream = resolve_stream(name, context, stack, FALSE, &fullName, &added);
  switch (argcount) {                  /* process according to argcount     */
    case 0:                            /* no name                           */
    case 1:                            /* name only                         */
      result = (RexxString *)send_message0(stream, OREF_CHAROUT);
      break;
    case 2:                            /* name and string                   */
      result = (RexxString *)send_message1(stream, OREF_CHAROUT, string);
      break;
    case 3:                            /* name, string and line             */
      result = (RexxString *)send_message2(stream, OREF_CHAROUT, string, position);
      break;
  }
  return result;                       /* all finished                      */
}

#define LINES_MIN 0
#define LINES_MAX    2
#define LINES_name   1
#define LINES_option 2

BUILTIN(LINES) {
  RexxObject   *stream;                /* target stream                     */
  RexxString   *name;                  /* stream name                       */
  RexxString   *option;                /* option: "N" or "C"                */
  RexxInteger  *result;                /* linein result                     */
  BOOL          added;                 /* add to stream table               */
  RexxString    *quick;

  fix_args(LINES);                     /* check required arguments          */

  name = optional_string(LINES, name); /* get the string name               */
  option = optional_string(LINES, option);
  if (check_queue(name)) {             /* is this "QUEUE:"                  */
                                       /* get the default output stream     */
    stream = CurrentActivity->local->at(OREF_REXXQUEUE);
                                       /* return count on the queue         */
    result = (RexxInteger *)send_message0(stream, OREF_QUERY);
  }
  else
  {
                                       /* get a stream for this name        */
    stream = resolve_stream(name, context, stack, TRUE, NULL, &added);

    if (option != OREF_NULL)
      switch (option->getChar(0)) {      /* process the option character      */
        case 'C':
        case 'c':
          quick = new_cstring("C");
          break;
        case 'N':
        case 'n':
          quick = new_cstring("N");
          break;
        case '\0':                        /* argument 'O'mitted?               */
          quick = new_cstring("N");
          break;
        default:                         /* unknown option                    */
                                         /* this is an error                  */
          report_exception4(Error_Incorrect_call_list, new_cstring(CHAR_ARG), IntegerTwo, new_string("NC", 2), option);
          break;
    }
    else
      quick = new_cstring("N");

    /* use modified LINES method with quick flag       */
    result = (RexxInteger *)send_message1(stream, OREF_LINES,quick);
  }
                                       /* for compatibility this needs      */
                                       /* to only return 0 or 1             */
  if (quick->getChar(0) == 'N')
    return (result != IntegerZero) ? IntegerOne : IntegerZero;
  else
    return result;
}

#define CHARS_MIN 0
#define CHARS_MAX 1
#define CHARS_name   1

BUILTIN(CHARS) {
  RexxObject   *stream;                /* target stream                     */
  RexxString   *name;                  /* stream name                       */
  BOOL          added;                 /* add to stream table               */

  fix_args(CHARS);                     /* check required arguments          */

  name = optional_string(CHARS, name); /* get the string name               */
  if (check_queue(name))               /* is this "QUEUE:"                  */
                                       /* this isn't allowed                */
    report_exception1(Error_Incorrect_call_queue_no_char, OREF_CHARS);
                                       /* get a stream for this name        */
  stream = resolve_stream(name, context, stack, TRUE, NULL, &added);
  return send_message0(stream, OREF_CHARS);
}

#define STREAM_MIN 1
#undef STREAM_MAX                      /* already defined in AIX            */
#define STREAM_MAX 3
#define STREAM_name      1
#define STREAM_operation 2
#define STREAM_command   3

#define STREAM_STATUS      'S'
#define STREAM_DESCRIPTION 'D'
#define STREAM_COMMAND     'C'

BUILTIN(STREAM) {
  RexxObject   *stream;                /* target stream                     */
  RexxString   *name;                  /* stream name                       */
  RexxString   *action;                /* stream action                     */
  RexxString   *command;               /* stream command                    */
  RexxString   *command_upper;         /* stream command upper case         */
  RexxObject   *result;                /* function result                   */
  CHAR          action_char;           /* reduced action character          */
  RexxString   *fullName;              /* fully qual'd stream name          */
  BOOL          added;
  BOOL          fOpen = FALSE;         /* open flag                         */
  BOOL          fClose = FALSE;        /* close flag                        */

  fix_args(STREAM);                    /* check required arguments          */
                                       /* get the string name               */
  name = required_string(STREAM, name);
  if (name->length == 0)               /* check name validity               */
                                       /* raise an error                    */
    report_exception2(Error_Incorrect_call_stream_name, OREF_STREAM, name);
                                       /* get any operation                 */
  action = optional_string(STREAM, operation);
                                       /* get any command                   */
  command = optional_string(STREAM, command);
                                       /* get a stream for this name        */
  /* we need to now, whether the stream already was in the stream table (added = FALSE) */
//stream = resolve_stream(name, context, stack, TRUE, &added);

  if (action == OREF_NULL) {           /* no action given?                  */
    action_char = STREAM_STATUS;       /* this is a status attempt          */
  }
  else {
    if (action->length == 0) {         /* get a null string?                */
                                       /* this is an error                  */
      report_exception4(Error_Incorrect_call_list, OREF_STREAM, IntegerTwo, new_string("SDC", 3), action);
    }
    else {
                                       /* get the option character          */
      action_char = toupper(action->getChar(0));
    }
  }
  switch (action_char) {               /* process the options               */
    case STREAM_STATUS:                /* stream(name, s)                   */
      if (argcount > 2) {              /* given a third argument?           */
                                       /* raise an error                    */
        report_exception2(Error_Incorrect_call_maxarg, OREF_STREAM, IntegerTwo);
      }
      stream = resolve_stream(name, context, stack, TRUE, NULL, NULL);
                                       /* get the stream state              */
      result = send_message0(stream, OREF_STATE);
      break;

    case STREAM_DESCRIPTION:           /* stream(name, d)                   */
      if (argcount > 2) {              /* given a third argument?           */
                                       /* raise an error                    */
        report_exception2(Error_Incorrect_call_maxarg, OREF_STREAM, IntegerTwo);
      }
      stream = resolve_stream(name, context, stack, TRUE, NULL, NULL);
                                       /* get the stream description        */
      result = send_message0(stream, OREF_DESCRIPTION);
      break;

    case STREAM_COMMAND:               /* stream(name, c, command)          */
      if (argcount < 3) {              /* given a third argument?           */
                                       /* raise an error                    */
        report_exception2(Error_Incorrect_call_minarg, OREF_STREAM, IntegerThree);
      }
                                       /* get the stream description        */
      save(command);                   /* use save instead of hold */

      /* I have to check the command twice because in the RexxMethods (i.g. query_exists)
         I don't have access to the activation and thus not to the streamtable.
         It's also not possible to pass context as the second argument because
         stream is a RexxMethod and USE ARG RexxActivation is not possible */
      command_upper = command->upper();
      save(command_upper);           /* use save instead of hold */


      fOpen = fClose = FALSE;    /* this whole part was reworked and moved up here */
      if (command_upper->wordPos(new_cstring("OPEN"), OREF_NULL)->value > 0) {
        fOpen = TRUE;
        stream = resolve_stream(name, context, stack, TRUE, &fullName, &added);
      }
      else if (command_upper->wordPos(new_cstring("CLOSE"), OREF_NULL)->value > 0) {
        fClose = TRUE;
        stream = resolve_stream(name, context, stack, TRUE, &fullName, &added);
      }
      else
        stream = resolve_stream(name, context, stack, TRUE, NULL, NULL);

      result = send_message1(stream, OREF_COMMAND, command);

      /* this repairs the removed code below */
      if (fClose) {
        context->getStreams()->remove(fullName);
      } else if (added && fOpen) {
        /* if open failed, remove the stream object from stream table again */
        if (memcmp("READY:", ((RexxString*) result)->stringData, 6) != 0) {
          context->getStreams()->remove(fullName);
        }
      }

      discard(command);        /* use discard instead of discard_hold */
      discard(command_upper);  /* use discard instead of discard_hold */
      break;

    default:
                                       /* this is an error                  */
      report_exception4(Error_Incorrect_call_list, OREF_STREAM, IntegerTwo, new_string("SDC", 3), action);
      break;
  }
  return result;                       /* return the function result        */
}

#define QUEUED_MIN 0
#define QUEUED_MAX 0

BUILTIN(QUEUED) {
  RexxObject   *queue;                 /* current queue object              */
  RexxInteger  *queuesize;             /* returned queue size from sys exit */

  check_args(QUEUED);                  /* check on required number of args  */
                                       /* get the default output stream     */
  if (CurrentActivity->sysExitMsqSiz(context, &queuesize)) {
    queue = CurrentActivity->local->at(OREF_REXXQUEUE);
                                       /* return count on the queue         */
    return send_message0(queue, OREF_QUEUED);
  }
  else
    return queuesize;                  /* return count from system exit     */
}

#define CONDITION_MIN 0
#define CONDITION_MAX 1
#define CONDITION_option 1

BUILTIN(CONDITION) {
  INT   style;                         /* style of condition output         */
  RexxString    *option;               /* function option                   */
  RexxDirectory *conditionobj;         /* current trapped condition object  */
  RexxObject    *result;               /* returned result                   */

  fix_args(CONDITION);                 /* expand arguments to full value    */
                                       /* get the option string             */
  option = optional_string(CONDITION, option);
  if (option == OREF_NULL)             /* just using default format?        */
    style = 'I';                       /* use the 'Normal form              */
  else if (option->length == 0)        /* have a null string?               */
                                       /* this is an error                  */
    report_exception4(Error_Incorrect_call_list, new_cstring(CHAR_CONDITION), IntegerOne, new_string("ACDIOS", 6), option);
  else                                 /* need to process an option         */
                                       /* option is first character         */
    style = toupper(option->getChar(0));
                                       /* get current trapped condition     */
  conditionobj = context->getConditionObj();

  result = OREF_NULLSTRING;            /* use a nullstring safety net       */
  switch (style) {                     /* process various CONDITION objects */

    case 'A':                          /* 'A'dditional                      */
      if (conditionobj != OREF_NULL) { /* have a condition object?          */
                                       /* retrieve the additional info      */
        result = conditionobj->at(OREF_ADDITIONAL);
        if (result == OREF_NULL)       /* not there?                        */
          result = TheNilObject;       /* return .nil                       */
        else
          result = result->copy();     /* copy the result info              */
      }
      else
        result = TheNilObject;         /* return .nil if not there          */
      break;

    case 'I':                          /* 'I'nstruction                     */
      if (conditionobj != OREF_NULL)   /* have a condition object?          */
                                       /* retrieve the instruction info     */
        result = conditionobj->at(OREF_INSTRUCTION);
      break;

    case 'D':                          /* 'D'escription                     */
      if (conditionobj != OREF_NULL) { /* have a condition object?          */
                                       /* retrieve the description info     */
        result = conditionobj->at(OREF_DESCRIPTION);
        if (result == OREF_NULL)       /* not found?                        */
          result = OREF_NULLSTRING;    /* return a null string if nothing   */
      }
      break;

    case 'C':                          /* 'C'ondition name                  */
      if (conditionobj != OREF_NULL)   /* have a condition object?          */
                                       /* retrieve the condition name       */
        result = conditionobj->at(OREF_CONDITION);
      break;

    case 'O':                          /* 'C'ondition name                  */
      if (conditionobj != OREF_NULL)   /* have a condition object?          */
        result = conditionobj->copy(); /* just return a copy of this        */
      else
        result = TheNilObject;         /* return the NIL object             */
      break;

    case 'S':                          /* 'S'tate                           */
      if (conditionobj != OREF_NULL)   /* have a condition object?          */
                                       /* get the current trap state        */
        result = context->trapState((RexxString *)conditionobj->at(OREF_CONDITION));
      break;

    default:                           /* unknown option                    */
                                       /* report an error                   */
      report_exception4(Error_Incorrect_call_list, new_cstring(CHAR_CONDITION), IntegerOne, new_string("ACDIOS", 6), option);
      break;
  }
  return result;                       /* return requested value            */
}

#define CHANGESTR_MIN 3
#define CHANGESTR_MAX 4
#define CHANGESTR_needle     1
#define CHANGESTR_haystack   2
#define CHANGESTR_newneedle  3
#define CHANGESTR_count      4

BUILTIN(CHANGESTR) {
  RexxString *needle;                  /* needle to change                  */
  RexxString *haystack;                /* target haystack                   */
  RexxString *newneedle;               /* new, replacement string           */
  RexxInteger *count;                  // max replacement count

  fix_args(CHANGESTR);                 /* check on require number of args   */
                                       /* get string for new                */
  needle = required_string(CHANGESTR, needle);
                                       /* get string for target             */
  haystack = required_string(CHANGESTR, haystack);
                                       /* get string to change to           */
  newneedle = required_string(CHANGESTR, newneedle);
                                       /* length is optional                */
  count = optional_integer(CHANGESTR, count);
                                       /* go perform the pos function       */
  return haystack->changeStr(needle, newneedle, count);
}

#define COUNTSTR_MIN 2
#define COUNTSTR_MAX 2
#define COUNTSTR_needle     1
#define COUNTSTR_haystack   2

BUILTIN(COUNTSTR) {
  RexxString *needle;                  /* needle to change                  */
  RexxString *haystack;                /* target haystack                   */
  LONG        count;                   /* returned count                    */

  fix_args(COUNTSTR);                  /* check on require number of args   */
                                       /* get string for new                */
  needle = required_string(COUNTSTR, needle);
                                       /* get string for target             */
  haystack = required_string(COUNTSTR, haystack);
  count = haystack->countStr(needle);  /* go perform the countstr function  */
  return new_integer(count);           /* return the new count              */
}

#define DBADJUST_MIN 1
#define DBADJUST_MAX 2
#define DBADJUST_string  1
#define DBADJUST_option  2

BUILTIN(DBADJUST) {
  RexxString  *string;                 /* target string                     */
  RexxString  *option;                 /* function option                   */

  fix_args(DBADJUST);                  /* check on required number of args  */
                                       /* must have a string                */
  string = required_string(DBADJUST, string);
                                       /* force option to a string          */
  option = optional_string(DBADJUST, option);
  return string->dbAdjust(option);     /* call the adjust function          */
}

#define DBBRACKET_MIN 1
#define DBBRACKET_MAX 1
#define DBBRACKET_string  1

BUILTIN(DBBRACKET) {
  RexxString  *string;                 /* target string                     */

  fix_args(DBBRACKET);                 /* check on required number of args  */
                                       /* must have a string                */
  string = required_string(DBBRACKET, string);
  return string->dbBracket();          /* get the length                    */
}

#define DBUNBRACKET_MIN 1
#define DBUNBRACKET_MAX 1
#define DBUNBRACKET_string  1

BUILTIN(DBUNBRACKET) {
  RexxString  *string;                 /* target string                     */

  fix_args(DBUNBRACKET);               /* check on required number of args  */
                                       /* must have a string                */
  string = required_string(DBUNBRACKET, string);
  return string->dbUnBracket();        /* get the length                    */
}

#define DBTODBCS_MIN 1
#define DBTODBCS_MAX 1
#define DBTODBCS_string  1

BUILTIN(DBTODBCS) {
  RexxString  *string;                 /* target string                     */

  fix_args(DBTODBCS);                  /* check on required number of args  */
                                       /* must have a string                */
  string = required_string(DBTODBCS, string);
  return string->dbToDbcs();           /* do the string conversion          */
}

#define DBTOSBCS_MIN 1
#define DBTOSBCS_MAX 1
#define DBTOSBCS_string  1

BUILTIN(DBTOSBCS) {
  RexxString  *string;                 /* target string                     */

  fix_args(DBTOSBCS);                  /* check on required number of args  */
                                       /* must have a string                */
  string = required_string(DBTOSBCS, string);
  return string->dbToSbcs();           /* do the string conversion          */
}

#define DBVALIDATE_MIN 1
#define DBVALIDATE_MAX 2
#define DBVALIDATE_string  1
#define DBVALIDATE_option  2

BUILTIN(DBVALIDATE) {
  RexxString  *string;                 /* target string                     */
  RexxString  *option;                 /* function option                   */

  fix_args(DBVALIDATE);                /* check on required number of args  */
                                       /* must have a string                */
  string = required_string(DBVALIDATE, string);
                                       /* force option to a string          */
  option = optional_string(DBVALIDATE, option);
  return string->dbValidate(option);   /* validate the string               */
}

#define DBWIDTH_MIN 1
#define DBWIDTH_MAX 2
#define DBWIDTH_string  1
#define DBWIDTH_option  2

BUILTIN(DBWIDTH) {
  RexxString  *string;                 /* target string                     */
  RexxString  *option;                 /* function option                   */

  fix_args(DBWIDTH);                   /* check on required number of args  */
                                       /* must have a string                */
  string = required_string(DBWIDTH, string);
                                       /* force option to a string          */
  option = optional_string(DBWIDTH, option);
  return string->dbWidth(option);      /* validate the string               */
}

#define DBLEFT_MIN 2
#define DBLEFT_MAX 4
#define DBLEFT_string 1
#define DBLEFT_length 2
#define DBLEFT_pad    3
#define DBLEFT_option 4

BUILTIN(DBLEFT) {
  RexxString  *string;                 /* target string                     */
  RexxInteger *length;                 /* target string length              */
  RexxString  *pad;                    /* optional pad character            */
  RexxString  *option;                 /* function option                   */

  fix_args(DBLEFT);                    /* check on required number of args  */
                                       /* must have the first argument      */
  string = required_string(DBLEFT, string);
                                       /* length must be an integer         */
  length = required_integer(DBLEFT, length);
  pad = optional_string(DBLEFT, pad);  /* force the pad to a string         */
                                       /* force option to a string          */
  option = optional_string(DBLEFT, option);
                                       /* do the left function              */
  return string->dbLeft(length, pad, option);
}

#define DBRIGHT_MIN 2
#define DBRIGHT_MAX 4
#define DBRIGHT_string 1
#define DBRIGHT_length 2
#define DBRIGHT_pad    3
#define DBRIGHT_option 4

BUILTIN(DBRIGHT) {
  RexxString  *string;                 /* target string                     */
  RexxInteger *length;                 /* target string length              */
  RexxString  *pad;                    /* optional pad character            */
  RexxString  *option;                 /* function option                   */

  fix_args(DBRIGHT);                   /* check on required number of args  */
                                       /* must have the first argument      */
  string = required_string(DBRIGHT, string);
                                       /* length must be an integer         */
  length = required_integer(DBRIGHT, length);
  pad = optional_string(DBRIGHT, pad); /* force the pad to a string         */
                                       /* force option to a string          */
  option = optional_string(DBRIGHT, option);
                                       /* do the right function             */
  return string->dbRight(length, pad, option);
}

#define DBCENTER_MIN 2
#define DBCENTER_MAX 4
#define DBCENTER_string 1
#define DBCENTER_length 2
#define DBCENTER_pad    3
#define DBCENTER_option 4

BUILTIN(DBCENTER) {
  RexxString  *string;                 /* target string                     */
  RexxInteger *length;                 /* target string length              */
  RexxString  *pad;                    /* optional pad character            */
  RexxString  *option;                 /* function option                   */

  fix_args(DBCENTER);                  /* check on required number of args  */
                                       /* must have the first argument      */
  string = required_string(DBCENTER, string);
                                       /* length must be an integer         */
  length = required_integer(DBCENTER, length);
  pad = optional_string(DBCENTER, pad);/* force the pad to a string         */
                                       /* force option to a string          */
  option = optional_string(DBCENTER, option);
                                       /* do the right function             */
  return string->dbCenter(length, pad, option);
}

#define DBRLEFT_MIN 2
#define DBRLEFT_MAX 3
#define DBRLEFT_string 1
#define DBRLEFT_length 2
#define DBRLEFT_option 3

BUILTIN(DBRLEFT) {
  RexxString  *string;                 /* target string                     */
  RexxInteger *length;                 /* target string length              */
  RexxString  *option;                 /* function option                   */

  fix_args(DBRLEFT);                   /* check on required number of args  */
                                       /* must have the first argument      */
  string = required_string(DBRLEFT, string);
                                       /* length must be an integer         */
  length = required_integer(DBRLEFT, length);
                                       /* force option to a string          */
  option = optional_string(DBRLEFT, option);
                                       /* do the left function              */
  return string->dbRleft(length, option);
}

#define DBRRIGHT_MIN 2
#define DBRRIGHT_MAX 3
#define DBRRIGHT_string 1
#define DBRRIGHT_length 2
#define DBRRIGHT_option 3

BUILTIN(DBRRIGHT) {
  RexxString  *string;                 /* target string                     */
  RexxInteger *length;                 /* target string length              */
  RexxString  *option;                 /* function option                   */

  fix_args(DBRRIGHT);                  /* check on required number of args  */
                                       /* must have the first argument      */
  string = required_string(DBRRIGHT, string);
                                       /* length must be an integer         */
  length = required_integer(DBRRIGHT, length);
                                       /* force option to a string          */
  option = optional_string(DBRRIGHT, option);
                                       /* do the right function             */
  return string->dbRright(length, option);
}

                                       /* the following builtin function    */
                                       /* table must maintain the same order*/
                                       /* as the builtin function codes used*/
                                       /* in the token class builtin        */
                                       /* builtin function lookup           */
pbuiltin builtin_table[] = {
  NULL,                                /* NULL first entry as dummy         */
  &builtin_function_ABBREV           ,
  &builtin_function_ABS              ,
  &builtin_function_ADDRESS          ,
  &builtin_function_ARG              ,
  &builtin_function_B2X              ,
  &builtin_function_BITAND           ,
  &builtin_function_BITOR            ,
  &builtin_function_BITXOR           ,
  &builtin_function_C2D              ,
  &builtin_function_C2X              ,
  &builtin_function_CENTER           ,
  &builtin_function_CENTRE           ,
  &builtin_function_CHANGESTR        ,
  &builtin_function_CHARIN           ,
  &builtin_function_CHAROUT          ,
  &builtin_function_CHARS            ,
  &builtin_function_COMPARE          ,
  &builtin_function_CONDITION        ,
  &builtin_function_COPIES           ,
  &builtin_function_COUNTSTR         ,
  &builtin_function_D2C              ,
  &builtin_function_D2X              ,
  &builtin_function_DATATYPE         ,
  &builtin_function_DATE             ,
  &builtin_function_DBADJUST         ,
  &builtin_function_DBBRACKET        ,
  &builtin_function_DBCENTER         ,
  &builtin_function_DBLEFT           ,
  &builtin_function_DBRIGHT          ,
  &builtin_function_DBRLEFT          ,
  &builtin_function_DBRRIGHT         ,
  &builtin_function_DBTODBCS         ,
  &builtin_function_DBTOSBCS         ,
  &builtin_function_DBUNBRACKET      ,
  &builtin_function_DBVALIDATE       ,
  &builtin_function_DBWIDTH          ,
  &builtin_function_DELSTR           ,
  &builtin_function_DELWORD          ,
  &builtin_function_DIGITS           ,
  &builtin_function_ERRORTEXT        ,
  &builtin_function_FORM             ,
  &builtin_function_FORMAT           ,
  &builtin_function_FUZZ             ,
  &builtin_function_INSERT           ,
  &builtin_function_LASTPOS          ,
  &builtin_function_LEFT             ,
  &builtin_function_LENGTH           ,
  &builtin_function_LINEIN           ,
  &builtin_function_LINEOUT          ,
  &builtin_function_LINES            ,
  &builtin_function_MAX              ,
  &builtin_function_MIN              ,
  &builtin_function_OVERLAY          ,
  &builtin_function_POS              ,
  &builtin_function_QUEUED           ,
  &builtin_function_RANDOM           ,
  &builtin_function_REVERSE          ,
  &builtin_function_RIGHT            ,
  &builtin_function_SIGN             ,
  &builtin_function_SOURCELINE       ,
  &builtin_function_SPACE            ,
  &builtin_function_STREAM           ,
  &builtin_function_STRIP            ,
  &builtin_function_SUBSTR           ,
  &builtin_function_SUBWORD          ,
  &builtin_function_SYMBOL           ,
  &builtin_function_TIME             ,
  &builtin_function_TRACE            ,
  &builtin_function_TRANSLATE        ,
  &builtin_function_TRUNC            ,
  &builtin_function_VALUE            ,
  &builtin_function_VAR              ,
  &builtin_function_VERIFY           ,
  &builtin_function_WORD             ,
  &builtin_function_WORDINDEX        ,
  &builtin_function_WORDLENGTH       ,
  &builtin_function_WORDPOS          ,
  &builtin_function_WORDS            ,
  &builtin_function_X2B              ,
  &builtin_function_X2C              ,
  &builtin_function_X2D              ,
  &builtin_function_XRANGE           ,
  &builtin_function_USERID           ,
  &builtin_function_LOWER            ,
  &builtin_function_UPPER            ,
};

