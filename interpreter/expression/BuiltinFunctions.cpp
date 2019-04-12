/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2019 Rexx Language Association. All rights reserved.    */
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
/* REXX Translator                                                            */
/*                                                                            */
/* Builtin Function Execution Stubs                                           */
/*                                                                            */
/******************************************************************************/

#include "RexxCore.h"
#include "StringClass.hpp"
#include "DirectoryClass.hpp"
#include "ArrayClass.hpp"
#include "VariableDictionary.hpp"
#include "RexxActivation.hpp"
#include "Activity.hpp"
#include "ExpressionBaseVariable.hpp"
#include "BuiltinFunctions.hpp"
#include "RexxDateTime.hpp"
#include "Numerics.hpp"
#include "ProtectedObject.hpp"
#include "PackageManager.hpp"
#include "SystemInterpreter.hpp"
#include "SysFileSystem.hpp"
#include "MethodArguments.hpp"
#include "NumberStringClass.hpp"

#include <stdio.h>
#include <ctype.h>

// lots of global names used here, so make the
// namespace global.
using namespace GlobalNames;

// a note on the constants here.  These values are
// used to define what arguments are used by a BIF.  For
// example, for CENTER, we expect a minimum of 2 arguments and
// a maximum of 3.  The first argument is called "string", the
// second is called "length", the third is called "pad".  These
// values are used by the macros to locate the appropriate arguments
// from the expression stack.

BUILTIN(CENTER)
{
    const size_t CENTER_Min = 2;
    const size_t CENTER_Max = 3;
    const size_t CENTER_string = 1;
    const size_t CENTER_length = 2;
    const size_t CENTER_pad =    3;

    fix_args(CENTER);

    RexxString *string = required_string(CENTER, string);
    RexxInteger *length = required_integer(CENTER, length);
    RexxString *pad = optional_pad(CENTER, pad);

    return string->center(length, pad);
}


BUILTIN(CENTRE)
{
    const size_t CENTRE_Min = 2;
    const size_t CENTRE_Max = 3;
    const size_t CENTRE_string = 1;
    const size_t CENTRE_length = 2;
    const size_t CENTRE_pad =    3;

    fix_args(CENTRE);

    RexxString *string = required_string(CENTRE, string);
    RexxInteger *length = required_integer(CENTRE, length);
    RexxString *pad = optional_pad(CENTRE, pad);

    return string->center(length, pad);
}


BUILTIN(DELSTR)
{
    const size_t DELSTR_Min = 1;
    const size_t DELSTR_Max = 3;
    const size_t DELSTR_string = 1;
    const size_t DELSTR_n =      2;
    const size_t DELSTR_length = 3;

    fix_args(DELSTR);

    RexxString *string = required_string(DELSTR, string);
    RexxInteger *n = optional_integer(DELSTR, n);
    RexxInteger *length = optional_integer(DELSTR, length);

    return string->delstr(n, length);
}


BUILTIN(DELWORD)
{
    const size_t DELWORD_Min = 2;
    const size_t DELWORD_Max = 3;
    const size_t DELWORD_string = 1;
    const size_t DELWORD_n =      2;
    const size_t DELWORD_length = 3;

    fix_args(DELWORD);

    RexxString *string = required_string(DELWORD, string);
    RexxInteger *n = required_integer(DELWORD, n);
    RexxInteger *length = optional_integer(DELWORD, length);

    return string->delWord(n, length);
}


BUILTIN(INSERT)
{
    const size_t INSERT_Min = 2;
    const size_t INSERT_Max = 5;
    const size_t INSERT_new =    1;
    const size_t INSERT_target = 2;
    const size_t INSERT_n =      3;
    const size_t INSERT_length = 4;
    const size_t INSERT_pad =    5;

    fix_args(INSERT);

    RexxString *newString = required_string(INSERT, new);
    RexxString *target = required_string(INSERT, target);
    RexxInteger *n = optional_integer(INSERT, n);
    RexxInteger *length = optional_integer(INSERT, length);
    RexxString *pad = optional_pad(INSERT, pad);

    return target->insert(newString, n, length, pad);
}


BUILTIN(LEFT)
{
    const size_t LEFT_Min = 2;
    const size_t LEFT_Max = 3;
    const size_t LEFT_string = 1;
    const size_t LEFT_length = 2;
    const size_t LEFT_pad =    3;

    fix_args(LEFT);

    RexxString *string = required_string(LEFT, string);
    RexxInteger *length = optional_integer(LEFT, length);
    RexxString *pad = optional_pad(LEFT, pad);

    return string->left(length, pad);
}


BUILTIN(OVERLAY)
{
    const size_t OVERLAY_Min = 2;
    const size_t OVERLAY_Max = 5;
    const size_t OVERLAY_new =    1;
    const size_t OVERLAY_target = 2;
    const size_t OVERLAY_n =      3;
    const size_t OVERLAY_length = 4;
    const size_t OVERLAY_pad =    5;

    fix_args(OVERLAY);

    RexxString *newString = required_string(OVERLAY, new);
    RexxString *target = required_string(OVERLAY, target);
    RexxInteger *n = optional_integer(OVERLAY, n);
    RexxInteger *length = optional_integer(OVERLAY, length);
    RexxString *pad = optional_pad(OVERLAY, pad);

    return target->overlay(newString, n, length, pad);
}


BUILTIN(POS)
{
    const size_t POS_Min = 2;
    const size_t POS_Max = 4;
    const size_t POS_needle =   1;
    const size_t POS_haystack = 2;
    const size_t POS_start =    3;
    const size_t POS_range =    4;

    fix_args(POS);

    RexxString *needle = required_string(POS, needle);
    RexxString *haystack = required_string(POS, haystack);
    RexxInteger *start = optional_integer(POS, start);
    RexxInteger *range = optional_integer(POS, range);

    return haystack->posRexx(needle, start, range);
}


BUILTIN(LASTPOS)
{
    const size_t LASTPOS_Min = 2;
    const size_t LASTPOS_Max = 4;
    const size_t LASTPOS_needle =   1;
    const size_t LASTPOS_haystack = 2;
    const size_t LASTPOS_start =    3;
    const size_t LASTPOS_range =    4;

    fix_args(LASTPOS);

    RexxString *needle = required_string(LASTPOS, needle);
    RexxString *haystack = required_string(LASTPOS, haystack);
    RexxInteger *start = optional_integer(LASTPOS, start);
    RexxInteger *range = optional_integer(LASTPOS, range);

    return haystack->lastPosRexx(needle, start, range);
}


BUILTIN(REVERSE)
{
    const size_t REVERSE_Min = 1;
    const size_t REVERSE_Max = 1;
    const size_t REVERSE_string = 1;

    fix_args(REVERSE);

    RexxString *string = required_string(REVERSE, string);

    return string->reverse();
}

const size_t RIGHT_Min = 2;
const size_t RIGHT_Max = 3;
const size_t RIGHT_string = 1;
const size_t RIGHT_length = 2;
const size_t RIGHT_pad =    3;

BUILTIN(RIGHT)
{
    fix_args(RIGHT);

    RexxString *string = required_string(RIGHT, string);
    RexxInteger *length = optional_integer(RIGHT, length);
    RexxString *pad = optional_pad(RIGHT, pad);

    return string->right(length, pad);
}


BUILTIN(STRIP)
{
    const size_t STRIP_Min = 1;
    const size_t STRIP_Max = 3;
    const size_t STRIP_string = 1;
    const size_t STRIP_option = 2;
    const size_t STRIP_char =   3;

    fix_args(STRIP);

    RexxString *string = required_string(STRIP, string);
    RexxString *option = optional_string(STRIP, option);
    RexxString *character = optional_string(STRIP, char);

    return string->strip(option, character);
}


BUILTIN(SPACE)
{
    const size_t SPACE_Min = 1;
    const size_t SPACE_Max = 3;
    const size_t SPACE_string = 1;
    const size_t SPACE_n =      2;
    const size_t SPACE_pad =    3;

    fix_args(SPACE);

    RexxString *string = required_string(SPACE, string);
    RexxInteger *n = optional_integer(SPACE, n);
    RexxString *pad = optional_pad(SPACE, pad);

    return string->space(n, pad);
}


BUILTIN(SUBSTR)
{
    const size_t SUBSTR_Min = 2;
    const size_t SUBSTR_Max = 4;
    const size_t SUBSTR_string = 1;
    const size_t SUBSTR_n =      2;
    const size_t SUBSTR_length = 3;
    const size_t SUBSTR_pad =    4;

    fix_args(SUBSTR);

    RexxString *string = required_string(SUBSTR, string);
    RexxInteger *n = required_integer(SUBSTR, n);
    RexxInteger *length = optional_integer(SUBSTR, length);
    RexxString *pad = optional_pad(SUBSTR, pad);

    return string->substr(n, length, pad);
}


BUILTIN(LOWER)
{
    const size_t LOWER_Min = 1;
    const size_t LOWER_Max = 3;
    const size_t LOWER_string = 1;
    const size_t LOWER_n =      2;
    const size_t LOWER_length = 3;

    fix_args(LOWER);

    RexxString *string = required_string(LOWER, string);
    RexxInteger *n = optional_integer(LOWER, n);
    RexxInteger *length = optional_integer(LOWER, length);

    return string->lowerRexx(n, length);
}


BUILTIN(UPPER)
{
    const size_t UPPER_Min = 1;
    const size_t UPPER_Max = 3;
    const size_t UPPER_string = 1;
    const size_t UPPER_n =      2;
    const size_t UPPER_length = 3;

    fix_args(UPPER);

    RexxString *string = required_string(UPPER, string);
    RexxInteger *n = optional_integer(UPPER, n);
    RexxInteger *length = optional_integer(UPPER, length);

    return string->upperRexx(n, length);
}


BUILTIN(SUBWORD)
{
    const size_t SUBWORD_Min = 2;
    const size_t SUBWORD_Max = 3;
    const size_t SUBWORD_string = 1;
    const size_t SUBWORD_n =      2;
    const size_t SUBWORD_length = 3;

    fix_args(SUBWORD);

    RexxString *string = required_string(SUBWORD, string);
    RexxInteger *n = required_integer(SUBWORD, n);
    RexxInteger *length = optional_integer(SUBWORD, length);

    return string->subWord(n, length);
}


BUILTIN(WORD)
{
    const size_t WORD_Min = 2;
    const size_t WORD_Max = 2;
    const size_t WORD_string = 1;
    const size_t WORD_n =      2;

    fix_args(WORD);

    RexxString *string = required_string(WORD, string);
    RexxInteger *n = required_integer(WORD, n);

    return string->word(n);
}


BUILTIN(WORDINDEX)
{
    const size_t WORDINDEX_Min = 2;
    const size_t WORDINDEX_Max = 2;
    const size_t WORDINDEX_string = 1;
    const size_t WORDINDEX_n =      2;

    fix_args(WORDINDEX);

    RexxString *string = required_string(WORDINDEX, string);
    RexxInteger *n = required_integer(WORDINDEX, n);

    return string->wordIndex(n);
}


BUILTIN(WORDLENGTH)
{
    const size_t WORDLENGTH_Min = 2;
    const size_t WORDLENGTH_Max = 2;
    const size_t WORDLENGTH_string = 1;
    const size_t WORDLENGTH_n =      2;

    fix_args(WORDLENGTH);

    RexxString *string = required_string(WORDLENGTH, string);
    RexxInteger *n = required_integer(WORDLENGTH, n);
    return string->wordLength(n);
}


BUILTIN(COPIES)
{
    const size_t COPIES_Min = 2;
    const size_t COPIES_Max = 2;
    const size_t COPIES_string = 1;
    const size_t COPIES_n =      2;

    fix_args(COPIES);

    RexxString *string = required_string(COPIES, string);
    RexxInteger *n = required_integer(COPIES, n);

    return string->copies(n);
}


BUILTIN(WORDPOS)
{
    const size_t WORDPOS_Min = 2;
    const size_t WORDPOS_Max = 3;
    const size_t WORDPOS_phrase = 1;
    const size_t WORDPOS_string = 2;
    const size_t WORDPOS_start =  3;

    fix_args(WORDPOS);

    RexxString *phrase = required_string(WORDPOS, phrase);
    RexxString *string = required_string(WORDPOS, string);
    RexxInteger *start = optional_integer(WORDPOS, start);

    return string->wordPos(phrase, start);
}


BUILTIN(WORDS)
{
    const size_t WORDS_Min = 1;
    const size_t WORDS_Max = 1;
    const size_t WORDS_string = 1;

    fix_args(WORDS);

    RexxString *string = required_string(WORDS, string);

    return string->words();
}


BUILTIN(ABBREV)
{
    const size_t ABBREV_Min = 2;
    const size_t ABBREV_Max = 3;
    const size_t ABBREV_information = 1;
    const size_t ABBREV_info =        2;
    const size_t ABBREV_length =      3;

    fix_args(ABBREV);

    RexxString *information = required_string(ABBREV, information);
    RexxString *info = required_string(ABBREV, info);
    RexxInteger *length = optional_integer(ABBREV, length);

    return information->abbrev(info, length);
}


BUILTIN(BITAND)
{
    const size_t BITAND_Min = 1;
    const size_t BITAND_Max = 3;
    const size_t BITAND_string1 = 1;
    const size_t BITAND_string2 = 2;
    const size_t BITAND_pad =     3;

    fix_args(BITAND);

    RexxString *string1 = required_string(BITAND, string1);
    RexxString *string2 = optional_string(BITAND, string2);
    RexxString *pad = optional_pad(BITAND, pad);

    return string1->bitAnd(string2, pad);
}


BUILTIN(BITOR)
{
    const size_t BITOR_Min = 1;
    const size_t BITOR_Max = 3;
    const size_t BITOR_string1 = 1;
    const size_t BITOR_string2 = 2;
    const size_t BITOR_pad =     3;

    fix_args(BITOR);

    RexxString *string1 = required_string(BITOR, string1);
    RexxString *string2 = optional_string(BITOR, string2);
    RexxString *pad = optional_pad(BITOR, pad);

    return string1->bitOr(string2, pad);
}


BUILTIN(BITXOR)
{
    const size_t BITXOR_Min = 1;
    const size_t BITXOR_Max = 3;
    const size_t BITXOR_string1 = 1;
    const size_t BITXOR_string2 = 2;
    const size_t BITXOR_pad =     3;

    fix_args(BITXOR);

    RexxString *string1 = required_string(BITXOR, string1);
    RexxString *string2 = optional_string(BITXOR, string2);
    RexxString *pad = optional_pad(BITXOR, pad);

    return string1->bitXor(string2, pad);
}


BUILTIN(B2X)
{
    const size_t B2X_Min = 1;
    const size_t B2X_Max = 1;
    const size_t B2X_string =  1;

    fix_args(B2X);

    RexxString *string = required_string(B2X, string);

    return string->b2x();
}


BUILTIN(X2B)
{
    const size_t X2B_Min = 1;
    const size_t X2B_Max = 1;
    const size_t X2B_string =  1;

    fix_args(X2B);

    RexxString *string = required_string(X2B, string);

    return string->x2b();
}


BUILTIN(C2X)
{
    const size_t C2X_Min = 1;
    const size_t C2X_Max = 1;
    const size_t C2X_string =  1;

    fix_args(C2X);

    RexxString *string = required_string(C2X, string);

    return string->c2x();
}


BUILTIN(X2C)
{
    const size_t X2C_Min = 1;
    const size_t X2C_Max = 1;
    const size_t X2C_string =  1;

    fix_args(X2C);

    RexxString *string = required_string(X2C, string);

    return string->x2c();
}


BUILTIN(C2D)
{
    const size_t C2D_Min = 1;
    const size_t C2D_Max = 2;
    const size_t C2D_string =  1;
    const size_t C2D_n =       2;

    fix_args(C2D);

    RexxString *string = required_string(C2D, string);
    RexxInteger *n = optional_integer(C2D, n);

    return string->c2d(n);
}


BUILTIN(TRUNC)
{
    const size_t TRUNC_Min = 1;
    const size_t TRUNC_Max = 2;
    const size_t TRUNC_number =  1;
    const size_t TRUNC_n =       2;

    fix_args(TRUNC);

    RexxString *number = required_string(TRUNC, number);
    RexxInteger *n = optional_integer(TRUNC, n);

    return number->trunc(n);
}


BUILTIN(X2D)
{
    const size_t X2D_Min = 1;
    const size_t X2D_Max = 2;
    const size_t X2D_string =  1;
    const size_t X2D_n =       2;

    fix_args(X2D);

    RexxString *string = required_string(X2D, string);
    RexxInteger *n = optional_integer(X2D, n);

    return string->x2d(n);
}


BUILTIN(D2X)
{
    const size_t D2X_Min = 1;
    const size_t D2X_Max = 2;
    const size_t D2X_string =  1;
    const size_t D2X_n =       2;

    fix_args(D2X);

    RexxString *string = required_string(D2X, string);
    RexxInteger *n = optional_integer(D2X, n);

    return string->d2x(n);
}


BUILTIN(D2C)
{
    const size_t D2C_Min = 1;
    const size_t D2C_Max = 2;
    const size_t D2C_string =  1;
    const size_t D2C_n =       2;

    fix_args(D2C);

    RexxString *string = required_string(D2C, string);
    RexxInteger *n = optional_integer(D2C, n);

    return string->d2c(n);
}


BUILTIN(COMPARE)
{
    const size_t COMPARE_Min = 2;
    const size_t COMPARE_Max = 3;
    const size_t COMPARE_string1 = 1;
    const size_t COMPARE_string2 = 2;
    const size_t COMPARE_pad =     3;

    fix_args(COMPARE);

    RexxString *string1 = required_string(COMPARE, string1);
    RexxString *string2 = required_string(COMPARE, string2);
    RexxString *pad = optional_pad(COMPARE, pad);

    return string1->compare(string2, pad);
}


BUILTIN(LENGTH)
{
    const size_t LENGTH_Min = 1;
    const size_t LENGTH_Max = 1;
    const size_t LENGTH_string =  1;

    fix_args(LENGTH);

    RexxString *target = required_string(LENGTH, string);

    return target->lengthRexx();
}


BUILTIN(TRANSLATE)
{
    const size_t TRANSLATE_Min = 1;
    const size_t TRANSLATE_Max = 6;
    const size_t TRANSLATE_string =  1;
    const size_t TRANSLATE_tableo =  2;
    const size_t TRANSLATE_tablei =  3;
    const size_t TRANSLATE_pad =     4;
    const size_t TRANSLATE_start =   5;
    const size_t TRANSLATE_range =   6;

    fix_args(TRANSLATE);

    RexxString *string = required_string(TRANSLATE, string);
    RexxString *tableo = optional_string(TRANSLATE, tableo);
    RexxString *tablei = optional_string(TRANSLATE, tablei);
    RexxString *pad = optional_pad(TRANSLATE, pad);
    RexxInteger *start = optional_integer(TRANSLATE, start);
    RexxInteger *range = optional_integer(TRANSLATE, range);

    return string->translate(tableo, tablei, pad, start, range);
}


const size_t VERIFY_Min = 2;
const size_t VERIFY_Max = 5;
const size_t VERIFY_string =    1;
const size_t VERIFY_reference = 2;
const size_t VERIFY_option =    3;
const size_t VERIFY_start =     4;
const size_t VERIFY_range =     5;

BUILTIN(VERIFY)
{
    const size_t VERIFY_Min = 2;
    const size_t VERIFY_Max = 5;
    const size_t VERIFY_string =    1;
    const size_t VERIFY_reference = 2;
    const size_t VERIFY_option =    3;
    const size_t VERIFY_start =     4;
    const size_t VERIFY_range =     5;

    fix_args(VERIFY);

    RexxString *string = required_string(VERIFY, string);
    RexxString *reference = required_string(VERIFY, reference);
    RexxString *option = optional_string(VERIFY, option);
    RexxInteger *start = optional_integer(VERIFY, start);
    RexxInteger *range = optional_integer(VERIFY, range);

    return string->verify(reference, option, start, range);
}


BUILTIN(DATATYPE)
{
    const size_t DATATYPE_Min = 1;
    const size_t DATATYPE_Max = 2;
    const size_t DATATYPE_string =    1;
    const size_t DATATYPE_type =      2;

    fix_args(DATATYPE);

    RexxString *string = required_string(DATATYPE, string);
    RexxString *type = optional_string(DATATYPE, type);

    return string->dataType(type);
}


BUILTIN(ADDRESS)
{
    const size_t ADDRESS_Min = 0;
    const size_t ADDRESS_Max = 0;

    check_args(ADDRESS);

    return context->getAddress();
}


BUILTIN(DIGITS)
{
    const size_t DIGITS_Min = 0;
    const size_t DIGITS_Max = 0;

    check_args(DIGITS);

    return new_integer(context->digits());
}


BUILTIN(FUZZ)
{
    const size_t FUZZ_Min = 0;
    const size_t FUZZ_Max = 0;

    check_args(FUZZ);

    return new_integer(context->fuzz());
}


BUILTIN(FORM)
{
    const size_t FORM_Min = 0;
    const size_t FORM_Max = 0;

    check_args(FORM);

    return context->form() == Numerics::FORM_SCIENTIFIC ? SCIENTIFIC : ENGINEERING;
}


BUILTIN(USERID)
{
    const size_t USERID_Min = 0;
    const size_t USERID_Max = 0;

    check_args(USERID);
    return SystemInterpreter::getUserid();
}


BUILTIN(ERRORTEXT)
{
    const size_t ERRORTEXT_Min = 1;
    const size_t ERRORTEXT_Max = 1;
    const size_t ERRORTEXT_n =   1;

    check_args(ERRORTEXT);

    // verify this is in range for a Rexx error condition (we only retrieve
    // the major error message, which does not have substitutions)
    wholenumber_t error_number = (required_integer(ERRORTEXT, n))->getValue();
    if (error_number < 0 || error_number > 99)
    {
        reportException(Error_Incorrect_call_range, "ERRORTEXT", IntegerOne, error_number);
    }
    // get the error message for this number and return the text.
    RexxString *result = Interpreter::getMessageText(error_number * 1000);
    if (result == OREF_NULL)
    {
        result = GlobalNames::NULLSTRING;
    }
    return result;
}


BUILTIN(ARG)
{
    const size_t ARG_Min = 0;
    const size_t ARG_Max = 2;
    const size_t ARG_n =      1;
    const size_t ARG_option = 2;

    fix_args(ARG);
    RexxInteger *n = optional_integer(ARG, n);
    RexxString *option = optional_string(ARG, option);

    // get the arguments from the context
    RexxObject **arglist = context->getMethodArgumentList();
    size_t size = context->getMethodArgumentCount();

    // if there is no position, then any option is invalid
    if (n == OREF_NULL)
    {
        if (option != OREF_NULL)
        {
            reportException(Error_Incorrect_call_noarg, "ARG", IntegerOne);
        }

        // this is the default, which is to just return the count
        return new_integer(size);
    }
    // position with no argument is just looking for a specific arg.
    else if (option == OREF_NULL)
    {
        // verify this is a positive integer value
        size_t position = n->getValue();
        positive_integer(position, ARG, IntegerOne);

        // out of range is just a null string
        if (size < position)
        {
            return GlobalNames::NULLSTRING;
        }
        else
        {
            // if the argumetn does not exist, return a null string
            RexxObject *result = arglist[position - 1];
            if (result == OREF_NULL)
            {
                return GlobalNames::NULLSTRING;
            }
            return result;
        }
    }
    // process the different options.  We have both a position
    // and an option
    else
    {
        // get the postition value, which is the same for all options
        size_t position = n->getValue();
        positive_integer(position, ARG, IntegerOne);

        switch (toupper(option->getChar(0)))
        {
            // 'A'rray
            case 'A':
                // if the position is 1, return the entire array
                if (position == 1)
                {
                    return new_array(size, arglist);
                }
                // if greater than the size, this is an empty array
                else if (position > size)
                {

                    return new_array();
                }
                else
                {
                    // extract the sub array
                    return new_array(size - position + 1, &arglist[position - 1]);
                }
                break;

            // 'E'xist
            case 'E':
                // out of range is always false
                if (position > size)
                {
                    return TheFalseObject;
                }
                // test the actual arg position
                return booleanObject(arglist[position - 1] != OREF_NULL);
                break;

            // 'O'mitted argument?
            case 'O':
                // out if range is an omitted arg
                if (position > size)
                {
                    return TheTrueObject;
                }
                // check the actual position
                return booleanObject(arglist[position - 1] == OREF_NULL);
                break;

            // 'N'ormal, which is the same as the position and no argument
            case 'N':
                if (position > size)
                {
                    return GlobalNames::NULLSTRING;
                }
                else
                {
                    RexxObject *result = arglist[position - 1];
                    if (result == OREF_NULL)
                    {
                        return GlobalNames::NULLSTRING;
                    }
                    return result;
                }
                break;

            // unknown option
            default:
                reportException(Error_Incorrect_call_list, "ARG", IntegerTwo, "AENO", option);
                break;
        }
    }
    return GlobalNames::NULLSTRING;              // should never happen
}


BUILTIN(DATE)
{
    const size_t DATE_Min = 0;
    const size_t DATE_Max = 5;
    const size_t DATE_option =  1;
    const size_t DATE_indate =  2;
    const size_t DATE_option2 = 3;
    const size_t DATE_osep =    4;
    const size_t DATE_isep =    5;

    fix_args(DATE);

    RexxString *option = optional_string(DATE, option);
    RexxString *indate = optional_string(DATE, indate);
    RexxString *option2 = optional_string(DATE, option2);
    RexxString *osep = optional_string(DATE, osep);
    RexxString *isep = optional_string(DATE, isep);

    // get a copy of the current activation time.  This will ensure
    // a consistent timestamp across calls
    RexxDateTime current = context->getTime();
    // by default, we operate off of the current time.  We may end up
    // overwriting this
    RexxDateTime timestamp = current;

    // default for both input and output styles is 'N'ormal
    int style = 'N';
    int style2 = 'N';

    // now process the various option specifiers
    if (option != OREF_NULL)
    {
        if (option->getLength() == 0)
        {
            reportException(Error_Incorrect_call_list, "DATE", IntegerOne, "BDEFILMNOSTUW", option);
        }
        else
        {
            style = toupper(option->getChar(0));
        }
    }

    // opt2 or isep w/o date?
    if (indate == OREF_NULL && (option2 != OREF_NULL || isep != OREF_NULL))
    {
        reportException(Error_Incorrect_call_noarg, "DATE", IntegerTwo);
    }

    // have a second option specified?  Get the option character from that also
    if (option2 != OREF_NULL)
    {
        if (option2->getLength() == 0)
        {
            reportException(Error_Incorrect_call_list, "DATE", IntegerThree, "BDEFINOSTU", option2);
        }
        else
        {
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
            reportException(Error_Incorrect_call_format_incomp_sep, "DATE", IntegerOne, new_string((char)style), IntegerFour);
        }
        // must be zero or 1 character and cannot be alpha numeric
        if (osep->getLength() > 1 || (osep->getLength() == 1 && strchr(ALPHANUM, osep->getChar(0)) != NULL))
        {
            reportException(Error_Incorrect_call_parm_wrong_sep, "DATE", IntegerFour, osep);
        }
        // string objects are null terminated, so we can point directly at what will
        // be either 1 or 0 characters of data.
        outputSeparator = osep->getStringData();
    }

    // given an input timestamp to parse?  Need to figure out the form
    if (indate != OREF_NULL)
    {
        bool valid = true;

        const char *separator = NULL;      // different formats will override this

        // if we have a separator for input, perform validation here
        if (isep != OREF_NULL)
        {
            // only valid with certain styles
            if (strchr("BDMWL", style2) != NULL)
            {
                reportException(Error_Incorrect_call_format_incomp_sep, "DATE", IntegerThree, new_string((char *)&style2, 1), IntegerFive);
            }

            // explicitly specified delimiter, we need to validate this first
            if (isep->getLength() > 1 || (isep->getLength() == 1 && strchr(ALPHANUM, isep->getChar(0)) != NULL))
            {
                // the field delimiter must be a single character and NOT
                // alphanumeric, or a null character
                reportException(Error_Incorrect_call_parm_wrong_sep, new_string("DATE"), IntegerFive, isep);
            }

            // string objects are null terminated, so we can point directly at what will
            // be either 1 or 0 characters of data.
            separator = isep->getStringData();
        }

        // clear the time stamp copy
        timestamp.clear();
        // everything is done using the current timezone offset
        timestamp.setTimeZoneOffset(current.getTimeZoneOffset());
        // the RexxDateTime option does the parsing directly.
        switch (style2)
        {
            // 'N'ormal
            case 'N':
                valid = timestamp.parseNormalDate(indate->getStringData(), separator);
                break;

            // 'B'asedate
            case 'B':
            {
                // this is just a numeric value
                wholenumber_t basedays;
                if (!indate->numberValue(basedays) || !timestamp.setBaseDate(basedays))
                {
                    reportException(Error_Incorrect_call_format_invalid, "DATE", indate, new_string((char *)&style2, 1));
                }
                break;
            }

            // 'F'ull
            case 'F':
            {
                // this is a BIG numeric value, since it also includes the time portion
                int64_t basetime;
                if (!Numerics::objectToInt64(indate, basetime) || !timestamp.setBaseTime(basetime))
                {
                    reportException(Error_Incorrect_call_format_invalid, "DATE", indate, new_string((char *)&style2, 1));
                }
                break;
            }

            // 'T'icks, which is also a big number value
            case 'T':
            {
                int64_t basetime;
                if (!Numerics::objectToInt64(indate, basetime) || !timestamp.setUnixTime(basetime))
                {
                    reportException(Error_Incorrect_call_format_invalid, "DATE", indate, new_string((char *)&style2, 1));
                }
                break;
            }

            // 'D'ay of year
            case 'D':
            {
                // smaller numeric value
                wholenumber_t yearday;
                if (!indate->numberValue(yearday) || yearday < 0 || yearday > YEAR_DAYS + 1 ||
                    (yearday > YEAR_DAYS && !LeapYear(current.year)))
                {
                    reportException(Error_Incorrect_call_format_invalid, "DATE", indate, new_string((char *)&style2, 1));
                }
                // set the date directly
                timestamp.setDate(current.year, yearday);
                break;
            }

            // 'E'uropean format, days-month-year
            case 'E':
                valid = timestamp.parseEuropeanDate(indate->getStringData(), separator, current.year);
                break;

            // 'O'rdered format, year-month-day
            case 'O':
                valid = timestamp.parseOrderedDate(indate->getStringData(), separator, current.year);
                break;

            // 'I'SO 8601 format
            case 'I':
                valid = timestamp.parseISODate(indate->getStringData(), separator);
                break;

            // 'S'tandard format
            case 'S':
                valid = timestamp.parseStandardDate(indate->getStringData(), separator);
                break;

            // 'U'SA format, month-day-year
            case 'U':
                valid = timestamp.parseUsaDate(indate->getStringData(), separator, current.year);
                break;

            // invalid input option
            default:
                reportException(Error_Incorrect_call_list, "DATE", IntegerThree, "BDEFINOSTU", new_string((char *)&style2, 1));
                break;
        }

        // if there's a formatting error
        if (!valid)
        {
            // different error message depending on whether a separator was specified, or not.
            if (isep != OREF_NULL)
            {
                reportException(Error_Incorrect_call_format_incomp_sep, "DATE", IntegerTwo, indate, IntegerFive);
            }
            else
            {
                reportException(Error_Incorrect_call_format_invalid, "DATE", indate, new_string((char *)&style2, 1));
            }
        }
    }
    else
    {
        // just copy the current time stamp
        timestamp = current;
    }

    wholenumber_t day = timestamp.day;
    wholenumber_t month = timestamp.month;
    wholenumber_t year = timestamp.year;

    // our working output buffer
    char work[64];

    // reverse of the inputs...the time stamp handles most of these directly
    switch (style)
    {

        case 'B':
            // always return RexxInteger instead of String
            return new_integer(timestamp.getBaseDate());

        case 'F':
            // if possible, return RexxInteger instead of String
#ifdef __REXX64__
            return new_integer(timestamp.getBaseTime());
#else
            timestamp.formatBaseTime(work);
            break;
#endif

        case 'T':
            // if possible, return RexxInteger instead of String
#ifdef __REXX64__
            return new_integer(timestamp.getUnixTime());
#else
            timestamp.formatUnixTime(work);
            break;
#endif

        case 'D':
            // always return RexxInteger instead of String
            return new_integer(timestamp.getYearDay());

        case 'E':
            timestamp.formatEuropeanDate(work, outputSeparator);
            break;

        case 'L':
        {
            // the month name comes from the message repository
            RexxString *month_name = Interpreter::getMessageText(Message_Translations_January + month - 1);

            sprintf(work, "%zd %s %4.4zd", day, month_name->getStringData(), year);
            break;

        }

        case 'M':
            timestamp.formatMonthName(work);
            break;

        case 'N':
            timestamp.formatNormalDate(work, outputSeparator);
            break;

        case 'O':
            timestamp.formatOrderedDate(work, outputSeparator);
            break;

        case 'I':
            timestamp.formatISODate(work, outputSeparator);
            break;

        case 'S':
            timestamp.formatStandardDate(work, outputSeparator);
            break;

        case 'U':
            timestamp.formatUsaDate(work, outputSeparator);
            break;

        case 'W':
            timestamp.formatWeekDay(work);
            break;

        default:
            work[0] = style;
            reportException(Error_Incorrect_call_list, "DATE", IntegerOne, "BDEFILMNOSTUW", new_string(work, 1));
            break;
    }
    // return as a string object
    return new_string(work);
}


BUILTIN(TIME)
{
    const size_t TIME_Min = 0;
    const size_t TIME_Max = 3;
    const size_t TIME_option =  1;
    const size_t TIME_intime =  2;
    const size_t TIME_option2 = 3;

    fix_args(TIME);

    RexxString *option = optional_string(TIME, option);
    RexxString *intime = optional_string(TIME, intime);
    RexxString *option2 = optional_string(TIME, option2);

    // get the current activation time and a copy for the default input
    RexxDateTime current = context->getTime();
    RexxDateTime timestamp = current;

    int style = 'N';                     // get the default style

    // do we have a style option specified?  Validate, and retrieve
    if (option != OREF_NULL)
    {
        // null strings not allowed as an option character
        if (option->getLength() == 0)
        {
            reportException(Error_Incorrect_call_list, "TIME", IntegerOne, "CEFHLMNORST", option);
        }
        // we only use the first character
        style = toupper(option->getChar(0));
    }

    // now repeat with the input style
    int style2 = 'N';

    // has the input style been specified?
    if (option2 != OREF_NULL)
    {
        // the second option requires an input date
        if (intime == OREF_NULL)
        {
            reportException(Error_Incorrect_call_noarg, "TIME", IntegerTwo);
        }
        // again, must be at least one character, of which we only use the first
        if (option2->getLength() == 0)
        {
            reportException(Error_Incorrect_call_list, "TIME", IntegerThree, "CFHLMNOST", option2);
        }
        style2 = toupper(option2->getChar(0));
    }

    // we have an input time, so we need to parse this
    if (intime != OREF_NULL)
    {
        // the input timestamp is not valid with the elapsed time options, and
        // using an offset as an input isn't really meaningful
        if (style == 'R' || style == 'E')
        {
            reportException(Error_Incorrect_call_invalid_conversion, "TIME", new_string((char *)&style, 1));
        }
        bool valid = true;                 // assume this is a good timestamp
        timestamp.clear();                 // clear everything out
        // everything is done using the current timezone offset
        timestamp.setTimeZoneOffset(current.getTimeZoneOffset());

        switch (style2)
        {
            // 'N'ormal default style, 01:23:45 format (24 hour)
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

            // 'H'our format...
            case 'H':
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
                int64_t basetime;
                if (!Numerics::objectToInt64(intime, basetime) || !timestamp.setBaseTime(basetime))
                {
                    reportException(Error_Incorrect_call_format_invalid, "TIME", intime, new_string((char *)&style2, 1));
                }
                break;
            }

            case 'T':                        /* 'T'icks datetime stamp            */
            {
                int64_t basetime;
                if (!Numerics::objectToInt64(intime, basetime) || !timestamp.setUnixTime(basetime))
                {
                    reportException(Error_Incorrect_call_format_invalid, "TIME", intime, new_string((char *)&style2, 1));
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

            // an invalid input style
            default:
                reportException(Error_Incorrect_call_list, "TIME", IntegerThree, "CFHLMNOST", new_string((char)style2));
                break;
        }
        if (!valid)
        {
            reportException(Error_Incorrect_call_format_invalid, "TIME", intime, new_string((char *)&style2, 1) );
        }
    }

    // our working output buffer
    char work[64];

    // now output styles
    switch (style)
    {
        case 'E':                         // 'E'lapsed time
        case 'R':                         // 'R'eset elapsed time
        {
            // get the elapsed time from the context
            int64_t startTime = context->getElapsed();
            // substract the time values
            int64_t threshold = current.getUTCBaseTime() - startTime;
            // a negative value always returns zero and we reset
            if (threshold < 0)
            {
                strcpy(work, "0");
                context->resetElapsed();
            }
            // zero is always exactly zero
            else if (threshold == 0)
            {
                strcpy(work, "0");
            }
            else
            {
                // format as a long time
                sprintf(work, "%d.%06d", (int)(threshold / (int64_t)MICROSECONDS), (int)(threshold % (int64_t)MICROSECONDS));
            }

            // if this is a reset call, then we reset the timer also
            if (style == 'R')
            {
                context->resetElapsed();
            }
            break;
        }

        case 'C':                         // 'C'ivil time
            timestamp.formatCivilTime(work);
            break;

        case 'H':                         // 'Hours'
            // always return RexxInteger instead of String
            return new_integer(timestamp.hours);

        case 'L':                         // 'L'ong format
            timestamp.formatLongTime(work);
            break;

        case 'M':                         // 'M'inutes format
            // always return RexxInteger instead of String
            return new_integer(timestamp.hours * MINUTES_IN_HOUR + timestamp.minutes);

        case 'N':                         // 'N'ormal format...the default
            timestamp.formatNormalTime(work);
            break;

        case 'S':                         // 'S'econds format...total seconds
            // always return RexxInteger instead of String
            return new_integer((timestamp.hours * MINUTES_IN_HOUR + timestamp.minutes) * SECONDS_IN_MINUTE + timestamp.seconds);

        case 'F':                          // 'F'ull
            // if possible, return RexxInteger instead of String
#ifdef __REXX64__
            return new_integer(timestamp.getBaseTime());
#else
            timestamp.formatBaseTime(work);
            break;
#endif

        case 'T':                          // 'T'icks
            // if possible, return RexxInteger instead of String
#ifdef __REXX64__
            return new_integer(timestamp.getUnixTime());
#else
            timestamp.formatUnixTime(work);
            break;
#endif

        case 'O':                          // 'O'ffset.  microseconds offset from UTC
            // if possible, return RexxInteger instead of String
#ifdef __REXX64__
            return new_integer(timestamp.timeZoneOffset);
#else
            timestamp.formatTimeZone(work);
            break;
#endif

        // unknown output format
        default:
            work[0] = style;
            reportException(Error_Incorrect_call_list, "TIME", IntegerOne, "CEFHLMNORST", new_string(work, 1));
            break;
    }

    // return as a string object
    return new_string(work);
}


BUILTIN(RANDOM)
{
    const size_t RANDOM_Min = 0;
    const size_t RANDOM_Max = 3;
    const size_t RANDOM_minimum = 1;
    const size_t RANDOM_maximum = 2;
    const size_t RANDOM_seed =    3;

    fix_args(RANDOM);

    RexxInteger *minimum;
    RexxInteger *maximum;
    // we need a special case here.  the interpretation of Random is such that
    // random() is NOT the same as Random(,).
    if (argcount == 2 && arg_omitted(RANDOM, minimum) && arg_omitted(RANDOM, maximum))
    {
        minimum = IntegerZero;
        maximum = new_integer(999);
    }
    else
    {
        minimum = optional_integer(RANDOM, minimum);
        maximum = optional_integer(RANDOM, maximum);
    }

    RexxInteger *seed = optional_integer(RANDOM, seed);

    // the activation handles random number generation
    return context->random(minimum, maximum, seed);
}


BUILTIN(XRANGE)
{
    const size_t XRANGE_Min = 0;
    const size_t XRANGE_Max = argcount;

    fix_args(XRANGE);

    char startchar, endchar;           // start and end positions
    typedef enum {START_END, CHAR_CLASS} arg_t;
    arg_t argumentType;                // character class or start/end range

    RexxString *first, *second, *result;
    RexxString::StringBuilder result_builder;
    size_t length, totalLength = 0;
    size_t XRANGE_arg;
    const char *characterClass;

    // we will need to know the total length of the result string
    // before we can begin to build it
    // if there are more than one or two args, we'll have to step through
    // all our args twice: first, to count the total string length, and
    // a second time to append all the pieces together
    typedef enum {CALC_LENGTH, BUILD_STRING} work_t;
    work_t mode = CALC_LENGTH;         // step one: calculate total length

    for (size_t loops = 1; loops <= 2; loops++)
    {
        XRANGE_arg = 0;
        // we want to enter our loop even if argcount is zero
        while (XRANGE_arg == 0 || XRANGE_arg < argcount)
        {
            // default start and end positions are the full range
            startchar = 0;
            endchar = (char)0xff;
            argumentType = START_END;

            // for each loop, we can accept either:
            // - no args
            // - first arg length 1, no second arg: a start byte only
            // - first arg length larger than 1, no second arg: a character class
            // - no first arg, second arg length 1: an end byte only
            // - first arg length 1, second arg length 1: both a start and an end byte

            XRANGE_arg++;
            if ((first = optional_string(XRANGE, arg)) != OREF_NULL)
            {
                // must be a single character or a character class name
                if (first->getLength() == 1)
                {
                    // single character means a start byte
                    startchar = first->getChar(0);
                }
                else
                {
                    // must be a character class name
                    argumentType = CHAR_CLASS;
                    if      (first->strCaselessCompare("alnum")) characterClass = RexxString::ALNUM;
                    else if (first->strCaselessCompare("alpha")) characterClass = RexxString::ALPHA;
                    else if (first->strCaselessCompare("blank")) characterClass = RexxString::BLANK;
                    else if (first->strCaselessCompare("cntrl")) characterClass = RexxString::CNTRL;
                    else if (first->strCaselessCompare("digit")) characterClass = RexxString::DIGIT;
                    else if (first->strCaselessCompare("graph")) characterClass = RexxString::GRAPH;
                    else if (first->strCaselessCompare("lower")) characterClass = RexxString::LOWER;
                    else if (first->strCaselessCompare("print")) characterClass = RexxString::PRINT;
                    else if (first->strCaselessCompare("punct")) characterClass = RexxString::PUNCT;
                    else if (first->strCaselessCompare("space")) characterClass = RexxString::SPACE;
                    else if (first->strCaselessCompare("upper")) characterClass = RexxString::UPPER;
                    else if (first->strCaselessCompare("xdigit")) characterClass = RexxString::XDIGIT;
                    else reportException(Error_Incorrect_call_pad_or_name, "XRANGE", new_integer(XRANGE_arg), first);

                }
            }
            if (argumentType == CHAR_CLASS)
            {
                // CNTRL contains a leading NUL character, so we calculate length here
                length = 1 + strlen(characterClass + 1);

                // just one character class arg?  we can finish this early
                if (mode == CALC_LENGTH && argcount == 1)
                {
                    return new_string(characterClass, length);
                }
                else if (mode == CALC_LENGTH)
                {
                    totalLength += length;
                }
                else // mode == BUILD_STRING
                {
                    result_builder.append(characterClass, length);
                }

                // if this was a character class, we won't have a second arg
                continue;
            }

            // if run out of args, endchar is already set to its default
            XRANGE_arg++;
            if ((second = optional_string(XRANGE, arg)) != OREF_NULL)
            {
                // must be a single character
                if (second->getLength() != 1)
                {
                    reportException(Error_Incorrect_call_pad, "XRANGE", new_integer(XRANGE_arg), second);
                }
                else
                {
                    // the single character is the end byte
                    endchar = second->getChar(0);
                }
            }

            length = 1 + (endchar < startchar ? 256 - startchar + endchar : endchar - startchar);

            // just two args?  we can finish this early
            if (mode == CALC_LENGTH && argcount <= 2)
            {
                // create a new string to build the result
                result = raw_string(length);
                result_builder.init(result);
                for (size_t i = 0; i < length; i++)
                {
                    // NOTE:  This depends on the fact that we are only inserting the
                    // least significant byte here, so the wrap situation is handled
                    // automatically.
                    result_builder.append(startchar++);
                }
                return result;
            }
            else if (mode == CALC_LENGTH)
            {
                totalLength += length;
            }
            else // mode == BUILD_STRING
            {
                for (size_t i = 0; i < length; i++)
                {
                    // NOTE:  This depends on the fact that we are only inserting the
                    // least significant byte here, so the wrap situation is handled
                    // automatically.
                    result_builder.append(startchar++);
                }
            }
        }

        if (mode == CALC_LENGTH)
        {
            // finished counting length, switch to building string
            mode = BUILD_STRING;       // step two: build the string

            // create a new string to build the result
            result = raw_string(totalLength);
            result_builder.init(result);
        }
    }

    // finished building string
    return result;
}


BUILTIN(SYMBOL)
{
    const size_t SYMBOL_Min = 1;
    const size_t SYMBOL_Max = 1;
    const size_t SYMBOL_name =    1;

    fix_args(SYMBOL);

    RexxString *name = required_string(SYMBOL, name);

    // get a retriever for this name
    RexxVariableBase *variable = VariableDictionary::getVariableRetriever(name);
    // a parsing failure is "BAD"
    if (variable == OREF_NULL)
    {
        return new_string("BAD");
    }
    // if this is a constant symbol, this is LIT
    else if (isString(variable))
    {
        return new_string("LIT");
    }
    else
    {
        // see if the variable exists in the context.  Non-set
        // variables are "LIT", set variables are "VAR"
        if (!variable->exists(context))
        {
            return new_string("LIT");
        }
        else
        {
            return new_string("VAR");
        }
    }
}


BUILTIN(VAR)
{
    const size_t VAR_Min = 1;
    const size_t VAR_Max = 1;
    const size_t VAR_name =    1;

    fix_args(VAR);
    RexxString *variable = required_string(VAR, name);

    // like SYMBOL, but this is a simple true/false return vallue
    RexxVariableBase *retriever = VariableDictionary::getVariableRetriever(variable);
    // invalid variable name is false
    if (retriever == OREF_NULL)
    {
        return TheFalseObject;
    }
    // a constant string is also false
    else if (isString(retriever))
    {
        return TheFalseObject;
    }
    else
    {
        // return the existance flag.
        return booleanObject(retriever->exists(context));
    }
}


BUILTIN(VALUE)
{
    const size_t VALUE_Min = 1;
    const size_t VALUE_Max = 3;
    const size_t VALUE_name =     1;
    const size_t VALUE_newValue = 2;
    const size_t VALUE_selector = 3;

    fix_args(VALUE);

    RexxString *variable = required_string(VALUE, name);
    RexxObject *newvalue = optional_argument(VALUE, newValue);
    RexxString *selector = optional_string(VALUE, selector);

    // if not there, we return a null string
    ProtectedObject result;


    // get the variable type
    StringSymbolType variableType = variable->isSymbol();
    bool assignable = variableType == STRING_NAME || variableType == STRING_STEM || variableType == STRING_COMPOUND_NAME;

    // no selector means we're looking up a local variable
    if (selector == OREF_NULL)
    {
        Protected<RexxVariableBase> retriever = VariableDictionary::getVariableRetriever(variable);
        // this could an invalid name, or we might be trying to assign a value to a non-variable
        // symbol.
        if (retriever == OREF_NULL || (newvalue != OREF_NULL && !assignable))
        {
            reportException(Error_Incorrect_call_symbol, "VALUE", IntegerOne, variable);
        }
        // get the variable value
        result = retriever->getValue(context);
        // given a new value?  Assign that
        if (newvalue != OREF_NULL)
        {
            retriever->assign(context, newvalue);
        }
        return result;
    }
    // a NULL string selector is .environment...whatever that means
    else if (selector->getLength() == 0)
    {

        result = (RexxObject *)TheEnvironment->entry(variable);
        // the value is .VARIABLE if not found
        if (result == OREF_NULL)
        {
            result = variable->upper()->concatToCstring(".");
        }
        // do the set also
        if (newvalue != OREF_NULL)
        {
            TheEnvironment->setEntry(variable, newvalue);
        }
        return result;
    }
    // an external selector value
    else
    {
    // The selector ENVIRONMENT means the same on every system, so we handle this here
        if (selector->strCaselessCompare("ENVIRONMENT"))
        {
            Protected<RexxString> name = variable;

            FileNameBuffer buffer;

            // get the variable as set as a result
            SystemInterpreter::getEnvironmentVariable(name->getStringData(), buffer);
            result = new_string(buffer);

            // set the variable if we have a new value
            if (newvalue != OREF_NULL)
            {
                // .nil is special, it removes the variable
                if (newvalue == TheNilObject)
                {
                    SystemInterpreter::setEnvironmentVariable(name->getStringData(), NULL);
                }
                // we need a string value for the set.
                else
                {
                    Protected<RexxString> stringValue = stringArgument(newvalue, ARG_TWO);

                    SystemInterpreter::setEnvironmentVariable(name->getStringData(), stringValue->getStringData());
                }
            }
            return result;
        }

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


BUILTIN(ABS)
{
    const size_t ABS_Min = 1;
    const size_t ABS_Max = 1;
    const size_t ABS_n =   1;

    fix_args(ABS);

    RexxObject *argument = get_arg(ABS, n);
    // and integer value can be processed quickly
    if (isInteger(argument))
    {
        return ((RexxInteger *)argument)->abs();
    }
    else if (isNumberString(argument))
    {
        return((NumberString *)argument)->abs();
    }

    // force to a string object and perform it on that
    RexxString *n = required_string(ABS, n);
    return n->abs();
}


BUILTIN(SIGN)
{
    const size_t SIGN_Min = 1;
    const size_t SIGN_Max = 1;
    const size_t SIGN_n =   1;

    fix_args(SIGN);

    RexxObject *argument = get_arg(SIGN, n);

    // check for the numeric forms and use them directly, otherwise
    // do this on the string value
    if (isInteger(argument))
    {
        return((RexxInteger *)argument)->sign();
    }
    else if (isNumberString(argument))
    {
        return((NumberString *)argument)->Sign();
    }

    RexxString *n = required_string(SIGN, n);
    return n->sign();
}


BUILTIN(FORMAT)
{
    const size_t FORMAT_Min = 1;
    const size_t FORMAT_Max = 5;
    const size_t FORMAT_number = 1;
    const size_t FORMAT_before = 2;
    const size_t FORMAT_after =  3;
    const size_t FORMAT_expp =   4;
    const size_t FORMAT_expt =   5;

    fix_args(FORMAT);

    RexxString *number = required_string(FORMAT, number);
    RexxInteger *before = optional_integer(FORMAT, before);
    RexxInteger *after = optional_integer(FORMAT, after);
    RexxInteger *expp = optional_integer(FORMAT, expp);
    RexxInteger *expt = optional_integer(FORMAT, expt);

    return number->format(before, after, expp, expt);
}


BUILTIN(MAX)
{
    const size_t MAX_Min = 1;
    const size_t MAX_Max = argcount;
    const size_t MAX_target = 1;

    check_args(MAX);

    RexxObject *argument = get_arg(MAX, target);

    // check for the numeric forms and use them directly, otherwise
    // do this on the string value
    if (isInteger(argument))
    {
        return((RexxInteger *)argument)->Max(stack->arguments(argcount - 1), argcount - 1);
    }
    else if (isNumberString(argument))
    {
        return((NumberString *)argument)->Max(stack->arguments(argcount - 1), argcount - 1);
    }

    // start with the string form
    RexxString *target = required_string(MAX, target);

    // we just just one fewer of the arguments
    return target->Max(stack->arguments(argcount - 1), argcount - 1);
}


BUILTIN(MIN)
{
    const size_t MIN_Min = 1;
    const size_t MIN_Max = argcount;
    const size_t MIN_target = 1;

    check_args(MIN);

    RexxObject *argument = get_arg(MIN, target);

    // check for the numeric forms and use them directly, otherwise
    // do this on the string value
    if (isInteger(argument))
    {
        return((RexxInteger *)argument)->Min(stack->arguments(argcount - 1), argcount - 1);
    }
    else if (isNumberString(argument))
    {
        return((NumberString *)argument)->Min(stack->arguments(argcount - 1), argcount - 1);
    }

    // use the string form
    RexxString *target = required_string(MIN, target);

    return target->Min(stack->arguments(argcount - 1), argcount - 1);
}


BUILTIN(SOURCELINE)
{
    const size_t SOURCELINE_Min = 0;
    const size_t SOURCELINE_Max = 1;
    const size_t SOURCELINE_n =   1;

    fix_args(SOURCELINE);

    // get the effective source object.  If we're in an interpret context, this will
    // be the one of our caller.
    PackageClass *package = context->getEffectivePackageObject();
    // and get the program size
    size_t size = package->sourceSize();
    if (argcount == 1)
    {
        // get the line number in binary
        size_t line_number = required_integer(SOURCELINE, n)->getValue();
        // and it must be a positive integer
        positive_integer((ssize_t)line_number, SOURCELINE, IntegerOne);
        // out of range is an error
        if (line_number > size)
        {
            reportException(Error_Incorrect_call_sourceline, line_number, size);
        }
        // get the specific line from the source
        return package->getLine(line_number);
    }
    // no argument just returns the size
    else
    {
        return new_integer(size);
    }
}


BUILTIN(TRACE)
{
    const size_t TRACE_Min = 0;
    const size_t TRACE_Max = 1;
    const size_t TRACE_setting = 1;

    fix_args(TRACE);

    RexxString *setting = optional_string(TRACE, setting);
    // get the existing setting before setting a new one
    RexxString *result = context->traceSetting();
    // if we have a new value, then set it
    if (setting != OREF_NULL)
    {
        context->setTrace(setting);
    }
    return result;
}

/**
 * Check to see if a stream name is a queue
 *
 * @param name   The name to check.
 *
 * @return True if this is a QUEUE value, false otherwise.
 */
bool check_queue(RexxString *name)
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


BUILTIN(LINEIN)
{
    const size_t LINEIN_Min = 0;
    const size_t LINEIN_Max = 3;
    const size_t LINEIN_name =   1;
    const size_t LINEIN_line =   2;
    const size_t LINEIN_count =  3;

    fix_args(LINEIN);

    RexxString *name = optional_string(LINEIN, name);
    RexxObject *line = optional_big_integer(LINEIN, line);
    RexxObject *count = optional_big_integer(LINEIN, count);

    // if this is linein request for "QUEUE:, this is really a pull
    // operation.
    if (check_queue(name))
    {
        RexxString *result;

        // handle both via the exit and the actual queue object
        if (context->getActivity()->callPullExit(context, result))
        {
            RexxObject *stream = context->getLocalEnvironment(STDQUE);
            ProtectedObject result;
            // we do this using a LINEIN method
            return stream->sendMessage(LINEIN, result);
        }
        return result;
    }
    else
    {
        bool added = false;
        Protected<RexxString> fullname;
        // get a stream for this name
        RexxObject *stream = context->resolveStream(name, true, fullname, &added);
        ProtectedObject result;
        switch (argcount)
        {
            // process based on the arguments
            // NAME only
            case 0:
            case 1:
                return stream->sendMessage(LINEIN, result);
                break;
            // start position specified
            case 2:
                return stream->sendMessage(LINEIN, line, result);
                break;
            // start and count specified
            case 3:
                return stream->sendMessage(LINEIN, line, count, result);
                break;
        }
    }
    return GlobalNames::NULLSTRING;  // should never happen
}


BUILTIN(CHARIN)
{
    const size_t CHARIN_Min = 0;
    const size_t CHARIN_Max = 3;
    const size_t CHARIN_name =   1;
    const size_t CHARIN_start =  2;
    const size_t CHARIN_count =  3;

    fix_args(CHARIN);

    RexxString *name = optional_string(CHARIN, name);
    RexxObject *position = optional_big_integer(CHARIN, start);
    RexxObject *count = optional_big_integer(CHARIN, count);

    // queue is not allowed for CHARIN
    if (check_queue(name))
    {
        reportException(Error_Incorrect_call_queue_no_char, CHARIN);
    }

    // resolve the stream name and send it the appropriate message
    bool added = false;
    Protected<RexxString> fullname;
    RexxObject *stream = context->resolveStream(name, true, fullname, &added);
    ProtectedObject result;
    switch (argcount)
    {
        case 0:
        case 1:
            return stream->sendMessage(CHARIN, result);
            break;
        case 2:
            return stream->sendMessage(CHARIN, position, result);
            break;
        case 3:
            return stream->sendMessage(CHARIN, position, count, result);
            break;
    }
    return GlobalNames::NULLSTRING;
}


BUILTIN(LINEOUT)
{
    const size_t LINEOUT_Min = 0;
    const size_t LINEOUT_Max = 3;
    const size_t LINEOUT_name =   1;
    const size_t LINEOUT_string = 2;
    const size_t LINEOUT_line =   3;

    fix_args(LINEOUT);

    RexxString *name = optional_string(LINEOUT, name);
    RexxString *string = optional_string(LINEOUT, string);
    RexxObject *line = optional_big_integer(LINEOUT, line);

    // lineout is allowed to the queue
    if (check_queue(name))
    {

        if (context->getActivity()->callPushExit(context, string, Activity::QUEUE_FIFO))
        {
            // lineout always queues to the queue
            if (string != OREF_NULL)
            {
                RexxObject *stream = context->getLocalEnvironment(STDQUE);
                ProtectedObject result;
                return stream->sendMessage(QUEUE, string, result);
            }
            else
            {
                return IntegerZero;
            }
        }
    }
    else
    {
        bool added;
        Protected<RexxString> fullName;
        // resolve the stream name and send the message based on the arguments
        RexxObject *stream = context->resolveStream(name, false, fullName, &added);
        ProtectedObject result;
        switch (argcount)
        {
            case 0:
            case 1:
                return stream->sendMessage(LINEOUT, result);
                break;
            case 2:
                return stream->sendMessage(LINEOUT, string, result);
                break;
            case 3:
                return stream->sendMessage(LINEOUT, string, line, result);
                break;
        }
    }
    return GlobalNames::NULLSTRING;
}


BUILTIN(CHAROUT)
{
    const size_t CHAROUT_Min = 0;
    const size_t CHAROUT_Max = 3;
    const size_t CHAROUT_name =   1;
    const size_t CHAROUT_string = 2;
    const size_t CHAROUT_start =  3;

    fix_args(CHAROUT);

    RexxString *name = optional_string(CHAROUT, name);
    RexxString *string = optional_string(CHAROUT, string);
    RexxObject *position = optional_big_integer(CHAROUT, start);

    // queues are not allowed with charout
    if (check_queue(name))
    {
        reportException(Error_Incorrect_call_queue_no_char, CHAROUT);
    }

    bool added;
    Protected<RexxString> fullname;
    // resolve the stream name
    RexxObject *stream = context->resolveStream(name, false, fullname, &added);
    ProtectedObject result;
    switch (argcount)
    {
        case 0:
        case 1:
            return stream->sendMessage(CHAROUT, result);
            break;
        case 2:
            return stream->sendMessage(CHAROUT, string, result);
            break;
        case 3:
            return stream->sendMessage(CHAROUT, string, position, result);
            break;
    }
    return GlobalNames::NULLSTRING;
}


BUILTIN(LINES)
{
    const size_t LINES_Min = 0;
    const size_t LINES_Max =    2;
    const size_t LINES_name =   1;
    const size_t LINES_option = 2;

    fix_args(LINES);

    RexxString *name = optional_string(LINES, name); /* get the string name               */
    RexxString *option = optional_string(LINES, option);
    RexxObject *result;
    ProtectedObject resultObj;
    int opt = 'N';

    if (option != OREF_NULL)
    {
        // get the first character
        opt = toupper(option->getChar(0));
        if (opt != 'C' && opt != 'N')
        {
            reportException(Error_Incorrect_call_list, "ARG", IntegerTwo, "NC", option);
        }
    }

    // for the queue, return the count of items in the queue
    if (check_queue(name))
    {
        RexxObject *stream = context->getLocalEnvironment(STDQUE);
        result = stream->sendMessage(QUEUED, resultObj);
    }
    else
    {
        bool added;
        Protected<RexxString> fullname;
        // resolve the stream
        RexxObject *stream = context->resolveStream(name, true, fullname, &added);
        result = stream->sendMessage(LINES, option, resultObj);
    }


    // for compatibility this needs to only return 0 or 1
    if (opt == 'N')
    {
        wholenumber_t count = 0;
        if (result->numberValue(count))
        {
            return (count != 0) ? IntegerOne : IntegerZero;
        }
        return result;    // not sure what this, just return directly
    }
    else
    {
        return result;
    }
}


BUILTIN(CHARS)
{
    const size_t CHARS_Min = 0;
    const size_t CHARS_Max = 1;
    const size_t CHARS_name =   1;

    fix_args(CHARS);

    RexxString *name = optional_string(CHARS, name);

    // queue not allowed with chars()
    if (check_queue(name))
    {
        reportException(Error_Incorrect_call_queue_no_char, CHARS);
    }

    // resolve the stream and send it the CHARS message
    bool added;
    Protected<RexxString> fullname;
    RexxObject *stream = context->resolveStream(name, true, fullname, &added);
    ProtectedObject result;
    return stream->sendMessage(CHARS, result);
}


BUILTIN(STREAM)
{
    const size_t STREAM_Min = 1;
    #undef STREAM_Max                      /* already defined in AIX            */
    const size_t STREAM_Max = 3;
    const size_t STREAM_name =      1;
    const size_t STREAM_operation = 2;
    const size_t STREAM_command =   3;

    const size_t STREAM_STATUS =      'S';
    const size_t STREAM_DESCRIPTION = 'D';
    const size_t STREAM_COMMAND =     'C';

    fix_args(STREAM);

    RexxString *name = required_string(STREAM, name);

    // null string not allowed for the name
    if (name->getLength() == 0)
    {
        reportException(Error_Incorrect_call_stream_name, STREAM, name);
    }

    RexxString *action = optional_string(STREAM, operation);
    RexxString *command = optional_string(STREAM, command);

    char action_char = STREAM_STATUS;
    // decode the action
    if (action != OREF_NULL)
    {
        // null string is not a valid option
        if (action->getLength() == 0)
        {
            reportException(Error_Incorrect_call_list, "STREAM", IntegerTwo, "SDC", action);
        }
        action_char = toupper(action->getChar(0));
    }

    switch (action_char)
    {
        // stream(name, 'S')
        case STREAM_STATUS:
            {
                // no third argument allowed with status
                if (argcount > 2)
                {
                    reportException(Error_Incorrect_call_maxarg, STREAM, IntegerTwo);
                }

                Protected<RexxString> fullname;
                // get the stream object and get the state
                RexxObject *stream = context->resolveStream(name, true, fullname, NULL);
                ProtectedObject result;
                return stream->sendMessage(STATE, result);
                break;
            }

        // stream(name, 'D')
        case STREAM_DESCRIPTION:
            {
                // only 2 args allowed here also
                if (argcount > 2)
                {
                    reportException(Error_Incorrect_call_maxarg, STREAM, IntegerTwo);
                }

                Protected<RexxString> fullname;
                RexxObject *stream = context->resolveStream(name, true, fullname, NULL);
                ProtectedObject result;
                return stream->sendMessage(DESCRIPTION, result);
                break;
            }

        // stream(name, 'C', command)
        case STREAM_COMMAND:
            {
                //the third argument is required here
                if (argcount < 3)
                {
                    reportException(Error_Incorrect_call_minarg, STREAM, IntegerThree);
                }
                ProtectedObject p(command);

                // I have to check the command twice because in the Rexx methods (i.g. query_exists)
                // I don't have access to the activation and thus not to the streamtable.
                // It's also not possible to pass context as the second argument because
                // stream is a MethodClass and USE ARG RexxActivation is not possible
                RexxString *command_upper = command->upper();
                ProtectedObject p1(command_upper);

                // an open request
                if (command_upper->wordPos(new_string("OPEN"), OREF_NULL)->getValue() > 0)
                {
                    Protected<RexxString> fullname;
                    bool added;
                    RexxObject *stream = context->resolveStream(name, true, fullname, &added);
                    ProtectedObject resultObj;
                    RexxString *result = (RexxString *)stream->sendMessage(COMMAND, command, resultObj);
                    // if open failed, remove the stream object from stream table again
                    if (!result->strCompare("READY:"))
                    {
                        context->getStreams()->remove(fullname);
                    }
                    return result;
                }
                // a close request
                else if (command_upper->wordPos(new_string("CLOSE"), OREF_NULL)->getValue() > 0)
                {
                    bool added;
                    Protected<RexxString> fullname;
                    RexxObject *stream = context->resolveStream(name, true, fullname, &added);
                    ProtectedObject resultObj;
                    RexxString *result = (RexxString *)stream->sendMessage(COMMAND, command, resultObj);
                    // remove this from the table after the close
                    context->getStreams()->remove(fullname);
                    return result;
                }
                // these are real operations that might cause an implicit open
                else if (command_upper->wordPos(new_string("SEEK"), OREF_NULL)->getValue() > 0 ||
                    command_upper->wordPos(new_string("POSITON"), OREF_NULL)->getValue() > 0)
                {
                    bool added;
                    Protected<RexxString> fullname;
                    RexxObject *stream = context->resolveStream(name, true, fullname, &added);
                    // this is a real operation, so pass along to the stream object
                    ProtectedObject resultObj;
                    RexxString *result = (RexxString *)stream->sendMessage(COMMAND, command, resultObj);
                    return result;
                }
                // all other commands just pass to the resolved stream object
                else
                {
                    Protected<RexxString> fullname;
                    RexxObject *stream = context->resolveStream(name, true, fullname, NULL);
                    ProtectedObject result;
                    return stream->sendMessage(COMMAND, command, result);
                }
                break;
            }

        default:
            reportException(Error_Incorrect_call_list, "STREAM", IntegerTwo, "SDC", action);
            break;
    }
    return OREF_NULL;
}


BUILTIN(QUEUED)
{
    const size_t QUEUED_Min = 0;
    const size_t QUEUED_Max = 0;

    check_args(QUEUED);

    RexxInteger  *queuesize;

    // see if the exit handles this, otherwise send a message to the current queue
    if (context->getActivity()->callQueueSizeExit(context, queuesize))
    {
        RexxObject *queue = context->getLocalEnvironment(STDQUE);
        ProtectedObject result;
        return queue->sendMessage(QUEUED, result);
    }
    else
    {
        return queuesize;
    }
}


BUILTIN(CONDITION)
{
    const size_t CONDITION_Min = 0;
    const size_t CONDITION_Max = 1;
    const size_t CONDITION_option = 1;

    fix_args(CONDITION);

    RexxString *option = optional_string(CONDITION, option);

    int   style = 'I';
    // do we have an explicit option?
    if (option != OREF_NULL)
    {
        // null string is not a valid option
        if (option->getLength() == 0)
        {
            reportException(Error_Incorrect_call_list, "CONDITION", IntegerOne, "ACDIORS", option);
        }

        style = toupper(option->getChar(0));
    }

    // get the current trapped condition
    DirectoryClass *conditionobj = context->getConditionObj();

    switch (style)
    {
        // condition('A'dditional)
        case 'A':
            if (conditionobj != OREF_NULL)
            {
                RexxObject *result = (RexxObject *)conditionobj->get(ADDITIONAL);
                // return either .nil or the additional information
                if (result == OREF_NULL)
                {
                    return TheNilObject;
                }
                else
                {
                    return (RexxObject *)result;
                }
            }
            else
            {
                return TheNilObject;
            }
            break;

        // condition('I'struction).  Returns either SYNTAX or CALL, or a null string if no condition
        case 'I':
            if (conditionobj != OREF_NULL)
            {
                return (RexxObject *)conditionobj->get(INSTRUCTION);
            }
            break;

        // condition('D'escription)
        case 'D':
            if (conditionobj != OREF_NULL)
            {
                // get the description from the object, return a null string if not there
                RexxObject *result = (RexxObject *)conditionobj->get(DESCRIPTION);
                if (result == OREF_NULL)
                {
                    result = GlobalNames::NULLSTRING;
                }
                return result;
            }
            break;

        // condition('C'ondition)
        case 'C':
            // if we have a condition object, return that value
            if (conditionobj != OREF_NULL)
            {
                return (RexxObject *)conditionobj->get(CONDITION);
            }
            break;

        // condition('O'bject)
        case 'O':
            // if we have a condition, return a copy of the condition object
            if (conditionobj != OREF_NULL)
            {
                return (RexxObject *)conditionobj->copy();
            }
            return TheNilObject;

        // condition('S'tate)
        case 'S':
            // get the current trap state from the condition object if we have one
            if (conditionobj != OREF_NULL)
            {
                return context->trapState((RexxString *)conditionobj->get(CONDITION));
            }
            break;

        // condition('R'eset)
        case 'R':
            // clear the current condition object
            context->setConditionObj(OREF_NULL);
            return GlobalNames::NULLSTRING;
            break;

        // an unknown option
        default:
            reportException(Error_Incorrect_call_list, "CONDITION", IntegerOne, "ACDIORS", option);
            break;
    }

    // most of the options fall through to here if there is no current condition.
    return GlobalNames::NULLSTRING;
}


BUILTIN(CHANGESTR)
{
    const size_t CHANGESTR_Min = 3;
    const size_t CHANGESTR_Max = 4;
    const size_t CHANGESTR_needle =     1;
    const size_t CHANGESTR_haystack =   2;
    const size_t CHANGESTR_newneedle =  3;
    const size_t CHANGESTR_count =      4;

    fix_args(CHANGESTR);

    RexxString *needle = required_string(CHANGESTR, needle);
    RexxString *haystack = required_string(CHANGESTR, haystack);
    RexxString *newneedle = required_string(CHANGESTR, newneedle);
    RexxInteger *count = optional_integer(CHANGESTR, count);

    return haystack->changeStr(needle, newneedle, count);
}


BUILTIN(COUNTSTR)
{
    const size_t COUNTSTR_Min = 2;
    const size_t COUNTSTR_Max = 2;
    const size_t COUNTSTR_needle =     1;
    const size_t COUNTSTR_haystack =   2;

    fix_args(COUNTSTR);

    RexxString *needle = required_string(COUNTSTR, needle);
    RexxString *haystack = required_string(COUNTSTR, haystack);

    return haystack->countStrRexx(needle);
}


BUILTIN(RXFUNCADD)
{
    const size_t RXFUNCADD_Min = 2;
    const size_t RXFUNCADD_Max = 3;
    const size_t RXFUNCADD_name =   1;
    const size_t RXFUNCADD_module = 2;
    const size_t RXFUNCADD_proc =   3;

    fix_args(RXFUNCADD);

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


BUILTIN(RXFUNCDROP)
{
    const size_t RXFUNCDROP_Min = 1;
    const size_t RXFUNCDROP_Max = 1;
    const size_t RXFUNCDROP_name =   1;

    fix_args(RXFUNCDROP);

    RexxString *name = required_string(RXFUNCDROP, name);

    // hand this off to the package manager.
    return PackageManager::dropRegisteredRoutine(name);
}


BUILTIN(RXFUNCQUERY)
{
    const size_t RXFUNCQUERY_Min = 1;
    const size_t RXFUNCQUERY_Max = 1;
    const size_t RXFUNCQUERY_name =   1;

    fix_args(RXFUNCQUERY);

    RexxString *name = required_string(RXFUNCQUERY, name);

    // hand this off to the package manager.
    return PackageManager::queryRegisteredRoutine(name);
}


BUILTIN(RXQUEUE)
{
    const size_t RXQUEUE_Min = 1;
    const size_t RXQUEUE_Max = 2;
    const size_t RXQUEUE_option = 1;
    const size_t RXQUEUE_name =   2;

    fix_args(RXQUEUE);

    RexxString *option = required_string(RXQUEUE, option);
    RexxString *queueName = optional_string(RXQUEUE, name);
    ProtectedObject result;

    switch (toupper(option->getChar(0)))
    {
        // 'G'et the current queue name
        case 'G':
        {
            // the queue name is not allowed with the 'G'et option
            if (queueName != OREF_NULL)
            {
                reportException(Error_Incorrect_call_maxarg, "RXQUEUE", IntegerOne);
            }
            RexxObject *queue = context->getLocalEnvironment(STDQUE);
            return queue->sendMessage(GlobalNames::GET, result);
        }

        // 'C'reate a named queue
        case 'C':
        {
            RexxObject *t = OREF_NULL;   // required for the findClass call
            // we need the RexxQueue class for this
            RexxClass *rexxQueue = TheRexxPackage->findClass(REXXQUEUE, t);

            // if no queue name specified, we allow a name to be
            // created for us
            if (queueName == OREF_NULL)
            {
                return rexxQueue->sendMessage(new_string("CREATE"), result);
            }
            else
            {
                // this must be a valid symbol
                if (queueName->isSymbol() == STRING_BAD_VARIABLE)
                {
                    reportException(Error_Incorrect_call_symbol, new_string("RXQUEUE"), IntegerTwo, queueName);
                }
                return rexxQueue->sendMessage(new_string("CREATE"), queueName, result);
            }
        }

        // 'S'et a new queue name
        case 'S':
        {
            // queueName is required
            if (queueName == OREF_NULL)
            {
                reportException(Error_Incorrect_call_minarg, "RXQUEUE", IntegerTwo);
            }
            // give the exit a pass at this
            context->getActivity()->callQueueNameExit(context, queueName);
            // this must be a valid symbol
            if (queueName->isSymbol() == STRING_BAD_VARIABLE)
            {
                reportException(Error_Incorrect_call_symbol, new_string("RXQUEUE"), IntegerTwo, queueName);
            }
            RexxObject *queue = context->getLocalEnvironment(STDQUE);
            return queue->sendMessage(new_string("SET"), queueName, result);
        }

        // 'O'pen a new queue name...creates if needed
        case 'O':
        {
            // queueName is required
            if (queueName == OREF_NULL)
            {
                reportException(Error_Incorrect_call_minarg, "RXQUEUE", IntegerTwo);
            }
            // we need the RexxQueue class for this
            RexxObject *t = OREF_NULL;   // required for the findClass call

            RexxClass *rexxQueue = TheRexxPackage->findClass(REXXQUEUE, t);
            // this must be a valid symbol
            if (queueName->isSymbol() == STRING_BAD_VARIABLE)
            {
                reportException(Error_Incorrect_call_symbol, new_string("RXQUEUE"), IntegerTwo, queueName);
            }
            return rexxQueue->sendMessage(new_string("OPEN"), queueName, result);
        }

        // 'E'xists
        case 'E':
        {
            // queueName is required
            if (queueName == OREF_NULL)
            {
                reportException(Error_Incorrect_call_minarg, "RXQUEUE", IntegerTwo);
            }
            RexxObject *t = OREF_NULL;   // required for the findClass call

            // we need the RexxQueue class for this
            RexxClass *rexxQueue = TheRexxPackage->findClass(REXXQUEUE,t);
            // this must be a valid symbol
            if (queueName->isSymbol() == STRING_BAD_VARIABLE)
            {
                reportException(Error_Incorrect_call_symbol, new_string("RXQUEUE"), IntegerTwo, queueName);
            }
            return rexxQueue->sendMessage(new_string("EXISTS"), queueName, result);
        }

        // 'D'elete
        case 'D':
        {
            // queueName is required
            if (queueName == OREF_NULL)
            {
                reportException(Error_Incorrect_call_minarg, "RXQUEUE", IntegerTwo);
            }
            RexxObject *t = OREF_NULL;   // required for the findClass call

            // we need the RexxQueue class for this
            RexxClass *rexxQueue = TheRexxPackage->findClass(REXXQUEUE, t);
            // this must be a valid symbol
            if (queueName->isSymbol() == STRING_BAD_VARIABLE)
            {
                reportException(Error_Incorrect_call_symbol, new_string("RXQUEUE"), IntegerTwo, queueName);
            }
            return rexxQueue->sendMessage(new_string("DELETE"), queueName, result);
        }

        default:
            reportException(Error_Incorrect_call_list, "RXQUEUE", IntegerOne, "CDEGOS", option);
    }

    return OREF_NULL;
}


BUILTIN(SETLOCAL)
{
    const size_t SETLOCAL_Min = 0;
    const size_t SETLOCAL_Max = 0;

    check_args(SETLOCAL);

    // the external environment implements this
    return SystemInterpreter::pushEnvironment(context);
}


BUILTIN(ENDLOCAL)
{
    const size_t ENDLOCAL_Min = 0;
    const size_t ENDLOCAL_Max = 0;

    check_args(ENDLOCAL);

    // the external environment implements this
    return SystemInterpreter::popEnvironment(context);
}


/**
 * Qualify a stream name.
 */
BUILTIN(QUALIFY)
{
    const size_t QUALIFY_Min = 1;
    const size_t QUALIFY_Max = 1;
    const size_t QUALIFY_name =  1;

    check_args(QUALIFY);

    RexxString *name = optional_string(QUALIFY, name);

    QualifiedName qualified_name(name->getStringData());
    return new_string(qualified_name);
}

// the following builtin function table must maintain the same order
// as the BuiltinCode type defined by the RexxToken class.
pbuiltin LanguageParser::builtinTable[] =
{
    NULL,    // NULL first entry as dummy
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
    &builtin_function_RXQUEUE          ,
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
    &builtin_function_QUALIFY          ,
};

