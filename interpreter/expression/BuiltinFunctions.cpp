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
/* REXX Translator                                                            */
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
#include "ProtectedObject.hpp"
#include "PackageManager.hpp"
#include "SystemInterpreter.hpp"
#include "SysFileSystem.hpp"


/* checks if pad is a single character string */
void checkPadArgument(const char *pFuncName, RexxObject *position, RexxString *pad)
{
    if (pad == OREF_NULL)
    {
        return;
    }
    if (pad->getLength() != 1)
    {
        reportException(Error_Incorrect_call_pad, pFuncName, position, pad);
    }
}

#define CENTER_MIN 2
#define CENTER_MAX 3
#define CENTER_string 1
#define CENTER_length 2
#define CENTER_pad    3

BUILTIN(CENTER)
{
    fix_args(CENTER);                    /* check on required number of args  */
                                         /* force first argument to a string  */
    RexxString *string = required_string(CENTER, string);
    /* this is a required length         */
    RexxInteger *length = required_integer(CENTER, length);
    RexxString *pad = optional_string(CENTER, pad);  /* the pad character must be one too */
    checkPadArgument(CHAR_CENTER, IntegerThree, pad);
    return string->center(length, pad);  /* do the center function            */
}

#define CENTRE_MIN 2
#define CENTRE_MAX 3
#define CENTRE_string 1
#define CENTRE_length 2
#define CENTRE_pad    3

BUILTIN(CENTRE)
{
    fix_args(CENTRE);                    /* check on required number of args  */
                                         /* force first argument to a string  */
    RexxString *string = required_string(CENTRE, string);
    /* this is a required length         */
    RexxInteger *length = required_integer(CENTRE, length);
    RexxString *pad = optional_string(CENTRE, pad);  /* the pad character must be one too */
    checkPadArgument(CHAR_CENTRE, IntegerThree, pad);
    return string->center(length, pad);  /* do the center function            */
}

#define DELSTR_MIN 2
#define DELSTR_MAX 3
#define DELSTR_string 1
#define DELSTR_n      2
#define DELSTR_length 3

BUILTIN(DELSTR)
{
    fix_args(DELSTR);                    /* check on required number of args  */
                                         /* must have the first argument      */
    RexxString *string = required_string(DELSTR, string);
    RexxInteger *n = required_integer(DELSTR, n);     /* need a delete position            */
    /* length is optional                */
    RexxInteger *length = optional_integer(DELSTR, length);
    return string->delstr(n, length);    /* do the delstr function            */
}

#define DELWORD_MIN 2
#define DELWORD_MAX 3
#define DELWORD_string 1
#define DELWORD_n      2
#define DELWORD_length 3

BUILTIN(DELWORD)
{
    fix_args(DELWORD);                   /* check on required number of args  */
                                         /* must have the first argument      */
    RexxString *string = required_string(DELWORD, string);
    RexxInteger *n = required_integer(DELWORD, n);    /* need a delete position            */
    /* length is optional                */
    RexxInteger *length = optional_integer(DELWORD, length);
    return string->delWord(n, length);   /* do the delword function           */
}

#define INSERT_MIN 2
#define INSERT_MAX 5
#define INSERT_new    1
#define INSERT_target 2
#define INSERT_n      3
#define INSERT_length 4
#define INSERT_pad    5

BUILTIN(INSERT)
{
    fix_args(INSERT);                    /* check on require number of args   */
                                         /* get string for new                */
    RexxString *newString = required_string(INSERT, new);
    /* get string for target             */
    RexxString *target = required_string(INSERT, target);
    RexxInteger *n = optional_integer(INSERT, n);     /* insert position is optional       */
    /* length is optional                */
    RexxInteger *length = optional_integer(INSERT, length);
    RexxString *pad = optional_string(INSERT, pad);  /* get string for pad                */
    /* go perform the insert function    */
    checkPadArgument(CHAR_INSERT, IntegerFour, pad);
    return target->insert(newString, n, length, pad);
}

#define LEFT_MIN 2
#define LEFT_MAX 3
#define LEFT_string 1
#define LEFT_length 2
#define LEFT_pad    3

BUILTIN(LEFT)
{
    fix_args(LEFT);                      /* check on required number of args  */
                                         /* must have the first argument      */
    RexxString *string = required_string(LEFT, string);
    /* length is optional                */
    RexxInteger *length = optional_integer(LEFT, length);
    RexxString *pad = optional_string(LEFT, pad);    /* pad must be a string also         */
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

BUILTIN(OVERLAY)
{
    fix_args(OVERLAY);                   /* check on require number of args   */
                                         /* get string for new                */
    RexxString *newString = required_string(OVERLAY, new);
    /* get string for target             */
    RexxString *target = required_string(OVERLAY, target);
    RexxInteger *n = optional_integer(OVERLAY, n);    /* overlay position is optional      */
    /* length is optional                */
    RexxInteger *length = optional_integer(OVERLAY, length);
    RexxString *pad = optional_string(OVERLAY, pad); /* get string for pad                */
    /* go perform the overlay function   */
    checkPadArgument(CHAR_OVERLAY, IntegerFive, pad);
    return target->overlay(newString, n, length, pad);
}

#define POS_MIN 2
#define POS_MAX 4
#define POS_needle   1
#define POS_haystack 2
#define POS_start    3
#define POS_range    4

BUILTIN(POS)
{
    fix_args(POS);                       /* check on require number of args   */
                                         /* get string for new                */
    RexxString *needle = required_string(POS, needle);
    /* get string for target             */
    RexxString *haystack = required_string(POS, haystack);
    RexxInteger *start = optional_integer(POS, start);/* start position is optional        */
    RexxInteger *range = optional_integer(POS, range);
    /* go perform the pos function       */
    return haystack->posRexx(needle, start, range);
}

#define LASTPOS_MIN 2
#define LASTPOS_MAX 4
#define LASTPOS_needle   1
#define LASTPOS_haystack 2
#define LASTPOS_start    3
#define LASTPOS_range    4

BUILTIN(LASTPOS)
{
    fix_args(LASTPOS);                   /* check on require number of args   */
                                         /* get string for new                */
    RexxString *needle = required_string(LASTPOS, needle);
    /* get string for target             */
    RexxString *haystack = required_string(LASTPOS, haystack);
    /* start position is optional        */
    RexxInteger *start = optional_integer(LASTPOS, start);
    RexxInteger *range = optional_integer(LASTPOS, range);
    /* go perform the lastpos function   */
    return haystack->lastPosRexx(needle, start, range);
}

#define REVERSE_MIN 1
#define REVERSE_MAX 1
#define REVERSE_string 1

BUILTIN(REVERSE)
{
    fix_args(REVERSE);                   /* check on require number of args   */
                                         /* get string for string             */
    RexxString *string = required_string(REVERSE, string);
    return string->reverse();            /* go perform the reverse function   */
}

#define RIGHT_MIN 2
#define RIGHT_MAX 3
#define RIGHT_string 1
#define RIGHT_length 2
#define RIGHT_pad    3

BUILTIN(RIGHT)
{
    fix_args(RIGHT);                     /* check on required number of args  */
                                         /* must have the first argument      */
    RexxString *string = required_string(RIGHT, string);
    /* length is optional                */
    RexxInteger *length = optional_integer(RIGHT, length);
    RexxString *pad = optional_string(RIGHT, pad);   /* pad must be a string also         */
    checkPadArgument(CHAR_RIGHT, IntegerThree, pad);
    return string->right(length, pad);   /* do the substr function            */
}

#define STRIP_MIN 1
#define STRIP_MAX 3
#define STRIP_string 1
#define STRIP_option 2
#define STRIP_char   3

BUILTIN(STRIP)
{
    fix_args(STRIP);                     /* check on required number of args  */
                                         /* must have the first argument      */
    RexxString *string = required_string(STRIP, string);
    /* option must be a string too       */
    RexxString *option = optional_string(STRIP, option);
    /* as is char as well                */
    RexxString *character = optional_string(STRIP, char);
    /* do the strip function             */
    return string->strip(option, character);
}

#define SPACE_MIN 1
#define SPACE_MAX 3
#define SPACE_string 1
#define SPACE_n      2
#define SPACE_pad    3


BUILTIN(SPACE)
{
    fix_args(SPACE);                     /* check on required number of args  */
                                         /* must have the first argument      */
    RexxString *string = required_string(SPACE, string);
    RexxInteger *n = optional_integer(SPACE, n);      /* spacing is an optional integer    */
    RexxString *pad = optional_string(SPACE, pad);   /* pad must be a string also         */
    checkPadArgument(CHAR_SPACE, IntegerThree, pad);
    return string->space(n, pad);        /* do the space function             */
}

#define SUBSTR_MIN 2
#define SUBSTR_MAX 4
#define SUBSTR_string 1
#define SUBSTR_n      2
#define SUBSTR_length 3
#define SUBSTR_pad    4


BUILTIN(SUBSTR)
{
    fix_args(SUBSTR);                    /* check on required number of args  */
                                         /* must have the first argument      */
    RexxString *string = required_string(SUBSTR, string);
    RexxInteger *n = required_integer(SUBSTR, n);     /* position is required              */
    /* length is optional                */
    RexxInteger *length = optional_integer(SUBSTR, length);
    RexxString *pad = optional_string(SUBSTR, pad);  /* pad must be a string also         */
    /* do the substr function            */
    checkPadArgument(CHAR_SUBSTR, IntegerFour, pad);
    return string->substr(n, length, pad);
}


#define LOWER_MIN 1
#define LOWER_MAX 3
#define LOWER_string 1
#define LOWER_n      2
#define LOWER_length 3


BUILTIN(LOWER)
{
    fix_args(LOWER);                     /* check on required number of args  */
                                         /* must have the first argument      */
    RexxString *string = required_string(LOWER, string);
    RexxInteger *n = optional_integer(LOWER, n);      /* position is optional              */
    /* length is optional                */
    RexxInteger *length = optional_integer(LOWER, length);
    /* do the LOWER function            */
    return string->lowerRexx(n, length);
}


#define UPPER_MIN 1
#define UPPER_MAX 3
#define UPPER_string 1
#define UPPER_n      2
#define UPPER_length 3


BUILTIN(UPPER)
{
    fix_args(UPPER);                     /* check on required number of args  */
                                         /* must have the first argument      */
    RexxString *string = required_string(UPPER, string);
    RexxInteger *n = optional_integer(UPPER, n);       /* position is optional              */
    /* length is optional                */
    RexxInteger *length = optional_integer(UPPER, length);
    /* do the UPPER function            */
    return string->upperRexx(n, length);
}


#define SUBWORD_MIN 2
#define SUBWORD_MAX 3
#define SUBWORD_string 1
#define SUBWORD_n      2
#define SUBWORD_length 3

BUILTIN(SUBWORD)
{
    fix_args(SUBWORD);                   /* check on required number of args  */
                                         /* must have the first argument      */
    RexxString *string = required_string(SUBWORD, string);
    RexxInteger *n = required_integer(SUBWORD, n);    /* position is required              */
    /* length is optional                */
    RexxInteger *length = optional_integer(SUBWORD, length);
    return string->subWord(n, length);   /* do the subword function           */
}

#define WORD_MIN 2
#define WORD_MAX 2
#define WORD_string 1
#define WORD_n      2

BUILTIN(WORD)
{
    fix_args(WORD);                      /* check on required number of args  */
                                         /* must have the first argument      */
    RexxString *string = required_string(WORD, string);
    RexxInteger *n = required_integer(WORD, n);       /* position is required              */
    return string->word(n);              /* do the word function              */
}

#define WORDINDEX_MIN 2
#define WORDINDEX_MAX 2
#define WORDINDEX_string 1
#define WORDINDEX_n      2

BUILTIN(WORDINDEX)
{
    fix_args(WORDINDEX);                 /* check on required number of args  */
                                         /* must have the first argument      */
    RexxString *string = required_string(WORDINDEX, string);
    RexxInteger *n = required_integer(WORDINDEX, n);  /* position is required              */
    return string->wordIndex(n);         /* do the wordindex function         */
}

#define WORDLENGTH_MIN 2
#define WORDLENGTH_MAX 2
#define WORDLENGTH_string 1
#define WORDLENGTH_n      2

BUILTIN(WORDLENGTH)
{
    fix_args(WORDLENGTH);                /* check on required number of args  */
                                         /* must have the first argument      */
    RexxString *string = required_string(WORDLENGTH, string);
    RexxInteger *n = required_integer(WORDLENGTH, n); /* position is required              */
    return string->wordLength(n);        /* do the wordlength function        */
}

#define COPIES_MIN 2
#define COPIES_MAX 2
#define COPIES_string 1
#define COPIES_n      2

BUILTIN(COPIES)
{
    fix_args(COPIES);                    /* check on required number of args  */
                                         /* must have the first argument      */
    RexxString *string = required_string(COPIES, string);
    RexxInteger *n = required_integer(COPIES, n);     /* position is required              */
    return string->copies(n);            /* do the copies function            */
}

#define WORDPOS_MIN 2
#define WORDPOS_MAX 3
#define WORDPOS_phrase 1
#define WORDPOS_string 2
#define WORDPOS_start  3

BUILTIN(WORDPOS)
{
    fix_args(WORDPOS);                   /* check on required number of args  */
                                         /* must have a phrase string         */
    RexxString *phrase = required_string(WORDPOS, phrase);
    /* must have the string argument     */
    RexxString *string = required_string(WORDPOS, string);
    /* start position is optional        */
    RexxInteger *start = optional_integer(WORDPOS, start);
    /* do the wordpos function           */
    return string->wordPos(phrase, start);
}

#define WORDS_MIN 1
#define WORDS_MAX 1
#define WORDS_string 1

BUILTIN(WORDS)
{
    fix_args(WORDS);                     /* check on required number of args  */
                                         /* must have the string argument     */
    RexxString *string = required_string(WORDS, string);
    return string->words();              /* do the words function             */
}

#define ABBREV_MIN 2
#define ABBREV_MAX 3
#define ABBREV_information 1
#define ABBREV_info        2
#define ABBREV_length      3

BUILTIN(ABBREV)
{
    fix_args(ABBREV);                    /* check on required number of args  */
                                         /* information must be a string arg  */
    RexxString *information = required_string(ABBREV, information);
    RexxString *info = required_string(ABBREV, info);/* info must also be a string        */
    /* length is optional                */
    RexxInteger *length = optional_integer(ABBREV, length);
    /* check on the abbreviation         */
    return information->abbrev(info, length);
}

#define BITAND_MIN 1
#define BITAND_MAX 3
#define BITAND_string1 1
#define BITAND_string2 2
#define BITAND_pad     3

BUILTIN(BITAND)
{
    fix_args(BITAND);                    /* check on required number of args  */
                                         /* must have the first string        */
    RexxString *string1 = required_string(BITAND, string1);
    /* second string is optional         */
    RexxString *string2 = optional_string(BITAND, string2);
    RexxString *pad = optional_string(BITAND, pad);  /* pad is optional also              */
    checkPadArgument(CHAR_BITAND, IntegerThree, pad);
    return string1->bitAnd(string2, pad);/* do the bitand function            */
}

#define BITOR_MIN 1
#define BITOR_MAX 3
#define BITOR_string1 1
#define BITOR_string2 2
#define BITOR_pad     3

BUILTIN(BITOR)
{
    fix_args(BITOR);                     /* check on required number of args  */
                                         /* must have the first string        */
    RexxString *string1 = required_string(BITOR, string1);
    /* second string is optional         */
    RexxString *string2 = optional_string(BITOR, string2);
    RexxString *pad = optional_string(BITOR, pad);   /* pad is optional also              */
    checkPadArgument(CHAR_BITOR, IntegerThree, pad);
    return string1->bitOr(string2, pad); /* do the bitor function             */
}

#define BITXOR_MIN 1
#define BITXOR_MAX 3
#define BITXOR_string1 1
#define BITXOR_string2 2
#define BITXOR_pad     3

BUILTIN(BITXOR)
{
    fix_args(BITXOR);                    /* check on required number of args  */
                                         /* must have the first string        */
    RexxString *string1 = required_string(BITXOR, string1);
    /* second string is optional         */
    RexxString *string2 = optional_string(BITXOR, string2);
    RexxString *pad = optional_string(BITXOR, pad);  /* pad is optional also              */
    checkPadArgument(CHAR_BITXOR, IntegerThree, pad);
    return string1->bitXor(string2, pad);/* do the bitxor function            */
}

#define B2X_MIN 1
#define B2X_MAX 1
#define B2X_string  1

BUILTIN(B2X)
{
    fix_args(B2X);                       /* check on required number of args  */
                                         /* must have the first string        */
    RexxString *string = required_string(B2X, string);
    return string->b2x();                /* do the b2x function               */
}

#define X2B_MIN 1
#define X2B_MAX 1
#define X2B_string  1

BUILTIN(X2B)
{
    fix_args(X2B);                       /* check on required number of args  */
                                         /* must have the first string        */
    RexxString *string = required_string(X2B, string);
    return string->x2b();                /* do the x2b function               */
}

#define C2X_MIN 1
#define C2X_MAX 1
#define C2X_string  1

BUILTIN(C2X)
{
    fix_args(C2X);                       /* check on required number of args  */
                                         /* must have the first string        */
    RexxString *string = required_string(C2X, string);
    return string->c2x();                /* do the c2x function               */
}

#define X2C_MIN 1
#define X2C_MAX 1
#define X2C_string  1

BUILTIN(X2C)
{
    fix_args(X2C);                       /* check on required number of args  */
                                         /* must have the first string        */
    RexxString *string = required_string(X2C, string);
    return string->x2c();                /* do the x2c function               */
}

#define C2D_MIN 1
#define C2D_MAX 2
#define C2D_string  1
#define C2D_n       2

BUILTIN(C2D)
{
    fix_args(C2D);                       /* check on required number of args  */
                                         /* must have the first string        */
    RexxString *string = required_string(C2D, string);
    RexxInteger *n = optional_integer(C2D, n);        /* length is optional                */
    return string->c2d(n);               /* do the c2d function               */
}

#define TRUNC_MIN 1
#define TRUNC_MAX 2
#define TRUNC_number  1
#define TRUNC_n       2

BUILTIN(TRUNC)
{
    fix_args(TRUNC);                     /* check on required number of args  */
                                         /* must have the first string        */
    RexxString *number = required_string(TRUNC, number);
    RexxInteger *n = optional_integer(TRUNC, n);      /* length is optional                */
    return number->trunc(n);             /* do the trunc function             */
}

#define X2D_MIN 1
#define X2D_MAX 2
#define X2D_string  1
#define X2D_n       2

BUILTIN(X2D)
{
    fix_args(X2D);                       /* check on required number of args  */
                                         /* must have the first string        */
    RexxString *string = required_string(X2D, string);
    RexxInteger *n = optional_integer(X2D, n);        /* length is optional                */
    return string->x2d(n);               /* do the x2d function               */
}

#define D2X_MIN 1
#define D2X_MAX 2
#define D2X_string  1
#define D2X_n       2

BUILTIN(D2X)
{
    fix_args(D2X);                       /* check on required number of args  */
                                         /* must have the first string        */
    RexxString *string = required_string(D2X, string);
    RexxInteger *n = optional_integer(D2X, n);        /* length is optional                */
    return string->d2x(n);               /* do the x2d function               */
}

#define D2C_MIN 1
#define D2C_MAX 2
#define D2C_string  1
#define D2C_n       2

BUILTIN(D2C)
{
    fix_args(D2C);                       /* check on required number of args  */
                                         /* must have the first string        */
    RexxString *string = required_string(D2C, string);
    RexxInteger *n = optional_integer(D2C, n);        /* length is optional                */
    return string->d2c(n);               /* do the x2d function               */
}

#define COMPARE_MIN 2
#define COMPARE_MAX 3
#define COMPARE_string1 1
#define COMPARE_string2 2
#define COMPARE_pad     3

BUILTIN(COMPARE)
{
    fix_args(COMPARE);                   /* check on required number of args  */
                                         /* must have the first string        */
    RexxString *string1 = required_string(COMPARE, string1);
    /* and the second string also        */
    RexxString *string2 = required_string(COMPARE, string2);
    RexxString *pad = optional_string(COMPARE, pad); /* padding is optional               */
                                         /* do the comparison                 */
    checkPadArgument(CHAR_COMPARE, IntegerThree, pad);
    return string1->compare(string2, pad);
}

#define LENGTH_MIN 1
#define LENGTH_MAX 1
#define LENGTH_string  1

BUILTIN(LENGTH)
{
    fix_args(LENGTH);                    /* check on required number of args  */
                                         /* must have a string                */
    RexxString *target = required_string(LENGTH, string);
    return target->lengthRexx();         /* get the length                    */
}

#define TRANSLATE_MIN 1
#define TRANSLATE_MAX 6
#define TRANSLATE_string  1
#define TRANSLATE_tableo  2
#define TRANSLATE_tablei  3
#define TRANSLATE_pad     4
#define TRANSLATE_start   5
#define TRANSLATE_range   6

BUILTIN(TRANSLATE)
{
    fix_args(TRANSLATE);                 /* check on required number of args  */
                                         /* must have a string                */
    RexxString *string = required_string(TRANSLATE, string);
    /* output table is optional          */
    RexxString *tableo = optional_string(TRANSLATE, tableo);
    /* input table is optional           */
    RexxString *tablei = optional_string(TRANSLATE, tablei);
    /* pad is also optional              */
    RexxString *pad = optional_string(TRANSLATE, pad);
    /* perform the translate             */
    checkPadArgument(CHAR_TRANSLATE, IntegerFour, pad);
    RexxInteger *start = optional_integer(TRANSLATE, start);
    RexxInteger *range = optional_integer(TRANSLATE, range);
    return string->translate(tableo, tablei, pad, start, range);
}

#define VERIFY_MIN 2
#define VERIFY_MAX 5
#define VERIFY_string    1
#define VERIFY_reference 2
#define VERIFY_option    3
#define VERIFY_start     4
#define VERIFY_range     5

BUILTIN(VERIFY)
{
    fix_args(VERIFY);                    /* check on required number of args  */
                                         /* must have a string                */
    RexxString *string = required_string(VERIFY, string);
    /* reference is also required        */
    RexxString *reference = required_string(VERIFY, reference);
    /* the options are optional          */
    RexxString *option = optional_string(VERIFY, option);
    /* start is optional                 */
    RexxInteger *start = optional_integer(VERIFY, start);
    /* start is optional                 */
    RexxInteger *range = optional_integer(VERIFY, range);
    /* do the verify function            */
    return string->verify(reference, option, start, range);
}

#define DATATYPE_MIN 1
#define DATATYPE_MAX 2
#define DATATYPE_string    1
#define DATATYPE_type      2

BUILTIN(DATATYPE)
{
    fix_args(DATATYPE);                  /* check on required number of args  */
                                         /* must have a string                */
    RexxString *string = required_string(DATATYPE, string);
    /* type must also be a string        */
    RexxString *type = optional_string(DATATYPE, type);
    return string->dataType(type);       /* call the datatype method          */
}

#define ADDRESS_MIN 0
#define ADDRESS_MAX 0

BUILTIN(ADDRESS)
{
    check_args(ADDRESS);                 /* check on required number of args  */
    return context->getAddress();        /* return the current address setting*/
}

#define DIGITS_MIN 0
#define DIGITS_MAX 0

BUILTIN(DIGITS)
{
    check_args(DIGITS);                  /* check on required number of args  */
    return new_integer(context->digits());   /* return as an option               */
}

#define FUZZ_MIN 0
#define FUZZ_MAX 0

BUILTIN(FUZZ)
{
    check_args(FUZZ);                    /* check on required number of args  */
    return new_integer(context->fuzz()); /* return as an integer object       */
}

#define FORM_MIN 0
#define FORM_MAX 0

BUILTIN(FORM)
{
    check_args(FORM);                    /* check on required number of args  */
                                         /* return the current form setting   */
    return context->form() == Numerics::FORM_SCIENTIFIC ? OREF_SCIENTIFIC : OREF_ENGINEERING;
}

#define USERID_MIN 0
#define USERID_MAX 0

BUILTIN(USERID)
{
    check_args(USERID);
    return SystemInterpreter::getUserid();
}

#define ERRORTEXT_MIN 1
#define ERRORTEXT_MAX 1
#define ERRORTEXT_n   1

BUILTIN(ERRORTEXT)
{
    check_args(ERRORTEXT);               /* check on required number of args  */
                                         /* get the error number              */
    wholenumber_t error_number = (required_integer(ERRORTEXT, n))->getValue();
    /* outside allowed range?            */
    if (error_number < 0 || error_number > 99)
    {
        /* this is an error                  */
        reportException(Error_Incorrect_call_range, CHAR_ERRORTEXT, IntegerOne, error_number);
    }
    /* retrieve the major error message  */
    RexxString *result = SystemInterpreter::getMessageText(error_number * 1000);
    if (result == OREF_NULL)             /* not found?                        */
    {
        result = OREF_NULLSTRING;          /* this is a null string result      */
    }
    return result;                       /* finished                          */
}

#define ARG_MIN 0
#undef ARG_MAX                      /* In AIX already defined            */
#define ARG_MAX 2
#define ARG_n      1
#define ARG_option 2

BUILTIN(ARG)
{
    fix_args(ARG);                       /* expand arguments to full value    */
    RexxInteger *n = optional_integer(ARG, n);        /* get the position info             */
                                         /* get the option string             */
    RexxString *option = optional_string(ARG, option);
    /* get the argument array            */
    RexxObject **arglist = context->getMethodArgumentList();
    size_t size = context->getMethodArgumentCount();
    /* have an option but no position?   */
    if (n == OREF_NULL)
    {                /* no position specified?            */
        if (option != OREF_NULL)           /* have an option with no position   */
        {
                                           /* raise an error                    */
            reportException(Error_Incorrect_call_noarg, CHAR_ARG, IntegerOne);
        }
        /* return the count as an object */
        return new_integer(size);
    }
    else if (option == OREF_NULL)
    {      /* just looking for a specific arg?  */
        size_t position = n->getValue();   /* get the integer value             */
                                           /* must be a positive integer        */
        positive_integer(position, ARG, IntegerOne);
        /* bigger than argument list size?   */
        if (size < position)
        {
            return OREF_NULLSTRING;          /* just return a null string         */
        }
        else
        {
            RexxObject *result = arglist[position - 1];  /* get actual value from arglist     */
            if (result == OREF_NULL)         /* argument wasn't there?            */
            {
                return OREF_NULLSTRING;      /* this too is a null string         */
            }
            return result;                   // return the argument stuff
        }
    }
    else
    {                               /* need to process an option         */
        size_t position = n->getValue();   /* get the integer value             */
                                           /* must be a positive integer        */
        positive_integer(position, ARG, IntegerOne);

        switch (option->getChar(0))
        {      /* process the option character      */

            case 'A':                        /* return argument array             */
            case 'a':                        /* return argument array             */
                if (position == 1)
                {           /* want it all?                      */
                    /* create an array result for the return */
                    return new (size, arglist) RexxArray;
                }
                else if (position > size)      /* beyond bounds of argument list?   */
                {
                                               /* this is a zero size array         */
                    return TheNullArray->copy();
                }
                else
                {                         /* need to extract a sub array       */
                    return new (size - position + 1, &arglist[position - 1]) RexxArray;
                }
                break;

            case 'E':                        /* argument 'E'xist?                 */
            case 'e':                        /* argument 'E'xist?                 */
                if (position > size)           /* too big for argument list?        */
                {
                    return TheFalseObject;     /* can't be true                     */
                }
                                                 /* have a real argument?             */
                else if (arglist[position - 1] == OREF_NULL)
                {
                    return TheFalseObject;     /* nope, this is false also          */
                }
                else
                {
                    return TheTrueObject;      /* have a real argument              */
                }
                break;

            case 'O':                        /* argument 'O'mitted?               */
            case 'o':                        /* argument 'O'mitted?               */
                if (position > size)           /* too big for argument list?        */
                {
                    return TheTrueObject;      /* must be omitted                   */
                }
                                                 /* have a real argument?             */
                else if (arglist[position - 1] == OREF_NULL)
                {
                    return TheTrueObject;        /* this is omitted also              */
                }
                else
                {
                    return TheFalseObject;     /* have a real argument              */
                }
                break;

            case 'N':                        /* 'N'ormal processing?              */
            case 'n':                        /* 'N'ormal processing?              */
                if (position > size)           /* bigger than argument list size?   */
                {
                    return OREF_NULLSTRING;    /* just return a null string         */
                }
                else
                {                         /* get actual value from arglist     */
                    RexxObject *result = arglist[position - 1];
                    if (result == OREF_NULL)     /* argument wasn't there?            */
                    {
                        return OREF_NULLSTRING;  /* this too is a null string         */
                    }
                    return result;
                }
                break;

            default:                         /* unknown option                    */
                /* this is an error                  */
                reportException(Error_Incorrect_call_list, CHAR_ARG, IntegerTwo, "AENO", option);
                break;
        }
    }
    return OREF_NULLSTRING;              // should never happen
}


#define DATE_MIN 0
#define DATE_MAX 5
#define DATE_option  1
#define DATE_indate  2
#define DATE_option2 3
#define DATE_osep    4
#define DATE_isep    5

BUILTIN(DATE)
{
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
        if (option->getLength() == 0)        /* have a null string?               */
        {
            /* this is an error                  */
            reportException(Error_Incorrect_call_list, CHAR_DATE, IntegerOne, "BDEFLMNOSTUW", option);
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
        reportException(Error_Incorrect_call_noarg, CHAR_DATE, IntegerTwo);
    }

    if (option2 != OREF_NULL)            /* just using default format?        */
    {
        if (option2->getLength() == 0)       /* have a null string?               */
        {
            /* this is an error                  */
            reportException(Error_Incorrect_call_list, CHAR_DATE, IntegerThree, "BDEFNOSTU", option2);
        }
        else                                 /* need to process an option         */
        {
            /* option is first character         */
            style2 = toupper(option2->getChar(0));
        }
    }

    const char *outputSeparator = NULL;      // each format has it's own default

    // validate the output separator is only used with supported styles
    if (osep != OREF_NULL)
    {
        // only certain styles support this option
        if (strchr("BDMWL", style) != NULL)
        {
            reportException(Error_Incorrect_call_format_incomp_sep, CHAR_DATE, IntegerOne, new_string((char)style), IntegerFour);
        }
        if (osep->getLength() > 1 || (osep->getLength() == 1 && strchr(ALPHANUM, osep->getChar(0)) != NULL))
        {
            reportException(Error_Incorrect_call_parm_wrong_sep, CHAR_DATE, IntegerFour, osep);
        }
        // string objects are null terminated, so we can point directly at what will
        // be either 1 or 0 characters of data.
        outputSeparator = osep->getStringData();
    }

    if (indate != OREF_NULL)             /* given a time stamp?               */
    {
        bool valid = true;                 /* assume have a good stamp          */

        const char *separator = NULL;      // different formats will override this
                                           /* begin addition                    */
        // if we have a separator, perform validation here
        if (isep != OREF_NULL)
        {
            if (strchr("BDMWL", style2) != NULL)
            {
                reportException(Error_Incorrect_call_format_incomp_sep, CHAR_DATE, IntegerThree, new_string((char *)&style2, 1), IntegerFive);
            }
            // explicitly specified delimiter, we need to validate this first
            if (isep->getLength() > 1 || (isep->getLength() == 1 && strchr(ALPHANUM, isep->getChar(0)) != NULL))
            {
                // the field delimiter must be a single character and NOT
                // alphanumeric, or a null character
                reportException(Error_Incorrect_call_parm_wrong_sep, new_string(CHAR_DATE), IntegerFive, isep);
            }
            // string objects are null terminated, so we can point directly at what will
            // be either 1 or 0 characters of data.
            separator = isep->getStringData();
        }

        /* clear the time stamp              */
        timestamp.clear();
        // everything is done using the current timezone offset
        timestamp.setTimeZoneOffset(current.getTimeZoneOffset());
        switch (style2)
        {                  /* convert to usable form per option2*/

            case 'N':                        /* 'N'ormal: default style           */
                valid = timestamp.parseNormalDate(indate->getStringData(), separator);
                break;

            case 'B':                        /* 'B'asedate                        */
                {
                    /*convert the value                  */
                    wholenumber_t basedays;
                    if (!indate->numberValue(basedays) || !timestamp.setBaseDate(basedays))
                    {
                        reportException(Error_Incorrect_call_format_invalid, CHAR_DATE, indate, new_string((char *)&style2, 1));
                    }
                    break;
                }

            case 'F':                        /* 'F'ull datetime stamp            */
                {
                    /*convert the value                  */
                    int64_t basetime;
                    if (!Numerics::objectToInt64(indate, basetime) || !timestamp.setBaseTime(basetime))
                    {
                        reportException(Error_Incorrect_call_format_invalid, CHAR_DATE, indate, new_string((char *)&style2, 1));
                    }
                    break;
                }

            case 'T':                        /* 'T'icks datetime stamp            */
                {
                    /*convert the value                  */
                    int64_t basetime;
                    if (!Numerics::objectToInt64(indate, basetime) || !timestamp.setUnixTime(basetime))
                    {
                        reportException(Error_Incorrect_call_format_invalid, CHAR_DATE, indate, new_string((char *)&style2, 1));
                    }
                    break;
                }

            case 'D':                        /* 'D'ay of year                     */
                {
                    /*convert the value                  */
                    wholenumber_t yearday;
                    if (!indate->numberValue(yearday) || yearday < 0 || yearday > YEAR_DAYS + 1 ||
                        (yearday > YEAR_DAYS && !LeapYear(current.year)))
                    {
                        reportException(Error_Incorrect_call_format_invalid, CHAR_DATE, indate, new_string((char *)&style2, 1));
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
                reportException(Error_Incorrect_call_list, CHAR_DATE, IntegerThree, "BDEFNOTSU", new_string((char *)&style2, 1));
                break;
        }
        // if there's a formatting error
        if (!valid)
        {
            // different error message depending on whether a separator was specified, or not.
            if (isep != OREF_NULL)
            {
                reportException(Error_Incorrect_call_format_incomp_sep, CHAR_DATE, IntegerTwo, indate, IntegerFive);
            }
            else
            {
                reportException(Error_Incorrect_call_format_invalid, CHAR_DATE, indate, new_string((char *)&style2, 1));
            }
        }
    }
    else
    {
        // just copy the current time stamp
        timestamp = current;
    }

    wholenumber_t day = timestamp.day;          /* get various date parts            */
    wholenumber_t month = timestamp.month;
    wholenumber_t year = timestamp.year;

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
                RexxString *month_name = SystemInterpreter::getMessageText(Message_Translations_January + month - 1);
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
            reportException(Error_Incorrect_call_list, CHAR_DATE, IntegerOne, "BDEFLMNOSTUW", new_string(work, 1));
            break;
    }
    /* now create a string object        */
    return new_string(work);
}


#define TIME_MIN 0
#define TIME_MAX 3
#define TIME_option  1
#define TIME_intime  2
#define TIME_option2 3

BUILTIN(TIME)
{
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
        if (option->getLength() == 0)
        {
            reportException(Error_Incorrect_call_list, CHAR_TIME, IntegerOne, "CEFHLMNORST", option);
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
            reportException(Error_Incorrect_call_noarg, CHAR_TIME, IntegerTwo);
        }
        // again, must be at least one character, of which we only use the first
        if (option2->getLength() == 0)
        {
            reportException(Error_Incorrect_call_list, CHAR_TIME, IntegerThree, "CFHLMNOST", option2);
        }
        style2 = toupper(option2->getChar(0));
    }


    if (intime != OREF_NULL)
    {
        // the input timestamp is not valid with the elapsed time options, and
        // using an offset as an input isn't really meaningful
        if (style == 'R' || style == 'E')
        {
            reportException(Error_Incorrect_call_invalid_conversion, CHAR_TIME, new_string((char *)&style, 1));
        }
        bool valid = true;                 // assume this is a good timestamp
        timestamp.clear();                 // clear everything out
        // everything is done using the current timezone offset
        timestamp.setTimeZoneOffset(current.getTimeZoneOffset());

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
                    wholenumber_t i;
                    valid = intime->numberValue(i) && timestamp.setHours(i);
                    break;
                }

            case 'S':                        /* 'S'econds format                  */
                {
                    wholenumber_t i;
                    valid = intime->numberValue(i) && timestamp.setSeconds(i);
                    break;
                }

            case 'M':                        /* 'M'inutes format                  */
                {
                    wholenumber_t i;
                    valid = intime->numberValue(i) && timestamp.setMinutes(i);
                    break;
                }

            case 'F':                        /* 'F'ull datetime stamp            */
                {
                    /*convert the value                  */
                    int64_t basetime;
                    if (!Numerics::objectToInt64(intime, basetime) || !timestamp.setBaseTime(basetime))
                    {
                        reportException(Error_Incorrect_call_format_invalid, CHAR_TIME, intime, new_string((char *)&style2, 1));
                    }
                    break;
                }

            case 'T':                        /* 'T'icks datetime stamp            */
                {
                    /*convert the value                  */
                    int64_t basetime;
                    if (!Numerics::objectToInt64(intime, basetime) || !timestamp.setUnixTime(basetime))
                    {
                        reportException(Error_Incorrect_call_format_invalid, CHAR_TIME, intime, new_string((char *)&style2, 1));
                    }
                    break;
                }

            case 'O':                          // 'O'ffset.  microseconds offset from UTC
                {
                    // everything comes from the current time stamp, but we will adjust to the new offset
                    timestamp = current;                 // and by default we work off of that time
                    wholenumber_t i;
                    valid = intime->numberValue(i) && timestamp.adjustTimeZone(i);
                    break;

                }

            default:
                work[0] = style2;              /* copy over the character           */
                reportException(Error_Incorrect_call_list, CHAR_TIME, IntegerThree, "CFHLMNOST", new_string(work, 1));
                break;
        }
        if (!valid)                        /* not convert cleanly?              */
        {
            reportException(Error_Incorrect_call_format_invalid, CHAR_TIME, intime, new_string((char *)&style2, 1) );
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
                int64_t threshold = current.getUTCBaseTime() - startTime;
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
                    sprintf(work, "%d.%06d", (int)(threshold / (int64_t)MICROSECONDS), (int)(threshold % (int64_t)MICROSECONDS));
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

        case 'O':                          // 'O'ffset.  microseconds offset from UTC
            timestamp.formatTimeZone(work);
            break;

        default:                          /* unknown format                    */
            work[0] = style;                /* copy over the character           */
            reportException(Error_Incorrect_call_list, CHAR_TIME, IntegerOne, "CEFHLMNORST", new_string(work, 1));
            break;
    }
    /* now create a string object        */
    return new_string(work);
}

#define RANDOM_MIN 0
#define RANDOM_MAX 3
#define RANDOM_minimum 1
#define RANDOM_maximum 2
#define RANDOM_seed    3

BUILTIN(RANDOM)
{
    RexxInteger *minimum;                /* RANDOM minimum value              */
    RexxInteger *maximum;                /* RANDOM maximum value              */

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
    RexxInteger *seed = optional_integer(RANDOM, seed);
    /* have the activation generate      */
    return context->random(minimum, maximum, seed);
}

#define XRANGE_MIN 0
#define XRANGE_MAX 2
#define XRANGE_start   1
#define XRANGE_end     2

BUILTIN(XRANGE)
{
    fix_args(XRANGE);                    /* expand arguments to full value    */
    char startchar = 0;                  /* set default start position        */
    char endchar = (char)0xff;           /* set default end position          */

                                         /* get the starting string           */
    RexxString *start = optional_string(XRANGE, start);
    RexxString *end = optional_string(XRANGE, end);  /* get the ending string             */

    if (start != OREF_NULL)
    {            /* have a start position             */
        if (start->getLength() != 1)            /* not a single character?           */
        {
            /* have an error                     */
            reportException(Error_Incorrect_call_pad, CHAR_XRANGE, IntegerOne, start);
        }
        startchar = start->getChar(0);     /* get the new start position        */
    }
    if (end != OREF_NULL)
    {              /* have an end position              */
        if (end->getLength() != 1)         /* not a single character?           */
        {
                                           /* have an error                     */
            reportException(Error_Incorrect_call_pad, CHAR_XRANGE, IntegerTwo, end);
        }
        endchar = end->getChar(0);         /* get the new end position          */
    }
    /* calculate result size             */
    size_t length = ((endchar < startchar) ? (256 - startchar) + endchar : (endchar - startchar)) + 1;
    RexxString *result = raw_string(length);         /* get a result string               */
    for (size_t i = 0; i < length; i++)         /* loop through result length        */
    {
        result->putChar(i, startchar++);   /* inserting each character          */
    }
    return result;                       /* finished                          */
}

#define SYMBOL_MIN 1
#define SYMBOL_MAX 1
#define SYMBOL_name    1

BUILTIN(SYMBOL)
{
    fix_args(SYMBOL);                    /* expand arguments to full value    */
                                         /* get the variable name             */
    RexxString *name = required_string(SYMBOL, name);
    /* get a variable retriever          */
    RexxVariableBase *variable = RexxVariableDictionary::getVariableRetriever(name);
    if (variable == OREF_NULL)           /* invalid variable name?            */
    {
                                         /* return the 'BAD' result           */
        return new_string(CHAR_BAD);
    }
    else if (isOfClass(String, variable))    /* directly returned a string?       */
    {
        /* this is a literal value           */
        return new_string(CHAR_LIT);
    }
    else
    {                               /* need to perform lookup            */
                                    /* see if variable has a value       */
        if (!variable->exists(context))
        {
            /* this is a literal value           */
            return new_string(CHAR_LIT);
        }
        else
        {
            /* this is a variable value          */
            return new_string(CHAR_VAR);
        }
    }
}

#define VAR_MIN 1
#define VAR_MAX 1
#define VAR_name    1

BUILTIN(VAR)
{
    fix_args(VAR);                       /* expand arguments to full value    */
                                         /* get the variable name             */
    RexxString *variable = required_string(VAR, name);
    /* get a variable retriever          */
    RexxVariableBase *retriever = RexxVariableDictionary::getVariableRetriever(variable);
    if (retriever == OREF_NULL)          /* invalid variable name?            */
    {
        return TheFalseObject;           /* return the 'BAD' result           */
    }
    else if (isOfClass(String, retriever))   /* directly returned a string?       */
    {
        return TheFalseObject;           /* this doesn't exist either         */
    }
    else
    {                               /* need to perform lookup            */
                                    /* get the variable value            */
        return retriever->exists(context) ? TheTrueObject : TheFalseObject;
    }
}

#define VALUE_MIN 1
#define VALUE_MAX 3
#define VALUE_name     1
#define VALUE_newValue 2
#define VALUE_selector 3

BUILTIN(VALUE)
{
    fix_args(VALUE);                     /* expand arguments to full value    */
                                         /* get the variable name             */
    RexxString *variable = required_string(VALUE, name);
    /* get the new value                 */
    RexxObject *newvalue = optional_argument(VALUE, newValue);
    /* and the selector                  */
    RexxString *selector = optional_string(VALUE, selector);
    // get the variable type
    int variableType = variable->isSymbol();
    bool assignable = variableType == STRING_NAME || variableType == STRING_STEM || variableType == STRING_COMPOUND_NAME;

    if (selector == OREF_NULL)           /* have a selector?                  */
    {
        /* get a variable retriever          */
        RexxVariableBase *retriever = RexxVariableDictionary::getVariableRetriever(variable);
        // this could an invalid name, or we might be trying to assign a value to a non-variable
        // symbol.
        if (retriever == OREF_NULL || (newvalue != OREF_NULL && !assignable))
        {
            reportException(Error_Incorrect_call_symbol, CHAR_VALUE, IntegerOne, variable);
        }
        /* get the variable value            */
        RexxObject *result = retriever->getValue(context);
        if (newvalue != OREF_NULL)       /* have a new value to assign?       */
        {
                                         /* do the assignment                 */
            retriever->assign(context, stack, newvalue);
        }
        return result;                       /* return the indicator              */
    }
    else if (selector->getLength() == 0)   /* null string selector?             */
    {
        /* get the existing value            */
        RexxObject *result = TheEnvironment->entry(variable);
        if (result == OREF_NULL)           /* not in the environment?           */
        {
            /* turn into ".VARIABLE" as value    */
            result = ((RexxString *)OREF_PERIOD)->concat(variable->upper());
        }
        if (newvalue != OREF_NULL)         /* have a new value?                 */
        {
            /* do the set also                   */
            TheEnvironment->setEntry(variable, newvalue);
        }
        return result;                       /* return the indicator              */
    }
    else                                 /* external value function           */
    {
        RexxObject *result;
        // try the platform defined selectors.
        if (SystemInterpreter::valueFunction(variable, newvalue, selector, result))
        {
            return result;
        }
        // if the exit passes on this, try the platform-defined selectors
        if (!context->getActivity()->callValueExit(context, selector, variable, newvalue, result))
        {
            return result;
        }
        // this is an exception
        reportException(Error_Incorrect_call_selector, selector);
    }
    return OREF_NULL;    // should never reach here
}

#define ABS_MIN 1
#define ABS_MAX 1
#define ABS_n   1

BUILTIN(ABS)
{
    fix_args(ABS);                       /* check on required number of args  */
    /* get the argument in question      */
    RexxObject *argument = get_arg(ABS, n);
    if (isOfClass(Integer, argument))
    {      /* integer object already?           */
        /* we can process this without conversion */
        return((RexxInteger *)argument)->abs();
    }
    else if (isOfClass(NumberString, argument))
    { /* how about already numeric?        */
        /* we can process this without conversion */
        return((RexxNumberString *)argument)->abs();
    }
    /* force to a string object          */
    RexxString *n = required_string(ABS, n);
    return n->abs();                   /* invoke the string ABS function    */
}

#define SIGN_MIN 1
#define SIGN_MAX 1
#define SIGN_n   1

BUILTIN(SIGN)
{
    fix_args(SIGN);                       /* check on required number of args  */
    /* get the argument in question      */
    RexxObject *argument = get_arg(SIGN, n);
    if (isOfClass(Integer, argument))
    {       /* integer object already?           */
        /* we can process this without conversion */
        return((RexxInteger *)argument)->sign();
    }
    else if (isOfClass(NumberString, argument))
    { /* how about already numeric?        */
        /* we can process this without conversion */
        return((RexxNumberString *)argument)->Sign();
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

BUILTIN(FORMAT)
{
    fix_args(FORMAT);                    /* check on required number of args  */
                                         /* force to a string object          */
    RexxString *number = required_string(FORMAT, number);
    /* before value is optional          */
    RexxInteger *before = optional_integer(FORMAT, before);
    /* after value is optional           */
    RexxInteger *after = optional_integer(FORMAT, after);
    /* expp value is optional            */
    RexxInteger *expp = optional_integer(FORMAT, expp);
    /* expt value is optional            */
    RexxInteger *expt = optional_integer(FORMAT, expt);
    /* invoke the string FORMAT function   */
    return number->format(before, after, expp, expt);
}

#define MAX_MIN 1
#define MAX_MAX argcount
#define MAX_target 1

BUILTIN(MAX)
{
    check_args(MAX);                     /* check on required args            */
    /* get the argument in question      */
    RexxObject *argument = get_arg(MAX, target);
    if (isOfClass(NumberString, argument))
    { /* how about already numeric?        */
        /* we can process this without conversion */
        return((RexxNumberString *)argument)->Max(stack->arguments(argcount - 1), argcount - 1);
    }
    /* get the target string             */
    RexxString *target = required_string(MAX, target);
    /* go perform the MIN function       */
    return target->Max(stack->arguments(argcount - 1), argcount - 1);
}

#define MIN_MIN 1
#define MIN_MAX argcount
#define MIN_target 1

BUILTIN(MIN)
{
    check_args(MIN);                     /* check on required args            */
    /* get the argument in question      */
    RexxObject *argument = get_arg(MIN, target);
    if (isOfClass(NumberString, argument))
    { /* how about already numeric?        */
        /* we can process this without conversion */
        return((RexxNumberString *)argument)->Min(stack->arguments(argcount - 1), argcount - 1);
    }
    /* get the target string             */
    RexxString *target = required_string(MIN, target);
    /* go perform the MIN function       */
    return target->Min(stack->arguments(argcount - 1), argcount - 1);
}

#define SOURCELINE_MIN 0
#define SOURCELINE_MAX 1
#define SOURCELINE_n   1

BUILTIN(SOURCELINE)
{
    fix_args(SOURCELINE);                /* check on required number of args  */
    // get the effective source object.  If we're in an interpret context, this will
    // be the one of our caller.
    RexxSource *source = context->getEffectiveSourceObject();
    size_t size = source->sourceSize();  /* get the program size              */
    if (argcount == 1)                   /* asking for a specific line?       */
    {
        /* get the line number               */
        size_t line_number = required_integer(SOURCELINE, n)->getValue();
        /* must be a positive integer        */
        positive_integer((ssize_t)line_number, SOURCELINE, IntegerOne);
        if (line_number > size)            /* larger than program source?       */
        {
            /* this is an error too?             */
            reportException(Error_Incorrect_call_sourceline, line_number, size);
        }
        /* get the specific line             */
        return(RexxObject *)source->get(line_number);
    }
    else
    {
        /* just return the source size       */
        return(RexxObject *)new_integer(size);
    }
}

#define TRACE_MIN 0
#define TRACE_MAX 1
#define TRACE_setting 1

BUILTIN(TRACE)
{
    RexxString  *result;                 /* returned result                   */
    RexxString  *setting;                /* new trace setting                 */

    fix_args(TRACE);                     /* check required arguments          */
                                         /* get the trace setting             */
    setting = optional_string(TRACE, setting);
    result = context->traceSetting();    /* get the existing trace setting    */
    if (setting != OREF_NULL)
    {          /* have a new setting?               */
        context->setTrace(setting);
    }
    return result;                       /* return old trace setting          */
}

/* check to see if stream is to queue*/
bool check_queue(RexxString *name)
/******************************************************************************/
/* Function:  Check to see if a stream name is a queue                        */
/******************************************************************************/
{
    if (name != OREF_NULL)               /* non-default name?                 */
    {
        return name->strCaselessCompare("QUEUE:");/* compare against the queue         */
    }
    else
    {
        return false;                   /* not the queue                     */
    }
}

#define LINEIN_MIN 0
#define LINEIN_MAX 3
#define LINEIN_name   1
#define LINEIN_line   2
#define LINEIN_count  3

BUILTIN(LINEIN)
{
    fix_args(LINEIN);                    /* check required arguments          */

    RexxString *name = optional_string(LINEIN, name);/* get the string name               */
                                         /* get the line position             */
    RexxInteger *line = optional_integer(LINEIN, line);
    /* and the optional count of lines   */
    RexxInteger *count = optional_integer(LINEIN, count);
    if (check_queue(name))
    {             /* is this "QUEUE:"                  */
        RexxString *result;
                  /* if exit declines call             */
        if (context->getActivity()->callPullExit(context, result))
        {
            /* get the default output stream     */
            RexxObject *stream = context->getLocalEnvironment(OREF_REXXQUEUE);
            /* pull from the queue               */
            return stream->sendMessage(OREF_LINEIN);
        }
        return result;
    }
    else
    {
        bool added = false;
        /* get a stream for this name        */
        RexxObject *stream = context->resolveStream(name, true, NULL, &added);
        switch (argcount)
        {                /* process according to argcount     */
            case 0:                          /* no name                           */
            case 1:                          /* name only                         */
                return stream->sendMessage(OREF_LINEIN);
                break;
            case 2:                          /* name and start                    */
                return stream->sendMessage(OREF_LINEIN, line);
                break;
            case 3:                          /* name, start and count             */
                return stream->sendMessage(OREF_LINEIN, line, count);
                break;
        }
    }
    return OREF_NULLSTRING;  // should never happen
}

#define CHARIN_MIN 0
#define CHARIN_MAX 3
#define CHARIN_name   1
#define CHARIN_start  2
#define CHARIN_count  3

BUILTIN(CHARIN)
{
    fix_args(CHARIN);                    /* check required arguments          */
                                         /* get the string name               */
    RexxString *name = optional_string(CHARIN, name);
    /* get the line position             */
    RexxInteger *position = optional_integer(CHARIN, start);
    /* and the optional count of chars   */
    RexxInteger *count = optional_integer(CHARIN, count);
    if (check_queue(name))               /* is this "QUEUE:"                  */
    {
                                         /* this isn't allowed                */
        reportException(Error_Incorrect_call_queue_no_char, OREF_CHARIN);
    }

    /* get a stream for this name        */
    bool added = false;
    RexxObject *stream = context->resolveStream(name, true, NULL, &added);
    switch (argcount)
    {                  /* process according to argcount     */
        case 0:                            /* no name                           */
        case 1:                            /* name only                         */
            return stream->sendMessage(OREF_CHARIN);
            break;
        case 2:                            /* name and string                   */
            return stream->sendMessage(OREF_CHARIN, position);
            break;
        case 3:                            /* name, string and line             */
            return stream->sendMessage(OREF_CHARIN, position, count);
            break;
    }
    return OREF_NULLSTRING;              /* should never get here             */
}

#define LINEOUT_MIN 0
#define LINEOUT_MAX 3
#define LINEOUT_name   1
#define LINEOUT_string 2
#define LINEOUT_line   3

BUILTIN(LINEOUT)
{
    fix_args(LINEOUT);                   /* check required arguments          */
                                         /* get the string name               */
    RexxString *name = optional_string(LINEOUT, name);
    /* get the output string             */
    RexxString *string = optional_string(LINEOUT, string);
    /* get the line position             */
    RexxInteger *line = optional_integer(LINEOUT, line);
    if (check_queue(name))
    {             /* is this "QUEUE:"                  */
                  /* if exit declines call             */
        if (context->getActivity()->callPushExit(context, string, QUEUE_FIFO))
        {
            if (string != OREF_NULL)
            {       /* have an actual string to write?   */
                    /* get the default output stream     */
                RexxObject *stream = context->getLocalEnvironment(OREF_REXXQUEUE);
                /* push onto the queue               */
                return stream->sendMessage(OREF_QUEUENAME, string);
            }
            else
            {
                /* always a zero residual            */
                return IntegerZero;
            }
        }
    }
    else
    {
        bool added;
        RexxString *fullName;
        /* get a stream for this name        */
        RexxObject *stream = context->resolveStream(name, false, &fullName, &added);
        switch (argcount)
        {                /* process according to argcount     */
            case 0:                          /* no name                           */
            case 1:                          /* name only                         */
                return stream->sendMessage(OREF_LINEOUT);
                break;
            case 2:                          /* name and string                   */
                return stream->sendMessage(OREF_LINEOUT, string);
                break;
            case 3:                          /* name, string and line             */
                return stream->sendMessage(OREF_LINEOUT, string, line);
                break;
        }
    }
    return OREF_NULLSTRING;              /* should never happen               */
}

#define CHAROUT_MIN 0
#define CHAROUT_MAX 3
#define CHAROUT_name   1
#define CHAROUT_string 2
#define CHAROUT_start  3

BUILTIN(CHAROUT)
{
    fix_args(CHAROUT);                   /* check required arguments          */
                                         /* get the string name               */
    RexxString *name = optional_string(CHAROUT, name);
    /* get the output string             */
    RexxString *string = optional_string(CHAROUT, string);
    /* get the line position             */
    RexxInteger *position = optional_integer(CHAROUT, start);
    if (check_queue(name))               /* is this "QUEUE:"                  */
    {
                                         /* this isn't allowed                */
        reportException(Error_Incorrect_call_queue_no_char, OREF_CHAROUT);
    }

    bool added;
    /* get a stream for this name        */
    RexxObject *stream = context->resolveStream(name, false, NULL, &added);
    switch (argcount)
    {                  /* process according to argcount     */
        case 0:                            /* no name                           */
        case 1:                            /* name only                         */
            return stream->sendMessage(OREF_CHAROUT);
            break;
        case 2:                            /* name and string                   */
            return stream->sendMessage(OREF_CHAROUT, string);
            break;
        case 3:                            /* name, string and line             */
            return stream->sendMessage(OREF_CHAROUT, string, position);
            break;
    }
    return OREF_NULLSTRING;              /* should never happen               */
}

#define LINES_MIN 0
#define LINES_MAX    2
#define LINES_name   1
#define LINES_option 2

BUILTIN(LINES)
{
    fix_args(LINES);                     /* check required arguments          */

    RexxString *name = optional_string(LINES, name); /* get the string name               */
    RexxString *option = optional_string(LINES, option);
    RexxObject *result;
    if (check_queue(name))
    {             /* is this "QUEUE:"                  */
                  /* get the default output stream     */
        RexxObject *stream = context->getLocalEnvironment(OREF_REXXQUEUE);
        /* return count on the queue         */
        result = stream->sendMessage(OREF_QUERY);
    }
    else
    {
        bool added;
        /* get a stream for this name        */
        RexxObject *stream = context->resolveStream(name, true, NULL, &added);

        if (option != OREF_NULL)
        {
            switch (option->getChar(0))
            {      /* process the option character      */
                case 'C':
                case 'c':
                    break;
                case 'N':
                case 'n':
                    break;
                default:                         /* unknown option                    */
                    /* this is an error                  */
                    reportException(Error_Incorrect_call_list, CHAR_ARG, IntegerTwo, "NC", option);
                    break;
            }
        }
        else
        {
            option = OREF_NORMAL;
        }

        /* use modified LINES method with quick flag       */
        result = stream->sendMessage(OREF_LINES, option);
    }
    /* for compatibility this needs      */
    /* to only return 0 or 1             */
    if (toupper(option->getChar(0)) == 'N')
    {
        return (result != IntegerZero) ? IntegerOne : IntegerZero;
    }
    else
    {
        return result;
    }
}

#define CHARS_MIN 0
#define CHARS_MAX 1
#define CHARS_name   1

BUILTIN(CHARS)
{
    fix_args(CHARS);                     /* check required arguments          */

    RexxString *name = optional_string(CHARS, name); /* get the string name               */
    if (check_queue(name))               /* is this "QUEUE:"                  */
    {
                                         /* this isn't allowed                */
        reportException(Error_Incorrect_call_queue_no_char, OREF_CHARS);
    }
    /* get a stream for this name        */
    bool added;
    RexxObject *stream = context->resolveStream(name, true, NULL, &added);
    return stream->sendMessage(OREF_CHARS);
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

BUILTIN(STREAM)
{
    fix_args(STREAM);                    /* check required arguments          */
                                         /* get the string name               */
    RexxString *name = required_string(STREAM, name);
    if (name->getLength() == 0)          /* check name validity               */
    {
        /* raise an error                    */
        reportException(Error_Incorrect_call_stream_name, OREF_STREAM, name);
    }
    /* get any operation                 */
    RexxString *action = optional_string(STREAM, operation);
    /* get any command                   */
    RexxString *command = optional_string(STREAM, command);

    char action_char = STREAM_STATUS;       /* this is a status attempt          */
    if (action != OREF_NULL)
    {           /* no action given?                  */
        if (action->getLength() == 0)
        {    /* get a null string?                */
             /* this is an error                  */
            reportException(Error_Incorrect_call_list, CHAR_STREAM, IntegerTwo, "SDC", action);
        }
        /* get the option character          */
        action_char = toupper(action->getChar(0));
    }

    switch (action_char)
    {               /* process the options               */
        case STREAM_STATUS:                /* stream(name, s)                   */
            {
                if (argcount > 2)
                {              /* given a third argument?           */
                               /* raise an error                    */
                    reportException(Error_Incorrect_call_maxarg, OREF_STREAM, IntegerTwo);
                }
                RexxObject *stream = context->resolveStream(name, true, NULL, NULL);
                /* get the stream state              */
                return stream->sendMessage(OREF_STATE);
                break;
            }

        case STREAM_DESCRIPTION:           /* stream(name, d)                   */
            {
                if (argcount > 2)
                {              /* given a third argument?           */
                               /* raise an error                    */
                    reportException(Error_Incorrect_call_maxarg, OREF_STREAM, IntegerTwo);
                }
                RexxObject *stream = context->resolveStream(name, true, NULL, NULL);
                /* get the stream description        */
                return stream->sendMessage(OREF_DESCRIPTION);
                break;
            }

        case STREAM_COMMAND:               /* stream(name, c, command)          */
            {
                if (argcount < 3)
                {              /* given a third argument?           */
                               /* raise an error                    */
                    reportException(Error_Incorrect_call_minarg, OREF_STREAM, IntegerThree);
                }
                /* get the stream description        */
                ProtectedObject p(command);

                /* I have to check the command twice because in the RexxMethods (i.g. query_exists)
                   I don't have access to the activation and thus not to the streamtable.
                   It's also not possible to pass context as the second argument because
                   stream is a RexxMethod and USE ARG RexxActivation is not possible */
                RexxString *command_upper = command->upper();
                ProtectedObject p1(command_upper);

                if (command_upper->wordPos(new_string("OPEN"), OREF_NULL)->getValue() > 0)
                {
                    RexxString *fullName;
                    bool added;
                    RexxObject *stream = context->resolveStream(name, true, &fullName, &added);
                    RexxString *result = (RexxString *)stream->sendMessage(OREF_COMMAND, command);
                    /* if open failed, remove the stream object from stream table again */
                    if (!result->strCompare("READY:"))
                    {
                        context->getStreams()->remove(fullName);
                    }
                    return result;
                }
                else if (command_upper->wordPos(new_string("CLOSE"), OREF_NULL)->getValue() > 0)
                {
                    RexxString *fullName;
                    bool added;
                    RexxObject *stream = context->resolveStream(name, true, &fullName, &added);
                    RexxString *result = (RexxString *)stream->sendMessage(OREF_COMMAND, command);
                    context->getStreams()->remove(fullName);
                    return result;
                }
                // these are real operations that might cause an implicit open
                else if (command_upper->wordPos(new_string("SEEK"), OREF_NULL)->getValue() > 0 ||
                    command_upper->wordPos(new_string("POSITON"), OREF_NULL)->getValue() > 0)
                {
                    RexxString *fullName;
                    bool added;
                    RexxObject *stream = context->resolveStream(name, true, &fullName, &added);
                    // this is a real operation, so just leave alone
                    RexxString *result = (RexxString *)stream->sendMessage(OREF_COMMAND, command);
                    return result;
                }
                else
                {
                    RexxObject *stream = context->resolveStream(name, true, NULL, NULL);
                    return stream->sendMessage(OREF_COMMAND, command);
                }
                break;
            }

        default:
            /* this is an error                  */
            reportException(Error_Incorrect_call_list, CHAR_STREAM, IntegerTwo, "SDC", action);
            break;
    }
    return OREF_NULL;                    /* should never happen        */
}

#define QUEUED_MIN 0
#define QUEUED_MAX 0

BUILTIN(QUEUED)
{

    check_args(QUEUED);                  /* check on required number of args  */
    RexxInteger  *queuesize;             /* returned queue size from sys exit */
                                         /* get the default output stream     */
    if (context->getActivity()->callQueueSizeExit(context, queuesize))
    {
        RexxObject *queue = context->getLocalEnvironment(OREF_REXXQUEUE);
        /* return count on the queue         */
        return queue->sendMessage(OREF_QUEUED);
    }
    else
    {
        return queuesize;                  /* return count from system exit     */
    }
}

#define CONDITION_MIN 0
#define CONDITION_MAX 1
#define CONDITION_option 1

BUILTIN(CONDITION)
{
    int   style = 'I';                   /* style of condition output         */
    fix_args(CONDITION);                 /* expand arguments to full value    */
                                         /* get the option string             */
    RexxString *option = optional_string(CONDITION, option);
    if (option != OREF_NULL)             /* just using default format?        */
    {
        if (option->getLength() == 0)   /* have a null string?               */
        {
            /* this is an error                  */
            reportException(Error_Incorrect_call_list, CHAR_CONDITION, IntegerOne, "ACDIOS", option);
        }

        /* option is first character         */
        style = toupper(option->getChar(0));
    }
    /* get current trapped condition     */
    RexxDirectory *conditionobj = context->getConditionObj();

    switch (style)
    {                     /* process various CONDITION objects */

        case 'A':                          /* 'A'dditional                      */
            if (conditionobj != OREF_NULL)
            { /* have a condition object?          */
              /* retrieve the additional info      */
                RexxObject *result = conditionobj->at(OREF_ADDITIONAL);
                if (result == OREF_NULL)       /* not there?                        */
                {
                    return TheNilObject;       /* return .nil                       */
                }
                else
                {
                    return result->copy();     /* copy the result info              */
                }
            }
            else
            {
                return TheNilObject;         /* return .nil if not there          */
            }
            break;

        case 'I':                          /* 'I'nstruction                     */
            if (conditionobj != OREF_NULL)   /* have a condition object?          */
            {
                /* retrieve the instruction info     */
                return conditionobj->at(OREF_INSTRUCTION);
            }
            break;

        case 'D':                          /* 'D'escription                     */
            if (conditionobj != OREF_NULL)
            { /* have a condition object?          */
              /* retrieve the description info     */
                RexxObject *result = conditionobj->at(OREF_DESCRIPTION);
                if (result == OREF_NULL)       /* not found?                        */
                {
                    result = OREF_NULLSTRING;    /* return a null string if nothing   */
                }
                return result;
            }
            break;

        case 'C':                          /* 'C'ondition name                  */
            if (conditionobj != OREF_NULL)   /* have a condition object?          */
            {
                /* retrieve the condition name       */
                return conditionobj->at(OREF_CONDITION);
            }
            break;

        case 'O':                          /* 'C'ondition name                  */
            if (conditionobj != OREF_NULL)   /* have a condition object?          */
            {
                return conditionobj->copy(); /* just return a copy of this        */
            }
            return TheNilObject;         /* return the NIL object             */

        case 'S':                          /* 'S'tate                           */
            if (conditionobj != OREF_NULL)   /* have a condition object?          */
            {
                /* get the current trap state        */
                return context->trapState((RexxString *)conditionobj->at(OREF_CONDITION));
            }
            break;

        default:                           /* unknown option                    */
            /* report an error                   */
            reportException(Error_Incorrect_call_list, CHAR_CONDITION, IntegerOne, "ACDIOS", option);
            break;
    }
    return OREF_NULLSTRING;
}

#define CHANGESTR_MIN 3
#define CHANGESTR_MAX 4
#define CHANGESTR_needle     1
#define CHANGESTR_haystack   2
#define CHANGESTR_newneedle  3
#define CHANGESTR_count      4

BUILTIN(CHANGESTR)
{
    fix_args(CHANGESTR);                 /* check on require number of args   */
                                         /* get string for new                */
    RexxString *needle = required_string(CHANGESTR, needle);
    /* get string for target             */
    RexxString *haystack = required_string(CHANGESTR, haystack);
    /* get string to change to           */
    RexxString *newneedle = required_string(CHANGESTR, newneedle);
    /* length is optional                */
    RexxInteger *count = optional_integer(CHANGESTR, count);
    /* go perform the pos function       */
    return haystack->changeStr(needle, newneedle, count);
}

#define COUNTSTR_MIN 2
#define COUNTSTR_MAX 2
#define COUNTSTR_needle     1
#define COUNTSTR_haystack   2

BUILTIN(COUNTSTR)
{
    fix_args(COUNTSTR);                  /* check on require number of args   */
                                         /* get string for new                */
    RexxString *needle = required_string(COUNTSTR, needle);
    /* get string for target             */
    RexxString *haystack = required_string(COUNTSTR, haystack);
    return haystack->countStrRexx(needle); /* go perform the countstr function  */
}


#define RXFUNCADD_MIN 2
#define RXFUNCADD_MAX 3
#define RXFUNCADD_name   1
#define RXFUNCADD_module 2
#define RXFUNCADD_proc   3

BUILTIN(RXFUNCADD)
{
    fix_args(RXFUNCADD);                 /* check on required number of args  */

    // we require a name and module, but the
    // procedure is optional.  If not specified, we
    // use the function name directly.
    RexxString *name = required_string(RXFUNCADD, name);
    RexxString *module = required_string(RXFUNCADD, module);
    RexxString *proc = optional_string(RXFUNCADD, proc);

    if (proc == OREF_NULL)
    {
        proc = name;
    }

    // hand this off to the package manager.
    return PackageManager::addRegisteredRoutine(name, module, proc);
}

#define RXFUNCDROP_MIN 1
#define RXFUNCDROP_MAX 1
#define RXFUNCDROP_name   1

BUILTIN(RXFUNCDROP)
{
    fix_args(RXFUNCDROP);                 /* check on required number of args  */

    // only a name is required.
    RexxString *name = required_string(RXFUNCDROP, name);

    // hand this off to the package manager.
    return PackageManager::dropRegisteredRoutine(name);
}

#define RXFUNCQUERY_MIN 1
#define RXFUNCQUERY_MAX 1
#define RXFUNCQUERY_name   1

BUILTIN(RXFUNCQUERY)
{
    fix_args(RXFUNCQUERY);                 /* check on required number of args  */

    // only a name is required.
    RexxString *name = required_string(RXFUNCQUERY, name);

    // hand this off to the package manager.
    return PackageManager::queryRegisteredRoutine(name);
}


#define QUEUEEXIT_MIN 1
#define QUEUEEXIT_MAX 1
#define QUEUEEXIT_name   1


// This somewhat funny function is implemented as a builtin because it
// requires quite a bit of internal access.
BUILTIN(QUEUEEXIT)
{
    fix_args(QUEUEEXIT);                   /* check on required number of args  */

    // only a name is required.
    RexxString *name = required_string(QUEUEEXIT, name);
    /* call the exit                     */
    context->getActivity()->callQueueNameExit(context, name);
    // make sure we have real object to return
    if (name == OREF_NULL)
    {
        name = OREF_NULLSTRING;
    }
    return name;
}

#define SETLOCAL_MIN 0
#define SETLOCAL_MAX 0

BUILTIN(SETLOCAL)
{
    check_args(SETLOCAL);              /* check on required number of args  */
    // the external environment implements this
    return SystemInterpreter::pushEnvironment(context);
}

#define ENDLOCAL_MIN 0
#define ENDLOCAL_MAX 0

BUILTIN(ENDLOCAL)
{
    check_args(ENDLOCAL);              /* check on required number of args  */
    // the external environment implements this
    return SystemInterpreter::popEnvironment(context);
}

#define QUALIFY_MIN 0
#define QUALIFY_MAX 1
#define QUALIFY_name  1

/**
 * Qualify a stream name.
 */
BUILTIN(QUALIFY)
{
    check_args(QUALIFY);               /* check on required number of args  */
    RexxString *name = optional_string(QUALIFY, name);

    char qualified_name[SysFileSystem::MaximumFileNameLength];
    // qualifyStreamName will not expand if not a null string on entry.
    qualified_name[0] = '\0';
    SysFileSystem::qualifyStreamName(name->getStringData(), qualified_name, sizeof(qualified_name));
    return new_string(qualified_name);
}

/* the following builtin function    */
/* table must maintain the same order*/
/* as the builtin function codes used*/
/* in the token class builtin        */
/* builtin function lookup           */
pbuiltin RexxSource::builtinTable[] = {
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
    &builtin_function_RXFUNCADD        ,
    &builtin_function_RXFUNCDROP       ,
    &builtin_function_RXFUNCQUERY      ,
    &builtin_function_ENDLOCAL         ,
    &builtin_function_SETLOCAL         ,
    &builtin_function_QUEUEEXIT        ,
    &builtin_function_QUALIFY          ,
};

