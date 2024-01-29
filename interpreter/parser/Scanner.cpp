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
/* Scanner portion of the REXX Source File Class                              */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "LanguageParser.hpp"
#include "GlobalNames.hpp"

#include <stdio.h>
#include <ctype.h>


/*********************************************************************
*  The following table detects alphanumeric characters and           *
*  special characters that can be part of an REXX symbol.            *
*  The table also convert lower case letters to upper case.          *
*********************************************************************/
int LanguageParser::characterTable[]={
#ifdef EBCDIC
 // This table was built using the IBM-1047 code page. It should be
 // universal across all EBCDIC code pages!
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /*            */
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /*            */
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /*            */
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /*            */
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /*            */
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /*            */
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /*            */
  0,   0,   0,   0,   0,  75,   0,   0,   0,   0, /*      .     */
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /*            */
 90,   0,   0,   0,   0,   0,   0,   0,   0,   0, /* !          */
  0,   0,   0,   0,   0,   0,   0,   0,   0, 109, /*          _ */
  0, 111,   0,   0,   0,   0,   0,   0,   0,   0, /*  ?         */
  0,   0,   0,   0,   0,   0,   0,   0,   0, 129, /*          a */
130, 131, 132, 133, 134, 135, 136, 137,   0,   0, /* bcdefghi   */
  0,   0,   0,   0,   0, 145, 146, 147, 148, 149, /*      jklmn */
150, 151, 152, 153,   0,   0,   0,   0,   0,   0, /* opqr       */
  0,   0, 162, 163, 164, 165, 166, 167, 168, 169, /*   stuvwxyz */
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /*            */
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /*            */
  0,   0,   0, 193, 194, 195, 196, 197, 198, 199, /*    ABCDEFG */
200, 201,   0,   0,   0,   0,   0,   0,   0, 209, /* HI       J */
210, 211, 212, 213, 214, 215, 216, 217,   0,   0, /* KLMNOPQR   */
  0,   0,   0,   0,   0,   0, 226, 227, 228, 229, /*       STUV */
230, 231, 232, 233,   0,   0,   0,   0,   0,   0, /* WXYZ       */
240, 241, 242, 243, 244, 245, 246, 247, 248, 249, /* 0123456789 */
  0,   0,   0,   0,   0,   0                      /*            */
#else
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
#endif
};

// some macros for commonly coded scanning operations...mostly to
// save some keystrokes and make things a little more readable
#define OPERATOR(op) (clause->newToken(TOKEN_OPERATOR, OPERATOR_##op, (RexxString *)GlobalNames::op, location))
#define CHECK_ASSIGNMENT(op) \
{\
   RexxToken *token = clause->newToken(TOKEN_OPERATOR, OPERATOR_##op, GlobalNames::op, location); \
   token->checkAssignment(this, GlobalNames::ASSIGNMENT_##op); \
   return token; \
}


/**
 * Save the current scanning location as a start position
 *
 * @param location A location object to store this in.
 */
void LanguageParser::startLocation(SourceLocation &location )
{
    // copy the start line location into the location object.
    location.setStart(lineNumber, lineOffset);
}

/**
 * Save the current scanning location.
 *
 * @param location A location object to store this in.
 */
void LanguageParser::endLocation(SourceLocation &location )
{
    // copy the end line location into the location object.
    location.setEnd(lineNumber, lineOffset);
}


/**
 * Save the current scanning location for a single character..
 *
 * @param location A location object to store this in.
 */
void LanguageParser::setLocation(SourceLocation &location )
{
    // copy the end line location into the location object.
    location.setLocation(lineNumber, lineOffset, lineNumber, lineOffset + 1);
}


/**
 * Find the next special character and validate
 * against a target character.
 *
 * @param target   The type of special we expect to find.
 * @param location A location object to record the object location against.
 *
 * @return true if we found the special, false otherwise.
 */
bool LanguageParser::nextSpecial(unsigned int target, SourceLocation &location )
{
    // find the start of the next token.
    unsigned int inch;

    // locate the next token.  Blanks are not significant here
    CharacterClass tokenClass = locateToken(inch, false);
    /* have something else on this line? */
    if (tokenClass != CLAUSE_EOF && tokenClass != CLAUSE_EOL)
    {
        // is the next character a match for our target?
        if (getChar() == target)
        {
            // step the position and record where we are as the end position
            stepPosition();
            endLocation(location);
            return true;
        }
    }
    return false;                        // didn't find the one we're looking for
}


/**
 * Scan over a Rexx comment, including checking for
 * comment nesting.
 */
void LanguageParser::scanComment()
{
    // comments can nest, so we need to keep track of our nesting level while
    // parsing.
    int level = 1;
    // step over the starting comment delimiter
    stepPosition(2);

    // we'll need to remember the starting lineNumber if we have an error.
    size_t startLine = lineNumber;

    // loop until our nesting level goes to zero.  We can also terminate
    // via an error if we hit the end of the file without finding the
    // closing delimiter.
    while (level > 0)
    {                  /* while still in a comment nest     */

        // hit the end of a line?, just step to the next line and keep going
        if (!moreChars())
        {
            nextLine();

            // but out of lines?  We've got a missing comment terminator
            if (!moreLines())
            {
                // record the ending position in our current clause
                clause->setEnd(lineNumber, lineOffset);
                // update the error information
                clauseLocation = clause->getLocation();
                syntaxError(Error_Unmatched_quote_comment, new_integer(startLine));
            }
            // keep scanning for the terminator
            continue;
        }

        // get the current char and step the position.
        unsigned int inch = nextChar();
        // at the end delimiter
        if (inch == '*' && getChar() == '/')
        {
            // this is our closer, step over the position and reduce the level.
            stepPosition();
            level--;
        }
        // start of a new nested character?
        else if (inch == '/' && getChar() == '*')
        {
            // we have a new nesting level to process
            stepPosition();
            level++;
        }
    }
}


/**
 * Locate the next "real" token in the parsing stream,
 * skipping extra blanks and comments.
 *
 * @param character If this is a "normal" character, this is the character value.
 * @param blanksSignificant
 *                  Indicates if blank characters should be considered as
 *                  significant in the current context.
 *
 * @return A type of the next token character.  If the type is
 *         NORMAL_CHAR, the character is also returned in the
 *         character reference argument.
 */
CharacterClass LanguageParser::locateToken(unsigned int &character, bool blanksSignificant)
{
    // default to having an invalid character
    character = INVALID_CHARACTER;

    // no more lines?  indicate we've hit the end of the file.
    if (!moreLines())
    {
        return CLAUSE_EOF;
    }
    // or, we could be at the end of the line...still a clause terminator,
    // but for a different reason.
    else if (!moreChars())
    {
        return CLAUSE_EOL;
    }

    // ok, we will scan as long as we have lines left.
    while (moreChars())
    {
        // next character from the line.
        unsigned char inch = getChar();
        // a recognized whitespace character?
        if (inch==' ' || inch=='\t')
        {
            // if blanks are significant, then return the indicator.
            if (blanksSignificant)
            {
                return SIGNIFICANT_BLANK;
            }
            else
            {
                // an ignorable blank...just step over it and try for another
                stepPosition();
            }
        }
        // possible continuation character?  We accept either ',' or '-' as a
        // continuation, but we also need to recognize that "--" is a line comment.
        else if (inch == ',' || inch == '-')
        {
            // line comment?  Just truncate the line and process as if
            // we ran out of characters and hit the end of the line.
            if (inch == '-' && followingChar() == '-')
            {
                truncateLine();
                return CLAUSE_EOL;
            }

            // assume for now that this is a real character...we need to
            // check for an end-of-line, ignoring any following blanks or comments.
            character = inch;

            // remember the current positions
            size_t startOffset = lineOffset;
            size_t startLine = lineNumber;
            // step over the current position to continue scanning.
            stepPosition();

            // skip blanks and tokens.  If we find anything else, we backup and treat
            // the character we found as a real character.
            while (true)
            {
                // hit the end of the clause while scanning for a continuation?
                if (!moreChars())
                {
                    // we've hit the end of the line without finding anything
                    // other than comments and white space.  If there are
                    // more lines in the file, we step to the next line.
                    // The continuation is functionally equivalent to a
                    // blank, so if blanks are significant, we can stop now,
                    // otherwise we need to continue scanning for a token character.
                    if (moreLines())
                    {
                        nextLine();
                        if (blanksSignificant)
                        {
                            return SIGNIFICANT_BLANK;
                        }
                    }
                    break;
                }

                // grab the next character and check for comments or white space
                unsigned int inch2 = getChar();

                // blanks are common, so check them first.
                if (inch2 == ' ' || inch2 == '\t')
                {
                    stepPosition();
                    continue;
                }
                // start of a traditional comment type?
                // we just step over the comment and continue.  Note, this might
                // actually step one or more lines...that's fine, the continuation still
                // holds.
                if (inch2 == '/' && followingChar() == '*')
                {
                    scanComment();
                    continue;
                }

                // line comment after a continuation char?  Truncate the line at this
                // position and loop around again to the end-of-line test so we
                // will handle the continuation.
                if (inch2 == '-' && followingChar() == '-')
                {
                    truncateLine();
                    continue;
                }

                // found something other than ignorable stuff, so the ',' or '-' is
                // a real character.  Return this now, after stepping back to the
                // saved position
                position(startLine, startOffset);
                character = inch;
                return NORMAL_CHAR;
            }
        }
        // could be a comment start.  Swallow that and continue.  Comments
        // are always ignored.  Note that his could leave us on a new line.
        else if (inch == '/' && followingChar() == '*')
        {
            scanComment();
        }
        else
        {
            // got a normal character, so we're done.
            character = inch;
            return NORMAL_CHAR;
            break;
        }
    }

    // hit the end of the line without finding anything.  Indicate end of clause
    return CLAUSE_EOL;
}


/**
 * Convert and check a hex constant, packing it down into a
 * string object.
 *
 * @param start  the start of the string data in the current
 *               source line.
 *
 * @param length The length of the raw string data.
 *
 * @return A Rexx string packed into 8-bit character form.
 */
RexxString* LanguageParser::packHexLiteral(size_t start, size_t length)
{
    // ""X is legal...it is just a null string
    // the rest is not nearly as easy :-)
    if (length == 0)
    {
        return GlobalNames::NULLSTRING;
    }

    // this is our counter for group packing
    int groupCount = 0;
    // our count of nibble characters we find...we can calculate the result length from this
    int nibbleCount = 0;
    // a pointer for scanning the data
    // scanning right-to-left makes it easier to identify correct whitespace positioning
    const char *inPointer = current + start + length - 1;

    // update the current clause location in case there are any errors
    clauseLocation = clause->getLocation();

    // first scan is to check REXX rules for validity of grouping

    // scan the entire input string from right to left
    for (size_t i = length; i > 0; i--)
    {
        // do we have a white space character?
        if (*inPointer == ' ' || *inPointer == '\t')
        {
            // now check to see if this is in a valid position.  We do not allow
            // blanks at the start or the end of the string, and blanks may
            // only appear at even hex digit (byte) boundaries.
            if (i == 1 || i == length) // no blank at string start or end
            {
                syntaxError(Error_Invalid_hex_hexblank, new_integer(i));

            }
            // not evenly divisible by two...bad blank placement
            else if ((groupCount & 1) != 0)
            {
                syntaxError(Error_Invalid_hex_invhex_group);
            }

            // we start a new group now
            groupCount = 0;
        }

        // non-blank character...for now, just count how many we have.
        else
        {
            // keep track of how large this group is and how many
            // total nibbles we have.
            groupCount++;
            nibbleCount++;
        }

        inPointer--;                        // step the input position
    }

    // second scan is to create the string value determined by the
    // hex constant.

    // reset the scan pointer
    inPointer = current + start;

    // this tells us how to process the first nibble
    size_t nibbleStart = (nibbleCount & 1);

    // get the final value size, rounding up if there are an odd
    // number of nibbles.  The first grouping of the string can have
    // an odd numbers, allowing things like 'a'x rather than '0a'x.
    size_t characterCount = nibbleCount / 2 + nibbleStart;

    // allocate a string we can pack the nibbles into
    RexxString *value = raw_string(characterCount);

    // this is for poking characters into the result
    size_t outPosition = 0;

    // scan the string again, packing the result number of
    // characters.
    for (size_t i = 0; i < characterCount; i++)
    {
        unsigned char byte = 0;

        // get the nibble character and pack it.
        unsigned char nibble = (unsigned char)*inPointer;
        // scan over white space, if we're there.
        while (nibble == ' ' || nibble == '\t')
        {
            nibble = (unsigned char)*(++inPointer);
        }

        // if we had an odd number of nibbles, the first time
        // we execute this loop, we'll only process 1 nibble.  After
        // the first time, we'll always do two at a time.  Since
        // we validated that the groups have an even number of non-blank
        // characters after the first, this will handle things fine.
        // Note also, since we do not allow leading blanks, we'll be
        // positioned correctly at the start.
        for (size_t k = nibbleStart; k < 2; k++)
        {
            // this should be a real character now
            nibble = (unsigned char)*inPointer++;
            // validate the digit and convert to a base binary value.
            // regular digit
            if (nibble >= '0' && nibble <= '9')
            {
                nibble -= '0';
            }
            // lowercase hex
            else if (nibble >= 'a' && nibble <= 'f')
            {
                nibble -= 'a';
                nibble += 10;
            }
            // uppercase hex
            else if (nibble >= 'A' && nibble <= 'F')
            {
                nibble -= 'A';
                nibble += 10;
            }
            else
            {
                // invalid character
                clauseLocation = clause->getLocation();
                char errorOutput = (char)nibble;
                syntaxError(Error_Invalid_hex_invhex, new_string(&errorOutput, 1));
            }
            // shift over the last nibble and add in the new one
            byte <<= 4;
            byte += nibble;
        }
        // we always process two nibbles at a time from here.
        nibbleStart = 0;
        // store in the output string
        value->putChar(outPosition++, byte);
    }

// return this...the caller will handle making this a common string.
    return value;
}


/**
 * Convert and check a binary constant, packing it down into a
 * string object.
 *
 * @param start  the start of the string data in the current
 *               source line.
 *
 * @param length The length of the raw string data.
 *
 * @return A Rexx string packed into 8-bit character form.
 */
RexxString* LanguageParser::packBinaryLiteral(size_t start, size_t length)
{
    // ""B is legal...it is just a null string
    // the rest is not nearly as easy :-)
    if (length == 0)
    {
        return GlobalNames::NULLSTRING;
    }

    // this is our counter for group packing
    int groupCount = 0;
    // our count of bit characters we find...we can calculate the result length from this
    int bitCount = 0;
    // a pointer for scanning the data
    // scanning right-to-left makes it easier to identify correct whitespace positioning
    const char *inPointer = current + start + length - 1;

    // update the current clause location in case there are any errors
    clauseLocation = clause->getLocation();

    // first scan is to check REXX rules for validity of grouping

    // scan the entire input string from right to left
    for (size_t i = length; i > 0; i--)
    {
        // do we have a white space character?
        if (*inPointer == ' ' || *inPointer == '\t')
        {
            // now check to see if this is in a valid position.  We do not allow
            // blanks at the start or the end of the string, and blanks may
            // only appear at 4-bit boundaries.
            if (i == 1 || i == length) // no blank at string start or end
            {
                syntaxError(Error_Invalid_hex_binblank, new_integer(i));

            }
            // not evenly divisible by four...bad blank placement
            else if ((groupCount & 3) != 0)
            {
                syntaxError(Error_Invalid_hex_invbin_group);
            }

            // we start a new group now
            groupCount = 0;
        }

        // non-blank character...for now, just count how many we have.
        else
        {
            // keep track of how large this group is and how many
            // total nibbles we have.
            groupCount++;
            bitCount++;
        }

        inPointer--;                        // step the input position
    }

    // second scan is to create the string value determined by the
    // hex constant.

    // reset the scan pointer
    inPointer = current + start;

    // this tells us how to process the first byte
    size_t byteSize = (bitCount & 7);

    // get the final value size, rounding up if there are extra bits
    // for the first byte.  The first grouping of the string can have
    // an odd number of bits.

    size_t characterCount = bitCount / 8 + (byteSize != 0);

    // if we have an even multiple of 8 bits, adjust the
    // first byte size.  After the first group, we always process
    // in groups of 8.
    if (byteSize == 0)
    {
        byteSize = 8;
    }

    // allocate a string we can pack the nibbles into
    RexxString *value = raw_string(characterCount);

    // this is for poking characters into the result
    size_t outPosition = 0;

    // scan the string again, packing the result number of
    // characters.
    for (size_t i = 0; i < characterCount; i++)
    {
        unsigned char byte = 0;

        // if we had an odd number of bits, the first time
        // we execute this loop, we'll process fewer than 8 bits.  After
        // the first time, we'll always do eight at a time.  Since
        // we validated that the groups have an even number of non-blank
        // characters after the first, this will handle things fine.
        // Note also, since we do not allow leading blanks, we'll be
        // positioned correctly at the start.
        for (size_t k = 0; k < byteSize; k++)
        {
            // get the bit character and pack it.
            unsigned char bit = (unsigned char)*inPointer++;
            // We can have white space between nibbles, so we need to do this here.
            while (bit == ' ' || bit == '\t')
            {
                bit = (unsigned char)*inPointer++;
            }
            // shift our accumulator
            byte <<= 1;
            // if this is a one bit, add this in
            if (bit == '1')
            {
                byte++;
            }
            // other option is a 0, else give an error
            else if (bit != '0')
            {
                // invalid character
                clauseLocation = clause->getLocation();
                char errorOutput = (char)bit;
                syntaxError(Error_Invalid_hex_invbin, new_string(&errorOutput, 1));
            }
        }
        // we always process 8 bits after the first byte
        byteSize = 8;
        // store in the output string
        value->putChar(outPosition++, byte);
    }

    // return this...the caller will handle making this a common string.
    return value;
}


/**
 * Extract a token from the source and create a new token object.
 * The token type and sub-type are set in the token, and any string
 * value extracted.
 *
 * @param previous Any previous token, necessary for determining if a
 *                 blank is significant.  This can be NULL.
 *
 * @return A new Token object representing the class of token.
 */
RexxToken *LanguageParser::sourceNextToken(RexxToken *previous )
{
    // ok, loop util we have a token to return
    for (;;)
    {
        // we need to know if blanks are significant in this context.
        bool blanksSignificant = previous == OREF_NULL ? false : previous->isBlankSignificant();
        unsigned int inch ;

        // locate the next token position
        CharacterClass tokenClass = locateToken(inch, blanksSignificant);

        SourceLocation location;
        // record a starting location.
        startLocation(location);
        // record this as just a single character by default
        location.adjustEnd(1);

        // hit the end of the file while scanning for the next token?  Return nothing
        if (tokenClass == CLAUSE_EOF)
        {
            return clause->newToken(TOKEN_EOC, CLAUSEEND_EOF, location);
        }

        // we hit the end of line on the clause
        else if (tokenClass == CLAUSE_EOL)
        {
            // mark the end offset as the current length of the line.
            location.setEndOffset(currentLength);
            // step to the next line and return a terminator token.
            nextLine();
            return clause->newToken(TOKEN_EOC, CLAUSEEND_EOL, location);
        }

        // we have what should be a significant blank character
        else if (tokenClass == SIGNIFICANT_BLANK )
        {
            // ok, this is possibly a significant blank, so scan ahead to the
            // next real token, ignoring the possibility of additional blanks.
            tokenClass = locateToken(inch, false);

            // in order for this blank to be truly significant, the next token
            // needs to be the start of a symbol, the start of a quoted literal,
            // or a paren or square bracket.
            if ((tokenClass == NORMAL_CHAR && isSymbolCharacter(inch)) ||
                 inch == '\"' || inch == '\'' ||
                 inch == '('  || inch == '[')
            {
                // this is a blank operator token
                return clause->newToken(TOKEN_BLANK, OPERATOR_BLANK, GlobalNames::BLANK, location);
            }

            // this is actually a non-significant blank, try again.
            continue;
        }
        else
        {
            // we've handled the special categories, so now process based on the
            // what we find in the first character.

            // translate this character to see if we have a good symbol character.
            unsigned int tran = translateChar(inch);
            if (tran != 0)
            {
                // scan off a symbol and return it.
                return scanSymbol();
            }
            // go scan a literal string
            else if (inch=='\'' || inch=='\"')
            {
                return scanLiteral();
            }
            // we have some other special character
            else
            {
                // step past the operator character position...we might need
                // to look at additional characters
                stepPosition();

                switch (inch)
                {
                    // these are likely various option or punctuation characters.

                    // closing paren (')')
                    case ')':
                    {
                        return clause->newToken(TOKEN_RIGHT, location);
                        break;
                    }

                    // closing square bracket (']')
                    case ']':
                    {
                        return clause->newToken(TOKEN_SQRIGHT, location);
                        break;
                    }

                    // open paren ('(')
                    case '(':
                    {
                        return clause->newToken(TOKEN_LEFT, location);
                        break;
                    }

                    // open left bracket ('[')
                    case '[':
                    {

                        return clause->newToken(TOKEN_SQLEFT, location);
                        break;
                    }

                    // comma...probably an argument list delimiter, since
                    // continuation commas are handled at scan time.
                    case ',':
                    {
                        return clause->newToken(TOKEN_COMMA, location);
                        break;
                    }

                    // semicolon...always a clause terminator
                    case ';':
                    {
                        return clause->newToken(TOKEN_EOC, CLAUSEEND_SEMICOLON, location);
                        break;
                    }

                    // colon...lots uses depending on context.
                    case ':':
                    {
                        // next one a colon also?
                        if (nextSpecial(':', location))
                        {
                            // double colon is a special character class.
                            return clause->newToken(TOKEN_DCOLON, location);
                        }
                        else
                        {
                            // single is nothing special
                            return clause->newToken(TOKEN_COLON, location);
                        }
                        break;
                    }

                    // the twiddle...we also handle the double case as a special
                    case '~':
                    {
                        if (nextSpecial('~', location))
                        {
                            // this is a special character class
                            return clause->newToken(TOKEN_DTILDE, location);
                        }
                        else
                        {
                            return clause->newToken(TOKEN_TILDE, location);
                        }
                        break;
                    }

                    // addition operator
                    case '+':
                    {
                        CHECK_ASSIGNMENT(PLUS);                       // this is allowed as an assignment shortcut
                        break;
                    }

                    // subtraction operator
                    case '-':
                    {
                        CHECK_ASSIGNMENT(SUBTRACT);                  // this is allowed as an assignment shortcut
                        break;
                    }

                    // integer division operator
                    case '%':
                    {
                        CHECK_ASSIGNMENT(INTDIV);                   // this is allowed as an assignment shortcut
                        break;
                    }

                    // division, or first char of remainder operator
                    case '/':
                    {
                        // two slashes is a special operator
                        if (nextSpecial('/', location))
                        {
                            // remainder operator
                            CHECK_ASSIGNMENT(REMAINDER);
                        }
                        // normal division
                        else
                        {
                            CHECK_ASSIGNMENT(DIVIDE);  // this is allowed as an assignment shortcut
                        }
                        break;
                    }

                    // multiply, or potentially a power operator
                    case '*':
                    {
                        // two start is power
                        if (nextSpecial('*', location))
                        {
                            // also an assignment operator, but not sure why anybody would use it!
                            CHECK_ASSIGNMENT(POWER);
                        }
                        // standard multiply
                        else
                        {
                            CHECK_ASSIGNMENT(MULTIPLY)          // this is allowed as an assignment shortcut
                        }
                        break;
                    }

                    // ampersand...an AND or XOR operator
                    case '&':
                    {
                        // double ampersand?
                        if (nextSpecial('&', location))
                        {

                            CHECK_ASSIGNMENT(XOR);  // this is allowed as an assignment shortcut
                        }
                        // simple AND operation
                        else
                        {
                            CHECK_ASSIGNMENT(AND);         // this is allowed as an assignment shortcut
                        }
                        break;
                    }

                    // vertical bar...could be the logical OR or concatenate.
                    case '|':
                    {
                        // doubled is concatenate, which can also be used as an assignment shortcut.
                        if (nextSpecial('|', location))
                        {
                            CHECK_ASSIGNMENT(CONCATENATE);
                        }
                        // logical OR, which can be an assignment shortcut
                        else
                        {
                            CHECK_ASSIGNMENT(OR);         // this is allowed as an assignment shortcut
                        }
                        break;
                    }

                    // equal sign...doubled can have special meaning too.
                    case '=':
                    {
                        // double is a strict equal operator
                        if (nextSpecial('=', location))
                        {
                            return OPERATOR(STRICT_EQUAL);
                        }
                        // simple equal
                        else
                        {
                            return OPERATOR(EQUAL);
                        }
                        break;
                    }

                    // less than sign...could be quite a few operators depending
                    // on what it is followed by
                    case '<':
                    {
                         // << or <<=
                         if (nextSpecial('<', location))
                         {
                             // <<=
                             if (nextSpecial('=', location))
                             {
                                 return OPERATOR(STRICT_LESSTHAN_EQUAL);
                             }
                             // <<
                             else
                             {
                                 return OPERATOR(STRICT_LESSTHAN);
                             }
                         }
                         // <=
                         else if (nextSpecial('=', location))
                         {
                             return OPERATOR(LESSTHAN_EQUAL);
                         }
                         // <>
                         else if (nextSpecial('>', location))
                         {
                             return OPERATOR(LESSTHAN_GREATERTHAN);
                         }
                         // just plain old <
                         else
                         {
                             return OPERATOR(LESSTHAN);
                         }
                         break;
                    }

                    // greater than sign.  Like less than, a lot of
                    // operators start with this.
                    case '>':
                    {
                        // >> or >>=
                        if (nextSpecial('>', location))
                        {
                            // >>=
                            if (nextSpecial('=', location))
                            {
                                return OPERATOR(STRICT_GREATERTHAN_EQUAL);
                            }
                            // >>
                            else
                            {
                                return OPERATOR(STRICT_GREATERTHAN);
                            }
                        }
                        // >=
                        else if (nextSpecial('=', location))
                        {
                            return OPERATOR(GREATERTHAN_EQUAL);
                        }
                        // ><
                        else if (nextSpecial('<', location))
                        {
                            return OPERATOR(GREATERTHAN_LESSTHAN);
                        }
                        // Just >
                        else
                        {
                            return OPERATOR(GREATERTHAN);
                        }
                        break;
                    }

                    // backslash...logical not or a negated comparison
                    case '\\':
                    {
                        // \=
                        if (nextSpecial('=', location))
                        {
                            // \==
                            if (nextSpecial('=', location))
                            {
                                return OPERATOR(STRICT_BACKSLASH_EQUAL);
                            }
                            // \=
                            else
                            {
                                return OPERATOR(BACKSLASH_EQUAL);
                            }
                        }
                        // \> variations
                        else if (nextSpecial('>', location))
                        {
                            // \>>
                            if (nextSpecial('>', location))
                            {
                                return OPERATOR(STRICT_BACKSLASH_GREATERTHAN);
                            }
                            // \>
                            else
                            {
                                return OPERATOR(BACKSLASH_GREATERTHAN);
                            }
                        }
                        // \< variations s than sign?        */
                        else if (nextSpecial('<', location))
                        {
                            // \<<
                            if (nextSpecial('<', location))
                            {
                                return OPERATOR(STRICT_BACKSLASH_LESSTHAN);
                            }
                            // \<
                            else
                            {
                                return OPERATOR(BACKSLASH_LESSTHAN);
                            }
                        }
                        // simple \ logical not
                        else
                        {
                            return OPERATOR(BACKSLASH);
                        }
                        break;
                    }

                    // we accept either of these as alternatives.  This is the similar
                    // situation as \...lots of variants
                    case (unsigned char)0xAA:      /* logical not  (need unsigned cast) */
                    case (unsigned char)0xAC:      /* logical not  (need unsigned cast) */
                    {
                        // not equal variants                */
                        if (nextSpecial('=', location))
                        {
                            // not ==
                            if (nextSpecial('=', location))
                            {
                                return OPERATOR(STRICT_BACKSLASH_EQUAL);
                            }
                            // not =
                            else
                            {
                                return OPERATOR(BACKSLASH_EQUAL);
                            }
                        }
                        // not > variants
                        else if (nextSpecial('>', location))
                        {
                            // not >>
                            if (nextSpecial('>', location))
                            {
                                return OPERATOR(STRICT_BACKSLASH_GREATERTHAN);
                            }
                            // not >
                            else
                            {
                                return OPERATOR(BACKSLASH_GREATERTHAN);
                            }
                        }
                        // not < variants
                        else if (nextSpecial('<', location))
                        {
                            // not <<
                            if (nextSpecial('<', location))
                            {
                                return OPERATOR(STRICT_BACKSLASH_LESSTHAN);
                            }
                            // not <
                            else
                            {
                                return OPERATOR(BACKSLASH_LESSTHAN);
                            }
                        }
                        // just a logical NOT
                        else
                        {
                            return OPERATOR(BACKSLASH);
                        }
                        break;
                    }

                    // invalid character of some type
                    default:
                    {
                        char   badchar[4];                    // working buffers for the errors
                        char   hexbadchar[4];

                        // mark current position n in clause */
                        clause->setEnd(lineNumber, lineOffset);
                        // update the error information
                        clauseLocation = clause->getLocation();
                        snprintf(badchar, sizeof(badchar), "%c", inch);
                        snprintf(hexbadchar, sizeof(hexbadchar), "%2.2X", inch);
                        // report the error with the invalid character displayed normally and in hex.
                        syntaxError(Error_Invalid_character_char, new_string(badchar), new_string(hexbadchar));
                        break;
                    }
                }
            }
        }
        break;
    }
    // should never get here
    return OREF_NULL;
}

// different scanning states for scanning numeric symbols
typedef enum
{
    EXP_START,
    EXP_EXCLUDED,
    EXP_DIGIT,
    EXP_SPOINT,
    EXP_POINT,
    EXP_E,
    EXP_ESIGN,
    EXP_EDIGIT,
} SymbolScanState;


/**
 * Scan a symbol from the source and return a token
 * identifying the type of symbol (constant, dot, variable, etc.).
 *
 * @return A token object describing the symbol.
 */
RexxToken *LanguageParser::scanSymbol()
{

    // we're in a clean scan state now
    SymbolScanState state = EXP_START;
    size_t eoffset = 0;                   // position of exponential sign for backing up.
    size_t start = lineOffset;            // remember token start position
    int dotCount = 0;                     // no periods yet

    // set the start position for the token
    SourceLocation location;
    startLocation(location);

    unsigned int inch = getChar();       // ok, get the current character to start this off
    // ok, loop through the token until we've consumed it all.
    for (;;)
    {
        // keep a count of periods...we use this to determine stem/compound and numeric values
        if (inch == '.')
        {
            dotCount++;
        }

        // finite state machine to establish numeric constant (with possible
        // included sign in exponential form)

        switch (state)
        {
            // this is our beginning state...we know nothing about this symbol yet.

            case EXP_START:
            {
                // have a digit at the start?  Potential number, so
                // we're looking for digits here.
                if (inch >= '0' && inch <= '9')
                {
                    state = EXP_DIGIT;
                }
                // if this is a dot, then we've got a starting decimal
                // point.  This could be a number or an environment symbol
                else if (inch == '.')
                {
                    state = EXP_SPOINT;
                }
                // a non-numeric character.  A number is not possible.
                else
                {
                    state = EXP_EXCLUDED;
                }
                break;
            }

            // we're scanning digits, still potentially a number.
            case EXP_DIGIT:
            {
                // is this a period?  Since we're scanning digits, this
                // is must be the first period and is a decimal point.
                // switch to scanning the part after the decimal.
                if (inch=='.')
                {
                    state = EXP_POINT;
                }
                // So far, the form is "digitsE"...this can still be a number,
                // but know we're looking for an exponent.
                else if (inch=='E' || inch == 'e')
                {
                    state = EXP_E;
                }
                // other non-digit?  We're no longer scanning a number.
                else if (inch < '0' || inch > '9')
                {
                    state = EXP_EXCLUDED;
                }
                // if we encounter a digit, the state is unchanged
                break;
            }

            // we're scanning from a leading decimal point.  How we
            // go from here depends on the next character.
            case EXP_SPOINT:
            {
                // not a digit immediately after the period, we're
                // scanning a normal symbol from here.
                if (inch < '0' || inch > '9')
                {
                    state = EXP_EXCLUDED;  /* not a number                      */
                }
                // second character is a digit, so we're scanning the
                // part after the decimal.
                else
                {
                    state = EXP_POINT;
                }
                break;
            }

            // scanning after a decimal point.  From here, we could hit
            // the 'E' for exponential notation.
            case EXP_POINT:
            {
                // potential exponential, switch scan to the exponent part.
                if (inch == 'E' || inch == 'e')
                {
                    state = EXP_E;
                }
                // non-digit other than an 'E'?, no longer a valid numeric.
                else if (inch < '0' || inch > '9')
                {
                    state = EXP_EXCLUDED;
                }
                // if we find a digit, the state is unchanged.
                break;
            }

            // we have a valid number up to an 'E'...now we can have digits or
            // a sign for the exponent.  The digit will be handled here, but
            // the +/- is either a symbol terminator or part of the symbol.
            // we check that at the end-of-symbol processing
            case EXP_E:
            {
                // switching to process the exponent digits
                if (inch >= '0' && inch <= '9')
                {
                    state = EXP_EDIGIT;
                }
                // we handle the sign situation below.
                break;
            }

            // we're scanning a potential numeric value, and we've just
            // had the sign, so we're looking for digits after that.   If there
            // are no digits, then the sign actually terminated the symbol, so
            // we need to back up.
            case EXP_ESIGN:
            {
                // found a digit here?  switching into exponent scan mode.
                if (inch >= '0' && inch <= '9')
                {
                    state = EXP_EDIGIT;
                }
                else
                {
                    // non-digit cannot be a number.
                    state = EXP_EXCLUDED;
                }
                break;
            }

            // scanning for exponent digits.  No longer numeric if we find a non-digit.
            case EXP_EDIGIT:
            {
                if (inch < '0' || inch > '9')
                {
                    state = EXP_EXCLUDED;
                }
                break;                   /* go get the next character         */
            }

            // once EXP_EXCLUDED is reached the state doesn't change.  We're
            // just consuming symbol characters from here.
            case EXP_EXCLUDED:
            {
                break;                   /* go get the next character         */
            }
        }

        // handled all of the states, now handle the termination checks.
        stepPosition();

        // did we step past an exponential sign but found an invalid exponent?
        if (eoffset != 0 && state == EXP_EXCLUDED)
        {
            // we need to back up the scan pointer to the sign position and
            // stop...this is the end of the symbol.
            lineOffset = eoffset;
            break;                     /* and we're finished with this      */
        }

        // have we reached the end of the line?  Also done.
        if (!moreChars())
        {
            break;
        }

        // get the next character and validate as a symbol character.
        inch = getChar();
        // if this was a good symbol character, run around the loop again and
        // see how this impacts the state machine
        if (translateChar(inch) != 0)
        {
            continue;
        }

        // we have a non-symbol character abutting the symbol characters.  If
        // we just scanned the 'E' (or 'e') in a potential exponent number,
        // a '+' or '-' is potentially part of the symbol value.
        if (state == EXP_E && (inch == '+' || inch == '-'))
        {
            // the sign might be at the end of the line.  If there
            // are no characters after that, no point in switching states.
            if (!haveNextChar())
            {
                // this is not a number and we've found the end position
                state = EXP_EXCLUDED;
                break;
            }

            // this only works if there are only digits after this point.
            // we need to remember this position in case we have to back up.
            eoffset = lineOffset;
            // step past the sign and switch the scanning state to look for
            // the exponent digits after a sign.
            stepPosition();
            state = EXP_ESIGN;

            // everything is all set up so we can back up.  Now get the next
            // character and see how things go from here.
            inch = getChar();
            // if this was a good symbol character, run around the loop again and
            // see how this impacts the state machine
            if (translateChar(inch) != 0)
            {
                continue;
            }

            // this is not a number...mark it so and also back up to before
            // the sign position.
            state = EXP_EXCLUDED;
            // we need to back up the scan pointer to the sign position and
            // stop...this is the end of the symbol.
            lineOffset = eoffset;
            break;
        }
        else
        {
            // We've reached a non-symbol character.  State remains in whatever
            // the last state was.
            break;
        }
    }

    // ok, we've located the end position of the symbol token.  The final state
    // will tell us what sort of symbol we have.  Generally, EXP_EXCLUDED indicates
    // a non-numeric value, all other states will be valid numbers.

    // lineOffset is now one character past the end of the symbol.
    size_t length = lineOffset - start;
    // we're going to need to copy this into a string object.
    RexxString *value = raw_string(length);

    // we also can tag numeric types.
    TokenSubclass numeric = SUBTYPE_NONE;

    // ok, copy each character over, translating to uppercase.
    for (size_t i = 0; i < length; i++)
    {
        inch = getChar(start + i);
        unsigned int tran = translateChar(inch);
        // if this translates ok, then this is a normal symbol
        // character.
        if (tran != 0)
        {
            value->putChar(i, tran);
        }
        else
        {
            // this is an exponent sign.  Use the original
            value->putChar(i, inch);
        }
    }

    // mark the value as being all uppercase.
    value->setUpperOnly();
    // get the common string value so we don't keep around multiple copies
    // of variable names.
    value = commonString(value);
    // record the current position in the clause
    clause->setEnd(lineNumber, lineOffset);

    // but is this symbol too long?  Sigh, this is an error, but
    // we need the symbol name for the error message.
    if (length > (size_t)MAX_SYMBOL_LENGTH)
    {
        // update the error information
        clauseLocation = clause->getLocation();
        syntaxError(Error_Name_too_long_name, value);
    }

    // now see if we can figure out some subtypes.
    TokenSubclass subclass = SUBTYPE_NONE;

    // we determine a lot from the first character.
    inch = getChar(start);

    // a solo period?  This is a special symbol, at least in parse templates.
    if (length == 1 && inch == '.')
    {
        subclass = SYMBOL_DUMMY;
    }
    // leading digit?  this is a constant symbol, but we might know
    // even more based on the final scan state.
    else if (inch >= '0' && inch <= '9')
    {
        // this is a constant symbol
        subclass = SYMBOL_CONSTANT;
        // if all digits and not longer than REXXINTEGER_DIGITS, we can
        // use integer objects instead.
        if (state == EXP_DIGIT && length <= (size_t)Numerics::REXXINTEGER_DIGITS)
        {
            // no leading zero or only zero?
            if (inch != '0' || length == 1)
            {
                // we can make this an integer object
                numeric = INTEGER_CONSTANT;
            }
        }
    }
    // beginning with a period, this is a dot symbol (although
    // potentially a number.
    else if (inch == '.')
    {
        // Beginning with a period, this is either a dot symbol or a
        // number.  If the last scan state was EXP_EXCLUDED, this
        // is a dot symbol
        if (state == EXP_EXCLUDED)
        {
            subclass = SYMBOL_DOTSYMBOL;
        }
        else
        {
            // this is a constant symbol, but we can't do anything
            // additional with the numeric information.
            subclass = SYMBOL_CONSTANT;
        }
    }
    else
    {
        // a variable symbol, this has other subtypes.
        subclass = SYMBOL_VARIABLE;

        // if the symbol contains a period, this is either a stem or compound variable
        if (dotCount > 0)
        {
            // a stem variable has just one dot and that is on the end.
            if (dotCount == 1 && value->getChar(length - 1) == '.')
            {
                subclass = SYMBOL_STEM;
            }
            else
            {
                // this is a compound symbol
                subclass = SYMBOL_COMPOUND;
            }
        }
    }

    // now mark the token end location
    endLocation(location);

    // get a symbol token, including the numeric information.
    RexxToken *token = clause->newToken(TOKEN_SYMBOL, subclass, value, location);
    token->setNumeric(numeric);
    return token;
}


/**
 * Scan off a literal string (including hex or binary literals),
 * and return as a literal token.
 *
 * @return A token representing the literal.
 */
RexxToken *LanguageParser::scanLiteral()
{
    // set the start position for the token
    SourceLocation location;
    startLocation(location);

    // get the opening quote character and save for end matching
    unsigned int inch = getChar();
    unsigned int literalDelimiter = inch;

    size_t literalEnd = 0;       // will be the literal end position

    // save staring point, which is just past the quote.
    size_t start = lineOffset + 1;
    // keep track of doubled quotes so we know how large the final string will be
    int doubleQuotes = 0;

    // this is a simple literal until we can check after the end.
    TokenSubclass type = LITERAL_STRING;

    // ok, scan through the string looking for the closing delimiter.
    for (;;)
    {
        // first time, we're stepping over the opening quote, after that,
        // stepping over the previous character.
        stepPosition();

        // reached the end of the line without finding the closing
        // quote?  this is an error
        if (!moreChars())
        {
            // mark the end of the clause
            clause->setEnd(lineNumber, lineOffset);
            // update the error information
            clauseLocation = clause->getLocation();

            // we have different errors depending on the type of delimiter
            if (literalDelimiter == '\'')
            {
                syntaxError(Error_Unmatched_quote_single);
            }
            else
            {
                syntaxError(Error_Unmatched_quote_double);
            }
        }

        // get the next character and perform the delimiter checks.
        inch = getChar();

        if (literalDelimiter == inch)
        {
            // remember the (potential) data end position
            literalEnd = lineOffset - 1;
            // we need to look at what is after the delimiter.  There are
            // three possibilities:  1)  a doubled delimiter, 2) the symbol
            // 'X' for a hex literal, and 3) the symbol 'B' for a bit literal.
            // We only take care of 1) here, 2) and 3) will be handled after
            // we've determined this is a closing literal.
            stepPosition();

            // line ends with the literal...this is easy.
            if (!moreChars())
            {
                break;
            }

            // ok, now check for the doubled one.
            inch = getChar();
            if (inch != literalDelimiter)
            {
                break;             // really was the end
            }
            // still in the string, but final string will be shorter than
            // the scanned string.
            doubleQuotes++;
        }
    }

    // OK, we've found the end delimiter.  So far, so good.  Now we need to
    // take a peek after the literal to see we have a hex or bit literal.  We're
    // currently positioned at the first character after the literal end.

    // did we end the line with this literal?  If not, we need to look for
    // the hex or bin markers.
    if (moreChars())
    {
        // ok, get the next character.
        inch = getChar();

        // ok, followed by an X, but we need to make sure this is not part of
        // some longer symbol
        if (inch == 'x' || inch == 'X')
        {
            // if the following character is not a symbol character,
            // NOTE:  This also handles end-of-line situations.
            if (!isSymbolCharacter(getNextChar()))
            {
                stepPosition();
                type = LITERAL_HEX;      /* set the appropriate type          */
            }
        }
        // and perform the same check for a binary string
        else if (inch == 'b' || inch == 'B')
        {
            if (!isSymbolCharacter(getNextChar()))
            {
                stepPosition();
                type = LITERAL_BIN;      /* set the appropriate type          */
            }
        }
    }

    // literalEnd is pointing at the last character, start is pointing at the first, so
    // we need to add 1 to the length.
    size_t length = literalEnd - start + 1;

    // record the position
    clause->setEnd(lineNumber, lineOffset);

    RexxString *value;

    // does this literal require packing?
    if (type != LITERAL_STRING)
    {
        if (type == LITERAL_HEX)
        {
            value = packHexLiteral(start, length);
        }
        else
        {
            value = packBinaryLiteral(start, length);
        }
    }
    else
    {
        // no doubled quotes?  Just grab this directly without
        // scanning, which is faster
        if (doubleQuotes == 0)
        {
            value = new_string(current + start, length);
        }
        else
        {
            // get a string to hold the final length, minus the number
            // of doubled quotes.
            length -= doubleQuotes;
            value = raw_string(length);
            // copy over the value, accounting for the doubled quotes
            for (size_t i = 0, j = start; i < length; i++, j++)
            {
                // get the next character and check against the delimiter
                inch = getChar(j);
                if (inch == literalDelimiter)
                {
                    // just step one extra character for the doubles.
                    j++;
                }
                value->putChar(i, inch);
            }
        }
    }

    // force this to a common string
    value = commonString(value);

    // update the token location and create a new token for this
    endLocation(location);
    return clause->newToken(TOKEN_LITERAL, type, value, location);
}


/**
 * Scans a string for symbol validity and returns a
 * type indicator as to what type of symbol.
 *
 * @param string The string to scan.
 *
 * @return A type indicator indicating what sort of symbol we're
 *         looking at.
 */
StringSymbolType LanguageParser::scanSymbol(RexxString *string)
{
    size_t stringLength = string->getLength();

    // null string or too long is a bad variable.
    if (stringLength > MAX_SYMBOL_LENGTH || stringLength == 0)
    {
        return STRING_BAD_VARIABLE;
    }

    const char *linend = string->getStringData() + stringLength;

    // counter of periods in the name
    size_t compound = 0;
    // no exponent encountered
    bool haveExponent = false;

    const char *scan = string->getStringData();

    // first scan the entire string looking for an invalid character...that is a quick
    // out.  Note that we might encounter a "+" or "-" in a potential numeric value, so
    // we can just drop out
    while (scan < linend && isSymbolCharacter(*scan))
    {
        // count any periods we see along the way.
        if (*scan == '.')
        {
            compound++;
        }
        scan++;
    }

    // not used up the entire string before hitting a non-symbol character.
    // this could be because of a "+" or "-" in an exponent value, so we
    // can't bail immediately.
    if (scan < linend)
    {
        // we've found a non-symbol character.  This might be a '-' or '+', but
        // if we found this at the end of the string, then it can't be a number.
        // we can bail immediately
        if (scan + 1 >= linend)
        {
            return STRING_BAD_VARIABLE;
        }

        // something other than a sign is also bad
        if (*scan != '-' && *scan != '+')
        {
            return STRING_BAD_VARIABLE;
        }

        // we've found a "+" or "-".  The character before
        // this character must be a 'e' or 'E' and the rest of
        // the string must only be digits

        if (Utilities::toUpper(*(scan - 1)) != 'E')
        {
            return STRING_BAD_VARIABLE;
        }

        // verify the remainder
        scan++;
        while (scan < linend)
        {
            if (!Utilities::isDigit(*scan))
            {
                return STRING_BAD_VARIABLE;
            }
            scan++;
        }
        // we've seen an exponent value.
        haveExponent = true;
    }
    // we've scanned the entire string and it is "provisionally" valid.

    // now check for different types.  firs check for potential numbers
    if (string->getChar(0) == '.' || Utilities::isDigit(string->getChar(0)))
    {
        // only a period?
        if (compound == 1 && stringLength == 1)
        {
            return STRING_LITERAL_DOT;
        }
        // more than one period precludes this being a valid
        // number.  It also negates the validity of an exponent
        // if we've seen one.
        else if (compound > 1)
        {
            if (haveExponent)
            {
                return STRING_BAD_VARIABLE;
            }
            // just a literal token
            return STRING_LITERAL;
        }
        else
        {
            // this is potentially a number.  We scan the string up to the
            // exponent (if we have one), verifying that we have nothing but
            // periods and digits.

            scan = string->getStringData();

            while (scan < linend)
            {
                // stop scanning on first non-numeric
                if (!Utilities::isDigit(*scan) && *scan != '.')
                {
                    break;
                }
                scan++;
            }

            // used everything up, we're numeric
            if (scan >= linend)
            {
                return STRING_NUMERIC;
            }

            // found a potential exponent.  If the next character is
            // "+" or "-", we already validated everything past that point
            if (Utilities::toUpper(*scan) == 'E')
            {
                scan++;
                if (*scan == '-' || *scan == '+')
                {
                    return STRING_NUMERIC;
                }

                // after the 'E', we expect to find digits
                while (scan < linend)
                {
                    // stop scanning on first non-numeric
                    // this cannot be a number any longer
                    if (!Utilities::isDigit(*scan))
                    {
                        return STRING_LITERAL;
                    }
                    scan++;
                }
                // this is a number
                return STRING_NUMERIC;
            }
            // some other symbol character...this is a literal symbol
            return STRING_LITERAL;
        }
    }

    // no periods, does not begin with a digit...must be a simple variable
    else if (compound == 0)
    {
        return STRING_NAME;
    }
    // a stem has one period and it is the end character
    else if (compound == 1 && *(scan - 1) == '.')
    {
        return STRING_STEM;
    }
    // at least one period not at the beginning or end, this is a compound variable
    else
    {
        return STRING_COMPOUND_NAME;
    }
}
