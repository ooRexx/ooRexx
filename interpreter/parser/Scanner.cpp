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
/* Scanner portion of the REXX Source File Class                              */
/*                                                                            */
/******************************************************************************/
#include <ctype.h>
#include <string.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "SourceFile.hpp"

int RexxSource::precedence(
    RexxToken  *token)                 /* target token                      */
/******************************************************************************/
/* Fucntion:  Determine a token's operator precedence                         */
/******************************************************************************/
{
    switch (token->subclass)
    {           /* process based on subclass         */

        default:
            return 0;                        /* this is the bottom of the heap    */
            break;

        case OPERATOR_OR:
        case OPERATOR_XOR:
            return 1;                         /* various OR types are next         */
            break;

        case OPERATOR_AND:
            return 2;                         /* AND operator ahead of ORs         */
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
            return 3;                         /* concatenates are next             */
            break;

        case OPERATOR_ABUTTAL:
        case OPERATOR_CONCATENATE:
        case OPERATOR_BLANK:
            return 4;                         /* concatenates are next             */
            break;

        case OPERATOR_PLUS:
        case OPERATOR_SUBTRACT:
            return 5;                         /* plus and minus next               */
            break;

        case OPERATOR_MULTIPLY:
        case OPERATOR_DIVIDE:
        case OPERATOR_INTDIV:
        case OPERATOR_REMAINDER:
            return 6;                         /* mulitiply and divide afer simples */
            break;

        case OPERATOR_POWER:
            return 7;                         /* almost the top of the heap        */
            break;

        case OPERATOR_BACKSLASH:
            return 8;                         /* NOT is the top honcho             */
            break;
    }
}

/*********************************************************************
*  The following table detects alphanumeric characters and           *
*  special characters that can be part of an REXX symbol.            *
*  The table also convert lower case letters to upper case.          *
*********************************************************************/
int RexxSource::characterTable[]={
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

                                       /* some macros for commonly coded    */
                                       /* scanning operations...mostly to   */
                                       /* save some keystrokes and make     */
                                       /* things a little more readable     */
#define GETCHAR()  ((unsigned char)(this->current[this->line_offset]))
#define MORELINE() (this->line_offset < this->current_length)
#define OPERATOR(op) (this->clause->newToken(TOKEN_OPERATOR, OPERATOR_##op, (RexxString *)OREF_##op, location))
#define CHECK_ASSIGNMENT(op, token) (token->checkAssignment(this, (RexxString *)OREF_ASSIGNMENT_##op))

void RexxSource::endLocation(
  SourceLocation &location )           /* token location information        */
/****************************************************************************/
/* Function:  Record a tokens ending location                               */
/****************************************************************************/
{
    // copy the end line location
    location.setEnd(line_number, line_offset);
}

bool RexxSource::nextSpecial(
  unsigned int  target,                /* desired target character          */
  SourceLocation &location )           /* token location information        */
/****************************************************************************/
/* Function:  Find the next special character and verify against a target   */
/****************************************************************************/
{
    unsigned int inch = this->locateToken(OREF_NULL); /* find the next token               */
    /* have something else on this line? */
    if (inch != CLAUSEEND_EOF && inch != CLAUSEEND_EOL)
    {
        if (GETCHAR() == target)
        {         /* is the next character a match?    */
            this->line_offset++;             /* step over the next                */
            this->endLocation(location);     /* update the end location part      */
            return true;                     /* got what we need!                 */
        }
    }
    return false;                        // didn't find the one we're looking for
}

void RexxSource::comment()
/****************************************************************************/
/* Function:  Scan source to skip over a nest of comments                   */
/****************************************************************************/
{
    int level = 1;                           /* start the comment nesting         */
    this->line_offset += 2;              /* step over the comment start       */
    size_t startline = this->line_number;       /* remember the starting position    */
    while (level > 0)
    {                  /* while still in a comment nest     */
                       /* hit the end of a line?            */
        if (this->line_offset >= this->current_length)
        {
            this->nextLine();                /* need to go to the next line       */
                                             /* no more lines?                    */
            if (this->line_number > this->line_count)
            {
                /* record current position in clause */
                this->clause->setEnd(this->line_count, this->line_offset);
                // update the error information
                clauseLocation = clause->getLocation();
                /* error, must report                */
                syntaxError(Error_Unmatched_quote_comment, new_integer(startline));
            }
            continue;                        /* go loop around                    */
        }
        unsigned int inch = GETCHAR();                  /* get the next character            */
        this->line_offset++;               /* step past the character           */
                                           /* is this the end delimeter?        */
        if (inch == '*' && GETCHAR() == '/')
        {
            level--;                         /* reduce the nesting level          */
            this->line_offset++;             /* step the pointer over the close   */
        }
        /* start of a new comment?           */
        else if (inch == '/' && GETCHAR() == '*')
        {
            level++;                         /* increment the level               */
            this->line_offset++;             /* step the pointer over new start   */
        }
    }
}

unsigned int RexxSource::locateToken(
  RexxToken *previous )                /* previous token                    */
/****************************************************************************/
/* Function:  Locate next significant token in source, skipping extra       */
/*            blanks and comments.                                          */
/****************************************************************************/
{
    size_t          startline;            /* backward reset line number        */
    size_t          startoffset;          /* backward reset offset             */

    bool            blanks = false;       /* are blanks significant?           */

    unsigned int character = 0;           /* no specific character type yet    */
                                          /* check if blanks should be returned*/
    if (previous != OREF_NULL &&          /* no previous token, or             */
        /* have a symbol, literal, right     */
        /* paren or right square bracket     */
        (previous->classId == TOKEN_SYMBOL ||
         previous->classId == TOKEN_LITERAL ||
         previous->classId == TOKEN_RIGHT ||
         previous->classId == TOKEN_SQRIGHT))
    {
        blanks = true;                      /* blanks are significant here       */
    }

                                            /* no more lines in file?            */
    if (this->line_number > this->line_count)
    {
        character = CLAUSEEND_EOF;          /* return an end-of-file             */
    }
    else if (!MORELINE())                 /* reached the line end?             */
    {
        character = CLAUSEEND_EOL;          /* return an end-of-line             */
    }
    else
    {
        /* while more program to scan        */
        while (this->line_offset < this->current_length)
        {
            unsigned int inch = GETCHAR();                 /* get the next character            */
            if (inch==' ' || inch=='\t')
            {    /* blank or tab?                     */
                if (blanks)
                {                   /* is this significant?              */
                    character = TOKEN_BLANK;      /* have a blank character            */
                    break;                        /* got what we need                  */
                }
                else
                {
                    this->line_offset++;          /* step the position                 */
                    continue;                     /* go around again                   */
                }
            }
            /* possible continuation character?  */
            else if (inch == ',' || inch == '-')
            {
                /* line comment?                     */
                if (inch == '-' && this->line_offset + 1 < this->current_length &&
                    this->current[this->line_offset + 1] == '-')
                {
                    this->line_offset = this->current_length;
                    break;
                }

                character = inch;               /* assume for now real character     */
                /* we check for EOL (possibly following blanks and comments)         */
                startoffset = this->line_offset;/* remember the location             */
                startline = this->line_number;  /* remember the line position        */
                this->line_offset++;            /* step the position                 */

                                                /* skip blanks and comments          */
                while (this->line_offset < this->current_length)
                {
                    unsigned int inch2 = GETCHAR();            /* pick up the next character        */
                                                  /* comment level start?              */
                    if (inch2 == '/' && (this->line_offset + 1 < this->current_length) &&
                        this->current[this->line_offset + 1] == '*')
                    {
                        this->comment();            /* go skip over the comment          */
                        continue;                   /* and continue scanning             */
                    }
                    /* line comment?                     */
                    if (inch2 == '-' && (this->line_offset + 1 < this->current_length) &&
                        this->current[this->line_offset + 1] == '-')
                    {
                        /* go skip overto the end of line    */
                        this->line_offset = this->current_length;
                        break;
                    }
                    /* non-blank outside comment         */
                    if (inch2 != ' ' && inch2 != '\t')
                    {
                        break;                      /* done scanning                     */
                    }
                    this->line_offset++;          /* step over this character          */
                }
                /* found an EOL?                     */
                if (this->line_offset >= this->current_length)
                {
                    /* more lines in file?               */
                    if (this->line_number < this->line_count)
                    {
                        this->nextLine();           /* step to the next line             */
                        if (blanks)
                        {               /* blanks allowed?                   */
                            character = TOKEN_BLANK;  /* make this a blank token           */
                            break;                    /* finished here                     */
                        }
                    }
                }
                else
                {                          /* reset to the starting position    */
                    this->position(startline, startoffset);
                    character = inch;             /* this is a real character          */
                    break;                        /* other non-blank, done scanning    */
                }
            }
            /* comment level start?              */
            else if (inch == '/' && (this->line_offset + 1 < this->current_length) &&
                     this->current[this->line_offset + 1] == '*')
            {
                this->comment();                /* go skip over the comment          */
            }
            else
            {                            /* got the character                 */
                character = inch;               /* this is a good character          */
                break;                          /* done looping                      */
            }
        }
        if (!MORELINE())                    /* fallen off the end of the line?   */
        {
            character = CLAUSEEND_EOL;        /* this is an end of clause          */
        }
    }
    return character;                     /* return the character              */
}

RexxString *RexxSource::packLiteral(
  size_t     start,                    /* start of the literal in line      */
  size_t     length,                   /* length of the literal to reduce   */
  int        type )                    /* type of literal to process        */
/****************************************************************************/
/* Function:  Convert and check a hex or binary constant, packing it down   */
/*            into a string object.                                         */
/****************************************************************************/
{
    int    _first;                       /* switch to mark first group        */
    int    blanks;                       /* switch to say if scanning blanks  */
    int    count;                        /* count for group                   */
    size_t i;                            /* loop counter                      */
    size_t j;                            /* loop counter                      */
    size_t k;                            /* loop counter                      */
    size_t m;                            /* temporary integer                 */
    int    byte;                         /* individual byte of literal        */
    int    nibble;                       /* individual nibble of literal      */
    size_t oddhex;                       /* odd number of characters in first */
    size_t inpointer;                    /* current input position            */
    int    outpointer;                   /* current output pointer            */
    RexxString *value;                   /* reduced value                     */
    size_t real_length;                  /* real number of digits in string   */
    char   error_output[2];              /* used for formatting error         */

    _first = true;                        /* initialize group flags and        */
    count = 0;                            /* counters                          */
    blanks = false;
    error_output[1] = '\0';               /* terminate string                  */
                                          /* set initial input/output positions*/
    inpointer = start;                    /* get initial starting position     */

    if (!length)                          /* hex or binary null string?        */
    {
        value = OREF_NULLSTRING;            /* this is a null string             */
    }
    else
    {                                /* data to reduce                    */
        /* first scan is to check REXX rules for validity of grouping             */
        /* and to remove blanks                                                   */

        real_length = length;                /* pick up the string length         */
        for (i = 0; i < length; i++)
        {       /* loop through entire string        */
                /* got a blank?                      */
            if (this->current[inpointer] == ' ' || this->current[inpointer] == '\t')
            {
                blanks = true;                    /* remember scanning blanks          */
                /* don't like initial blanks or groups after the first                  */
                /* which are not in twos (hex) or fours (binary)                        */
                if (i == 0 ||                     /* if at the beginning               */
                    (!_first &&                   /* or past first group and not the   */
                     /* correct size                      */
                     (((count&1) && type == LITERAL_HEX) ||
                      ((count&3) && type == LITERAL_BIN))))
                {
                    m = i+1;                        /* place holder for new_integer invocation */
                    // update the error information
                    clauseLocation = clause->getLocation();
                    if (type == LITERAL_HEX)        /* hex string?                       */
                    {
                        /* report correct error              */
                        syntaxError(Error_Invalid_hex_hexblank, new_integer(m));
                    }
                    else                            /* need the binary message           */
                    {
                        syntaxError(Error_Invalid_hex_binblank, new_integer(m));
                    }
                }
                count = 0;                        /* this starts a new group           */
                real_length--;                    /* this shortens the value           */

            }
            else
            {
                if (blanks)                       /* had a blank group?                */
                {
                    _first = false;                 /* no longer on the lead grouping    */
                }
                blanks = false;                   /* not processing blanks now         */
                count++;                          /* count this significant character  */
            }
            inpointer++;                        /* step the input position           */
        }

        if (blanks ||                        /* trailing blanks or                */
            (!_first &&                      /* last group isn't correct count?   */
             (((count&1) && type == LITERAL_HEX) ||
              ((count&3) && type == LITERAL_BIN))))
        {
            m = i-1;                           /* place holder for new_integer invocation */
            // update the error information
            clauseLocation = clause->getLocation();
            if (type == LITERAL_HEX)           /* hex string?                       */
            {
                /* report correct error              */
                syntaxError(Error_Invalid_hex_hexblank, new_integer(m));
            }
            else                               /* need the binary message           */
            {
                syntaxError(Error_Invalid_hex_binblank, new_integer(m));
            }
        }

        /* second scan is to create the string value determined by the            */
        /* hex or binary constant.                                                */

        i = real_length;                     /* get the adjusted length           */
                                             /* reset the scan pointers           */
        inpointer = start;                   /* reset the scan pointer            */
        outpointer = 0;                      /* set the position a start          */
        if (type == LITERAL_HEX)
        {           /* hex literal?                      */
            oddhex = i&1;                      /* get any odd count                 */
            i >>= 1;                           /* divide by 2 ... and               */
            i += oddhex;                       /* add in the odd count              */
            value = raw_string(i);             /* get the final value               */

            for (j = 0; j < i; j++)
            {          /* loop for the appropriate count    */
                byte = 0;                        /* current byte is zero              */
                for (k = oddhex; k < 2; k++)
                {   /* loop either 1 or 2 times          */
                    /* get the next nibble               */
                    nibble = this->current[inpointer];
                    inpointer++;                   /* step to the next character        */
                    while (nibble == ' ' || nibble == '\t')
                    {   /* step over any inter-nibble blanks */
                        /* get the next nibble               */
                        nibble = this->current[inpointer];
                        inpointer++;                 /* step to the next character        */
                    }
                    /* real digit?                       */
                    if (nibble >= '0' && nibble <= '9')
                        nibble -= '0';               /* make base zero                    */
                                                     /* lowercase hex digit?              */
                    else if (nibble >= 'a' && nibble <= 'f')
                    {
                        nibble -= 'a';               /* subtract lowest and               */
                        nibble += 10;                /* add 10 to digit                   */
                    }                              /* uppercase hex digit?              */
                    else if (nibble >= 'A' && nibble <= 'F')
                    {
                        nibble -= 'A';               /* subtract lowest and               */
                        nibble += 10;                /* add 10 to digit                   */
                    }
                    else
                    {
                        // update the error information
                        clauseLocation = clause->getLocation();
                        error_output[0] = nibble;    /* copy the error character          */
                                                     /* report the invalid character      */
                        syntaxError(Error_Invalid_hex_invhex, new_string(&error_output[0]));
                    }
                    byte <<= 4;                    /* shift the last nibble over        */
                    byte += nibble;                /* add in the next nibble            */
                }
                oddhex = 0;                      /* remainder are full bytes          */
                value->putChar(outpointer, byte);/* store this in the output position */
                outpointer++;                    /* step to the next position         */
            }
            value = this->commonString(value); /* now force to a common string      */
        }
        else
        {                               /* convert to binary                 */
            oddhex = i&7;                      /* get the leading byte count        */
            if (oddhex)                        /* incomplete byte?                  */
            {
                oddhex = 8 - oddhex;             /* get the padding count             */
            }
            i += oddhex;                       /* and add that into total           */
            i >>= 3;                           /* get the byte count                */
            value = raw_string(i);             /* get the final value               */

            for (j = 0; j < i; j++)
            {          /* loop through the entire string    */
                byte = 0;                        /* zero the byte                     */
                for (k = oddhex; k < 8; k++)
                {   /* loop through each byte segment    */
                    /* get the next bit                  */
                    nibble = this->current[inpointer];
                    inpointer++;                   /* step to the next character        */
                    while (nibble == ' ' || nibble == '\t')
                    {  /* step over any inter-nibble blanks */
                        /* get the next nibble               */
                        nibble = this->current[inpointer];
                        inpointer++;                 /* step to the next character        */
                    }
                    byte <<= 1;                    /* shift the accumulator             */
                    if (nibble == '1')             /* got a one bit?                    */
                    {
                        byte++;                      /* add in the bit                    */
                    }
                    else if (nibble != '0')
                    {      /* not a '0' either?                 */
                        // update the error information
                        clauseLocation = clause->getLocation();
                        error_output[0] = nibble;    /* copy the error character          */
                                                     /* report the invalid character      */
                        syntaxError(Error_Invalid_hex_invbin, new_string(&error_output[0]));
                    }
                }
                oddhex = 0;                      /* use 8 bits for the remaining group*/
                value->putChar(outpointer, byte);/* store this in the output position */
                outpointer++;                    /* step to the next position         */
            }
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
    RexxToken  *token = OREF_NULL;        /* working token                     */
    RexxString *value;                    /* associate string value            */
    unsigned int inch;                    /* working input character           */
    size_t eoffset;                       /* location of exponential           */
    int    state;                         /* state of symbol scanning          */
    size_t start;                         /* scan start location               */
    size_t litend;                        /* end of literal data               */
    size_t length;                        /* length of extracted token         */
    int    dot_count;                     /* count of periods in symbol        */
    unsigned int literal_delimiter;       /* literal string delimiter          */
    int    type;                          /* type of literal token             */
    size_t i;                             /* loop counter                      */
    size_t j;                             /* loop counter                      */
    int    subclass;                      /* sub type of the token             */
    int    numeric;                       /* numeric type flag                 */
    SourceLocation location;              /* token location information        */
    char   tran;                          /* translated character              */
    char   badchar[4];                    /* working buffer for errors         */
    char   hexbadchar[4];                 /* working buffer for errors         */

    /* definitions of states of exponential numeric scan */
#define EXP_START    0
#define EXP_EXCLUDED 1
#define EXP_DIGIT    2
#define EXP_SPOINT   3
#define EXP_POINT    4
#define EXP_E        5
#define EXP_ESIGN    6
#define EXP_EDIGIT   7

    for (;;)
    {                           /* loop until we find a significant  */
                                /* token                             */
        inch = this->locateToken(previous);/* locate the next token position    */

        // record a starting location.
        location.setLocation(line_number, line_offset, line_number, line_offset + 1);

        if (inch == CLAUSEEND_EOF)
        {       /* reach the end of the source?      */
            token = OREF_NULL;               /* no token to return                */
            break;                           /* finished                          */
        }
        else if (inch == CLAUSEEND_EOL)
        {  /* some other end-of-clause          */
           /* make end the end of the line      */
            location.setEndOffset(current_length);
            /* return a clause terminator        */
            token = this->clause->newToken(TOKEN_EOC, CLAUSEEND_EOL, OREF_NULL, location);
            this->nextLine();                /* step to the next line             */
            break;                           /* have something to return          */
        }
        else if (inch == TOKEN_BLANK )
        {   /* some sort of white space?         */
            /* now go ahead to the next token    */
            inch = this->locateToken(OREF_NULL);
            /* is this blank significant?        */
            if (inch != CLAUSEEND_EOL  &&    /* not at the end                    */
                (isSymbolCharacter(inch) ||   /* and next is a symbol token        */
                 inch == '\"' ||              /* or start of a " quoted literal    */
                 inch == '\'' ||              /* or start of a ' quoted literal    */
                 inch == '('  ||              /* or a left parenthesis             */
                 inch == '['))
            {              /* or a left square bracket          */
                           /* return blank token                */
                token = this->clause->newToken(TOKEN_BLANK, OPERATOR_BLANK, (RexxString *)OREF_BLANK, location);
            }
            else                             /* non-significant blank             */
            {
                continue;                      /* just loop around again            */
            }
        }
        else
        {                             /* non-special token type            */
                                      /* process different token types     */
            tran = translateChar(inch);      /* do the table mapping              */
            if (tran != 0)
            {                 /* have a symbol character?          */
                state = EXP_START;             /* in a clean state now              */
                eoffset = 0;                   /* no exponential sign yet           */
                start = this->line_offset;     /* remember token start position     */
                dot_count = 0;                 /* no periods yet                    */
                for (;;)
                {                     /* loop through the token            */
                    if (inch == '.')             /* have a period?                    */
                    {
                        dot_count++;               /* remember we saw this one          */
                    }

                    /* finite state machine to establish numeric constant (with possible     */
                    /* included sign in exponential form)                                    */

                    switch (state)
                    {             /* process based on current state    */

                        case EXP_START:            /* beginning of scan                 */
                            /* have a digit at the start?        */
                            if (inch >= '0' && inch <= '9')
                            {
                                state = EXP_DIGIT;     /* now scanning digits               */
                            }
                            else if (inch == '.')    /* start with a decimal point?       */
                            {
                                state = EXP_SPOINT;    /* now scanning after the decimal    */
                            }
                            else                     /* must be a non-numeric character   */
                            {
                                state = EXP_EXCLUDED;  /* no longer a number                */
                            }
                            break;                   /* go process the next character     */

                        case EXP_DIGIT:            /* have at least one digit mantissa  */
                            if (inch=='.')           /* decimal point?                    */
                            {
                                state = EXP_POINT;     /* we've hit a decimal point         */
                            }
                            else if (tran=='E')      /* start of exponential?             */
                            {
                                state = EXP_E;         /* remember we've had the 'E' form   */
                            }
                                                       /* non-digit?                        */
                            else if (inch < '0' && inch > '9')
                            {
                                state = EXP_EXCLUDED;  /* no longer scanning a number       */
                            }
                            /* a digit leaves the state unchanged at EXP_DIGIT                      */
                            break;                   /* go get the next character         */

                        case EXP_SPOINT:           /* leading decimal point             */
                            /* not a digit?                      */
                            if (inch < '0' || inch > '9')
                            {
                                state = EXP_EXCLUDED;  /* not a number                      */
                            }
                            else                     /* digit character                   */
                            {
                                state = EXP_POINT;     /* processing a decimal number       */
                            }
                            break;                   /* go process the next character     */

                        case EXP_POINT:            /* have a decimal point              */
                            if (tran == 'E')         /* found the exponential?            */
                            {
                                state = EXP_E;         /* set exponent state                */
                            }
                                                       /* non-digit found?                  */
                            else if (inch < '0' || inch > '9')
                            {
                                state = EXP_EXCLUDED;  /* can't be a number                 */
                            }
                            /* a digit leaves the state unchanged at EXP_POINT                  */
                            break;                   /* go get another character          */

                        case EXP_E:                /* just had an exponent              */
                            /* next one a digit?                 */
                            if (inch >= '0' && inch <= '9')
                            {
                                state = EXP_EDIGIT;    /* now looking for exponent digits   */
                            }
                            /* a sign will be collected by the apparent end of symbol code below */
                            break;                   /* finished                          */

                        case EXP_ESIGN:            /* just had a signed exponent        */
                            /* got a digit?                      */
                            if (inch >= '0' && inch <= '9')
                            {
                                state = EXP_EDIGIT;    /* now looking for the exponent      */
                            }
                            else
                            {
                                state = EXP_EXCLUDED;  /* can't be a number                 */
                            }
                            break;                   /* go get the next digits            */

                        case EXP_EDIGIT:           /* processing the exponent digits    */
                            /* not a digit?                      */
                            if (inch < '0' || inch > '9')
                            {
                                state = EXP_EXCLUDED;  /* can't be a number                 */
                            }
                            break;                   /* go get the next character         */

                            /* once EXP_EXCLUDED is reached the state doesn't change */
                    }
                    this->line_offset++;         /* step the source pointer           */
                                                 /* had a bad exponent part?          */
                    if (eoffset && state == EXP_EXCLUDED)
                    {
                        /* back up the scan pointer          */
                        this->line_offset = eoffset;
                        break;                     /* and we're finished with this      */
                    }
                    if (!MORELINE())             /* reached the end of the line?      */
                    {
                        break;                     /* done processing                   */
                    }

                    inch = GETCHAR();            /* get the next character            */
                    tran = translateChar(inch);  /* translate the next character      */
                    if (tran != 0)               /* good symbol character?            */
                    {
                        continue;                  /* loop through the state machine    */
                    }
                                                   /* check for sign in correct state   */
                    if (state == EXP_E && (inch == '+' || inch == '-'))
                    {
                        /* remember current position         */
                        eoffset = this->line_offset;
                        state = EXP_ESIGN;         /* now looking for the exponent      */
                        this->line_offset++;       /* step past the sign                */
                        if (!MORELINE())
                        {         /* reached the end of the line?      */
                            state = EXP_EXCLUDED;    /* can't be a number                 */
                            break;                   /* quit looping                      */
                        }
                        inch = GETCHAR();          /* get the next character            */
                        tran = translateChar(inch);/* translate the next character      */
                        if (tran != 0)             /* good character?                   */
                        {
                            continue;                /* loop around                       */
                        }
                        else
                        {                     /* bad character                     */
                            state = EXP_EXCLUDED;    /* not a number                      */
                            break;                   /* break out of here                 */
                        }
                    }
                    else
                    {
                        break;                     /* reached a non-symbol character    */
                    }
                }
                /* this must be the end of the symbol - check whether we have too much   */
                /* need to step backward?            */
                if (eoffset && state != EXP_EDIGIT)
                {
                    this->line_offset = eoffset; /* restore the source pointer        */
                }
                                                 /* get the token length              */
                length = this->line_offset - start;
                value = raw_string(length);    /* get the final value               */
                numeric = 0;                   /* not a numeric constant yet        */
                for (i = 0; i < length; i++)
                { /* copy over and translate the value */
                  /* copy over the symbol value        */
                  /* (translating to uppercase         */
                  /* get the next character            */
                    inch = this->current[start + i];
                    if (isSymbolCharacter(inch))       /* normal symbol character (not +/-) */
                    {
                        inch = translateChar(inch);      /* translate to uppercase            */
                    }
                    value->putChar(i, inch);
                }
                value->setUpperOnly();         /* only contains uppercase           */
                                               /* now force to a common string      */
                value = this->commonString(value);
                /* record current position in clause */
                this->clause->setEnd(this->line_number, this->line_offset);
                if (length > (size_t)MAX_SYMBOL_LENGTH)/* result too long?                  */
                {
                    // update the error information
                    clauseLocation = clause->getLocation();
                    /* report the error                  */
                    syntaxError(Error_Name_too_long_name, value);
                }
                inch = this->current[start];   /* get the first character           */
                if (length == 1 && inch == '.')/* have a solo period?               */
                {
                    subclass = SYMBOL_DUMMY;     /* this is the place holder          */
                }
                                                 /* have a digit?                     */
                else if (inch >= '0' && inch <= '9')
                {
                    subclass = SYMBOL_CONSTANT;  /* have a constant symbol            */
                                                 /* can we optimize to an integer?    */
                    if (state == EXP_DIGIT && length < Numerics::DEFAULT_DIGITS)
                    {
                        /* no leading zero or only zero?     */
                        if (inch != '0' || length == 1)
                        {
                            /* we can make this an integer object*/
                            numeric = INTEGER_CONSTANT;
                        }
                    }
                }
                else if (inch == '.')
                {        /* may have an environmental symbol  */
                         /* get the second character          */
                    inch = this->current[start + 1];
                    /* have a digit?                     */
                    if (inch >= '0' && inch <= '9')
                    {
                        subclass = SYMBOL_CONSTANT;/* have a constant symbol            */
                    }
                    else
                    {
                        /* this is an environment symbol     */
                        subclass = SYMBOL_DOTSYMBOL;
                    }
                }
                else
                {                         /* variable type symbol              */
                                          /* set the default extended type     */
                    subclass = SYMBOL_VARIABLE;
                    if (dot_count > 0)
                    {         /* have a period in the name?        */
                              /* end in a dot?                     */
                        if (dot_count == 1 && value->getChar(length-1) == '.')
                        {
                            /* this is a stem variable           */
                            subclass = SYMBOL_STEM;
                        }
                        else                       /* have a compound variable          */
                        {
                            subclass = SYMBOL_COMPOUND;
                        }
                    }
                }
                this->endLocation(location);   /* record the end position           */
                                               /* get a symbol token                */
                token = this->clause->newToken(TOKEN_SYMBOL, subclass, value, location);
                token->setNumeric(numeric);    /* record any numeric side info      */
            }
            /* start of a quoted string?         */
            else if (inch=='\'' || inch=='\"')
            {
                literal_delimiter = inch;      /* save the starting character       */
                start = this->line_offset + 1; /* save the starting point           */
                dot_count = 0;                 /* no doubled quotes yet             */
                type = 0;                      /* working with a straight literal   */
                for (;;)
                {                   /* spin through the string           */
                    this->line_offset++;       /* step the pointer                  */
                    if (!MORELINE())
                    {         /* reached the end of the line?      */
                              /* record current position in clause */
                        this->clause->setEnd(this->line_number, this->line_offset);
                        // update the error information
                        clauseLocation = clause->getLocation();
                        if (literal_delimiter == '\'')
                        {
                            /* raise the appropriate error       */
                            syntaxError(Error_Unmatched_quote_single);
                        }
                        else
                        {
                            /* must be a double quote            */
                            syntaxError(Error_Unmatched_quote_double);
                        }
                    }
                    inch = GETCHAR();          /* get the next character            */
                                               /* is this the delimiter?            */
                    if (literal_delimiter == inch)
                    {
                        /* remember end location             */
                        litend = this->line_offset - 1;
                        this->line_offset++;     /* step to the next character        */
                        if (!MORELINE())         /* end of the line?                  */
                        {
                            break;                 /* we're finished                    */
                        }
                        inch = GETCHAR();        /* get the next character            */
                                                 /* not a doubled quote?              */
                        if (inch != literal_delimiter)
                        {
                            break;                 /* got the end                       */
                        }
                        dot_count++;             /* remember count of doubled quotes  */
                    }
                }
                if (MORELINE())
                {              /* have more on this line?           */
                    inch = GETCHAR();            /* get the next character            */
                                                 /* potentially a hex string?         */
                    if (inch == 'x' || inch == 'X')
                    {
                        this->line_offset++;       /* step to the next character        */
                                                   /* the end of the line, or           */
                                                   /* have another symbol character     */
                        if (MORELINE() && isSymbolCharacter(GETCHAR()))
                        {
                            this->line_offset--;     /* step back to the X                */
                        }
                        else
                        {
                            type = LITERAL_HEX;      /* set the appropriate type          */
                        }
                    }
                    /* potentially a binary string?      */
                    else if (inch == 'b' || inch == 'B')
                    {
                        this->line_offset++;       /* step to the next character        */
                                                   /* the end of the line, or           */
                                                   /* have another symbol character     */
                        if (MORELINE() && isSymbolCharacter(GETCHAR()))
                        {
                            this->line_offset--;     /* step back to the B                */
                        }
                        else
                        {
                            type = LITERAL_BIN;      /* set the appropriate type          */
                        }
                    }
                }
                length = litend - start + 1;   /* calculate the literal length      */
                                               /* record current position in clause */
                this->clause->setEnd(this->line_number, this->line_offset);
                if (type)                      /* need to pack a literal?           */
                {
                    /* compress into packed form         */
                    value = this->packLiteral(start, litend - start + 1, type) ;
                }
                else
                {
                    length = litend - start + 1; /* get length of literal data        */
                                                 /* get the final value string        */
                    value = raw_string(length - dot_count);
                    /* copy over and translate the value */
                    for (i = 0, j = 0; j < length; i++, j++)
                    {
                        /* get the next character            */
                        inch = this->current[start + j];
                        /* same as our delimiter?            */
                        if (inch == literal_delimiter)
                        {
                            j++;                     /* step one extra                    */
                        }
                        value->putChar(i, inch);   /* copy over the literal data        */
                    }
                    /* now force to a common string      */
                    value = this->commonString(value);
                }
                this->endLocation(location);  /* record the end position           */
                /* get a string token                */
                token = this->clause->newToken(TOKEN_LITERAL, 0, value, location);
            }
            else
            {                           /* other special character           */
                this->line_offset++;            /* step past it                      */

                switch (inch)
                {                 /* process operators and punctuation */

                    case ')':                     /* right parenthesis?                */
                        /* this is a special character class */
                        token = this->clause->newToken(TOKEN_RIGHT, 0, OREF_NULL, location);
                        break;

                    case ']':                     /* right square bracket              */
                        /* this is a special character class */
                        token = this->clause->newToken(TOKEN_SQRIGHT, 0, OREF_NULL, location);
                        break;

                    case '(':                     /* left parenthesis                  */
                        /* this is a special character class */
                        token = this->clause->newToken(TOKEN_LEFT, 0, OREF_NULL, location);
                        break;

                    case '[':                     /* left square bracket               */
                        /* this is a special character class */
                        token = this->clause->newToken(TOKEN_SQLEFT, 0, OREF_NULL, location);
                        break;

                    case ',':                     /* comma                             */
                        /* this is a special character class */
                        token = this->clause->newToken(TOKEN_COMMA, 0, OREF_NULL, location);
                        break;

                    case ';':                     /* semicolon                         */
                        /* this is a special character class */
                        token = this->clause->newToken(TOKEN_EOC, CLAUSEEND_SEMICOLON, OREF_NULL, location);
                        break;

                    case ':':                     /* colon                             */
                        /* next one a colon also?            */
                        if (this->nextSpecial(':', location))
                        {
                            /* this is a special character class */
                            token = this->clause->newToken(TOKEN_DCOLON, 0, OREF_NULL, location);
                        }
                        else
                        {
                            /* this is a special character class */
                            token = this->clause->newToken(TOKEN_COLON, 0, OREF_NULL, location);
                        }
                        break;

                    case '~':                     /* message send?                     */
                        /* next one a tilde also?            */
                        if (this->nextSpecial('~', location))
                            /* this is a special character class */
                            token = this->clause->newToken(TOKEN_DTILDE, 0, OREF_NULL, location);
                        else
                            /* this is a special character class */
                            token = this->clause->newToken(TOKEN_TILDE, 0, OREF_NULL, location);
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
                        if (this->nextSpecial('/', location))
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
                        if (this->nextSpecial('*', location))
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
                        if (this->nextSpecial('&', location))
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
                        if (this->nextSpecial('|', location))
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
                        if (this->nextSpecial('=', location))
                        {
                            /* this is an operator class         */
                            token = OPERATOR(STRICT_EQUAL);
                        }
                        else                        /* this is an operator class         */
                        {
                            token = OPERATOR(EQUAL);
                        }
                        break;

                    case '<':                     /* less than sign?                   */
                        /* next one a less than also?        */
                        if (this->nextSpecial('<', location))
                        {
                            /* have an equal sign after that?    */
                            if (this->nextSpecial('=', location))
                            {
                                /* this is an operator class         */
                                token = OPERATOR(STRICT_LESSTHAN_EQUAL);
                            }
                            else                      /* this is an operator class         */
                            {
                                token = OPERATOR(STRICT_LESSTHAN);
                            }
                        }
                        /* next one an equal sign?           */
                        else if (this->nextSpecial('=', location))
                        {
                            /* this is the <= operator           */
                            token = OPERATOR(LESSTHAN_EQUAL);
                        }
                        /* next one a greater than sign?     */
                        else if (this->nextSpecial('>', location))
                        {
                            /* this is the <> operator           */
                            token = OPERATOR(LESSTHAN_GREATERTHAN);
                        }
                        else                        /* this simply the < operator        */
                        {
                            token = OPERATOR(LESSTHAN);
                        }
                        break;

                    case '>':                     /* greater than sign?                */
                        /* next one a greater than also?     */
                        if (this->nextSpecial('>', location))
                        {
                            /* have an equal sign after that?    */
                            if (this->nextSpecial('=', location))
                            {
                                /* this is the >>= operator          */
                                token = OPERATOR(STRICT_GREATERTHAN_EQUAL);
                            }
                            else                      /* this is the >> operator           */
                            {
                                token = OPERATOR(STRICT_GREATERTHAN);
                            }
                        }
                        /* next one an equal sign?           */
                        else if (this->nextSpecial('=', location))
                        {
                            /* this is the >= operator           */
                            token = OPERATOR(GREATERTHAN_EQUAL);
                        }
                        /* next one a less than sign?        */
                        else if (this->nextSpecial('<', location))
                        {
                            /* this is the <> operator           */
                            token = OPERATOR(GREATERTHAN_LESSTHAN);
                        }
                        else                        /* this simply the > operator        */
                        {
                            token = OPERATOR(GREATERTHAN);
                        }
                        break;

                    case '\\':                    /* backslash                         */
                        /* next one an equal sign?           */
                        if (this->nextSpecial('=', location))
                        {
                            /* have an equal sign after that?    */
                            if (this->nextSpecial('=', location))
                            {
                                /* this is the \== operator          */
                                token = OPERATOR(STRICT_BACKSLASH_EQUAL);
                            }
                            else                      /* this is the \= operator           */
                            {
                                token = OPERATOR(BACKSLASH_EQUAL);
                            }
                        }
                        /* next one a greater than sign?     */
                        else if (this->nextSpecial('>', location))
                        {
                            /* have another greater than next?   */
                            if (this->nextSpecial('>', location))
                            {
                                /* this is the \>> operator          */
                                token = OPERATOR(STRICT_BACKSLASH_GREATERTHAN);
                            }
                            else                      /* this is the \> operator           */
                            {
                                token = OPERATOR(BACKSLASH_GREATERTHAN);
                            }
                        }
                        /* next one a less than sign?        */
                        else if (this->nextSpecial('<', location))
                        {
                            /* have another less than next?      */
                            if (this->nextSpecial('<', location))
                            {
                                /* this is the \<< operator          */
                                token = OPERATOR(STRICT_BACKSLASH_LESSTHAN);
                            }
                            else                      /* this is the \< operator           */
                            {
                                token = OPERATOR(BACKSLASH_LESSTHAN);
                            }
                        }
                        else                        /* this is just the NOT operator     */
                        {
                            token = OPERATOR(BACKSLASH);
                        }
                        break;

                    // we accept either of these as alternatives
                    case (unsigned char)0xAA:      /* logical not  (need unsigned cast) */
                    case (unsigned char)0xAC:      /* logical not  (need unsigned cast) */
                        /* next one an equal sign?           */
                        if (this->nextSpecial('=', location))
                        {
                            /* have an equal sign after that?    */
                            if (this->nextSpecial('=', location))
                            {
                                /* this is the \== operator          */
                                token = OPERATOR(STRICT_BACKSLASH_EQUAL);
                            }
                            else                      /* this is the \= operator           */
                            {
                                token = OPERATOR(BACKSLASH_EQUAL);
                            }
                        }
                        /* next one a greater than sign?     */
                        else if (this->nextSpecial('>', location))
                        {
                            /* have another greater than next?   */
                            if (this->nextSpecial('>', location))
                            {
                                /* this is the \>> operator          */
                                token = OPERATOR(STRICT_BACKSLASH_GREATERTHAN);
                            }
                            else                      /* this is the \> operator           */
                            {
                                token = OPERATOR(BACKSLASH_GREATERTHAN);
                            }
                        }
                        /* next one a less than sign?        */
                        else if (this->nextSpecial('<', location))
                        {
                            /* have another less than next?      */
                            if (this->nextSpecial('<', location))
                            {
                                /* this is the \<< operator          */
                                token = OPERATOR(STRICT_BACKSLASH_LESSTHAN);
                            }
                            else                      /* this is the \< operator           */
                            {
                                token = OPERATOR(BACKSLASH_LESSTHAN);
                            }
                        }                           /* this is just the BACKSLASH operator     */
                        else
                        {
                            token = OPERATOR(BACKSLASH);
                        }
                        break;

                    default:                      /* something else found              */
                        /* record current position in clause */
                        this->clause->setEnd(this->line_number, this->line_offset);
                        // update the error information
                        clauseLocation = clause->getLocation();
                        sprintf(badchar, "%c", inch);
                        sprintf(hexbadchar, "%2.2X", inch);
                        /* report the error                  */
                        syntaxError(Error_Invalid_character_char, new_string(badchar), new_string(hexbadchar));
                        break;
                }
            }
        }
        break;                             /* have a token now                  */
    }
    return token;                        /* return the next token             */
}
