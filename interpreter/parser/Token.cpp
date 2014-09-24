/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2015 Rexx Language Association. All rights reserved.    */
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
/* Primitive Translator Token Class                                           */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "Token.hpp"
#include "LanguageParser.hpp"
#include "GlobalNames.hpp"

/**
 * Create a new Token object.
 *
 * @param size   The size of the object.
 *
 * @return Storage for a new instance of a Token.
 */
void *RexxToken::operator new(size_t size)
{
    return new_object(size, T_Token);
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxToken::live(size_t liveMark)
{
    memory_mark(stringValue);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxToken::liveGeneral(MarkReason reason)
{
    memory_mark_general(stringValue);
}


/**
 * Check and update this token for the special assignment forms
 * (+=, -=, etc.).
 *
 * @param source The source for the original operator token.
 */
void RexxToken::checkAssignment(LanguageParser *parser, RexxString *newValue)
{
    // check if the next character is a special assignment shortcut
    if (parser->nextSpecial('=', tokenLocation))
    {
        // this is a special type, which uses the same subtype.
        classId = TOKEN_ASSIGNMENT;
        // this is the new string value of the token
        stringValue = newValue;
    }
}


/**
 * Determine a Token's operator precedence.
 *
 * @return A numeric ranking for operator characters.
 */
int RexxToken::precedence()
{
    // the subclass determines what type of operator this is.
    switch (subclass)
    {
        default:
            return 0;                         // this is the bottom of the heap
            break;

        case OPERATOR_OR:
        case OPERATOR_XOR:
            return 1;                         // various OR types are next
            break;

        case OPERATOR_AND:
            return 2;                         // AND operator ahead of ORs
            break;

        case OPERATOR_EQUAL:                  // comparisons are all together
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
            return 3;
            break;

        case OPERATOR_ABUTTAL:                // concatentates
        case OPERATOR_CONCATENATE:
        case OPERATOR_BLANK:
            return 4;
            break;

        case OPERATOR_PLUS:                   // plus and minus
        case OPERATOR_SUBTRACT:
            return 5;
            break;

        case OPERATOR_MULTIPLY:               // multiply and divide versions
        case OPERATOR_DIVIDE:
        case OPERATOR_INTDIV:
        case OPERATOR_REMAINDER:
            return 6;
            break;

        case OPERATOR_POWER:
            return 7;                         // almost the top of the heap
            break;

        case OPERATOR_BACKSLASH:
            return 8;                         // NOT is the top honcho
            break;
    }
}


/**
 * Test if a token qualifies as a terminator in the current
 * parsing context.
 *
 * @param terminators
 *               The list of terminators.
 *
 * @return true if this is a terminator, false if it doesn't match
 *         one of the terminator classes.
 */
bool RexxToken::isTerminator(int terminators)
{
    // process based on terminator class
    switch (classId)
    {
        case  TOKEN_EOC:                     // end-of-clause is always a terminator
        {
            return true;
        }
        case  TOKEN_COMMA:                   // commas are also always subexpression terminators
        {
            return true;
        }
        case  TOKEN_RIGHT:                   // found a right paren
        {
            if (terminators&TERM_RIGHT)
            {
                return true;
            }
            break;
        }
        case  TOKEN_SQRIGHT:                 // closing square bracket?
        {
            if (terminators&TERM_SQRIGHT)
            {
                return true;
            }
            break;
        }
        case  TOKEN_SYMBOL:                  // the token is a symbol...need to check on keyword terminators
        {
            // keyword terminators all set a special keyword flag.  We only check
            // symbols that are simple variables.
            if (terminators&TERM_KEYWORD && isSimpleVariable())
            {
                // map the keyword token to a key word code.  This are generally
                // keyword options on DO/LOOP, although THEN and WHEN are also terminators
                switch (subKeyword())
                {
                    case SUBKEY_TO:
                    {
                        if (terminators&TERM_TO)
                        {
                            return true;
                        }
                        break;
                    }
                    case SUBKEY_BY:
                    {
                         if (terminators&TERM_BY)
                         {
                             return true;
                         }
                         break;
                    }
                    case SUBKEY_FOR:
                    {
                        if (terminators&TERM_FOR)
                        {
                            return true;
                        }
                        break;
                    }
                    case SUBKEY_WHILE:           // a single terminator type picks up both
                    case SUBKEY_UNTIL:           // while and until
                    {
                        if (terminators&TERM_WHILE)
                        {
                            return true;
                        }
                        break;
                    }
                    case SUBKEY_WITH:            // WITH keyword in PARSE value
                    {
                        if (terminators&TERM_WITH)
                        {
                            return true;
                        }
                        break;
                    }
                    case SUBKEY_THEN:            // THEN subkeyword from IF or WHEN
                    {
                        if (terminators&TERM_THEN)
                        {
                            return true;
                        }
                        break;
                    }
                    default:                     // not a terminator type
                        break;
                }
            }
        }
        default:
            break;
    }
    return false;                    // no terminator found
}


/**
 * Upper case the string value.
 *
 * @return The upper case string value of the token.
 */
RexxString *RexxToken::upperValue()
{
    return stringValue->upper();
}


/**
 * Get a display value for a token.  For all symbols,
 * literals, and operators, this is just the string value
 * already set in the token.  For others, we generate
 * a string object automatically.
 *
 * @return The string display value of the token.
 */
RexxString *RexxToken::displayValue()
{
    // get the token string value.  Not all tokens have an assigned
    // string value, so we generate it here for the error.
    if (stringValue != OREF_NULL)
    {
        return stringValue;
    }

    // some tokens don't directly have a string value...we can provide one here
    switch (classId)
    {
        // blank operator
        case TOKEN_BLANK:
            return new_string(" ", 1);

        // end of clause...just use a semicolon, even though it might be
        // a linend.
        case TOKEN_EOC:
            return new_string(";", 1);

        // comma
        case TOKEN_COMMA:
            return new_string(",", 1);

        case TOKEN_LEFT:
            return new_string("(", 1);

        case TOKEN_RIGHT:
            return new_string(")", 1);

        case TOKEN_SQLEFT:
            return new_string("[", 1);

        case TOKEN_SQRIGHT:
            return new_string("]", 1);

        case TOKEN_COLON:
            return new_string(":", 1);

        case TOKEN_TILDE:
            return new_string("~", 1);

        case TOKEN_DTILDE:
            return new_string("~~", 2);

        case TOKEN_DCOLON:
            return new_string("::", 2);

        // token we don't have an answer for...just use a null string
        default:
            return (RexxString *)GlobalNames::NULLSTRING;
    }
}
