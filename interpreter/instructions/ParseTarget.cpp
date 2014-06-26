/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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
/* Primitive Procedure Parse Trigger Class                                    */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "RexxActivation.hpp"
#include "ParseTarget.hpp"
#include "ParseInstruction.hpp"

/**
 * Initialize a ParseTarget instance.
 *
 * @param _string    The string being parsed.
 * @param _arglist   The argument list context, if doing PARSE ARG.
 * @param _argcount  the count of arguments.
 * @param _translate The translation flags.
 * @param multiple   Indicates we have multiple strings to parse.
 * @param context    The current execution context.
 * @param s          The evaluation stack.
 */
void RexxTarget::init(RexxObject *_string, RexxObject **_arglist, size_t _argcount,
    size_t _translate, bool multiple, RexxActivation *context, RexxExpressionStack *s)
{
    translate = _translate;
    arglist = _arglist;
    argcount = _argcount;
    string = (RexxString *)_string;
    next_argument = 1;
    stack = s;
    stackTop = s->location();      // save the stack top for resets
    next(context);
}

/**
 * Step to the "next" string to parse, resetting all of the
 * cursor movement values.
 *
 * @param context The current execution context.
 */
void RexxTarget::next(RexxActivation *context)
/******************************************************************************/
/* Function:  Step to the "next" string to parse, resetting all of the        */
/*            cursor movement values.                                         */
/******************************************************************************/
{
    // if we have an argument list and we have an argument value in that
    // position, grab it.  Otherwise, our target string is just ""
    if (arglist != OREF_NULL)
    {
        if (next_argument > argcount)
        {
            string = OREF_NULLSTRING;
        }
        else
        {
            // get the next element
            string = (RexxString *)arglist[next_argument - 1];
            // omitted argument? use a null string
            if (string == OREF_NULL)
            {
                string = OREF_NULLSTRING;
            }
        }
    }
    else
    {
        // not PARSE ARG, so beyond the first template is always a null string
        if (next_argument != 1)
        {
            string = OREF_NULLSTRING;
        }
    }
    next_argument++;
    // make sure we have a string
    string = (RexxString *)REQUEST_STRING(string);
    // now handle translation options
    if (translate[parse_upper])
    {
        string = string->upper();
    }
    else if (translate[parse_lower])
    {
        string = string->lower();
    }

    // reset the stack to the entry top, and push this value on to protect it.
    stack->setTop(stackTop);
    stack->push(string);

    // if tracing results or intermediates, show the string being parsed.
    context->traceResult(string);

    // reset all of the parsing positions
    start = 0;
    pattern_end = 0;
    pattern_start = 0;
    // get the length for quicker access
    string_length = string->getLength();
    subcurrent = 0;                // no sub piece to process yet
}

/**
 * Move the current cursor to the string end.
 */
void RexxTarget::moveToEnd()
{
    // start from end of last pattern
    start = pattern_end;
    // pattern at the end too
    pattern_end = string_length;
    // no pattern length either
    pattern_start = string_length;
    // string end is at end also
    end = string_length;
    // set starting point
    subcurrent = start;
}

/**
 * Move the parse pointer forward
 *
 * @param offset The offset to move.
 */
void RexxTarget::forward(stringsize_t offset)
{
    // the start position is the last postion and the end
    // position is the start position + the offset we're moving
    start = pattern_start;
    end = start + offset;

    // if we've gone past the end, trunk to the string length
    if (end >= string_length)
    {
        end = string_length;
    }

    // if there was no forward movement, the special Rexx rule kicks in
    if (end <= start)      /* no forward movement?              */
    {
        // we match to the end, and start from the current start next time
        end = string_length;
        pattern_start = start;
    }
    // normal movement, everthing moves from the end
    else
    {
        pattern_start = end;
    }

    // our pattern is zero length because it is not a string match.
    pattern_end = pattern_start;
    // set the subpiece pointer
    subcurrent = start;
}


void RexxTarget::forwardLength(
    stringsize_t offset)               /* offset to move                    */
/******************************************************************************/
/* Arguments:  distance to move the parse pointer                             */
/******************************************************************************/
{
    start = pattern_start;   /* start position is last position   */
    end = start + offset;    /* set the end position              */
    if (end >= string_length)/* take us past the end?             */
    {
        end = string_length;   /* just use the end position         */
    }
    pattern_start = end;     /* this is new start position        */
                                         /* and have a zero length pattern    */
    pattern_end = pattern_start;
    subcurrent = start;      /* set the subpiece pointer          */
}


void RexxTarget::absolute(
    stringsize_t offset)               /* offset to move                    */
/******************************************************************************/
/* Arguments:  new parse position                                             */
/******************************************************************************/
{
    if (offset > 0)                      /* positive offset?                  */
    {
        offset--;                          /* make origin zero                  */
    }
    start = pattern_end;     /* start position is last position   */
    if ((size_t)offset <= start) /* backward movement?                */
    {
        end = string_length;   /* matches to the end                */
        pattern_start = offset;      /* pattern start is actual position  */
    }
    else                               /* forward movement                  */
    {
        end = offset;                /* use the specified position        */
                                           /* take us past the end?             */
        if (end >= string_length)
        {
            /* just use the end position         */
            end = string_length;
        }
        pattern_start = end;   /* this is new start position        */
    }
    /* and have a zero length pattern    */
    pattern_end = pattern_start;
    subcurrent = start;      /* set the subpiece pointer          */
}

void RexxTarget::backward(
    stringsize_t offset)               /* offset to move                    */
/******************************************************************************/
/* Arguments:  distance to move the parse pointer                             */
/******************************************************************************/
{
    start = pattern_start;   /* start position is last position   */
    end = string_length;     /* negatives always use to the end   */
    /* go past start of string?          */
    if (offset > pattern_start)
    {
        pattern_start = 0;         /* this resets to the start          */
    }
    else
    {
        pattern_start -= offset;   /* just back up                      */
    }
    /* and have a zero length pattern    */
    pattern_end = pattern_start;
    subcurrent = start;      /* set the subpiece pointer          */
}


void RexxTarget::backwardLength(
    stringsize_t offset)               /* offset to move                    */
/******************************************************************************/
/* Arguments:  distance to move the parse pointer                             */
/******************************************************************************/
{
    start = pattern_start;   /* start position is last position   */
    end = string_length;     /* negatives always use to the end   */
                                         /* go past start of string?          */
    if (offset > pattern_start)
    {
        start = 0;
    }
    else
    {
        start = pattern_start - offset;
    }
    end = pattern_start;     // the end is the starting location
                                         /* and have a zero length pattern    */
    pattern_end = pattern_start;
    subcurrent = start;      /* set the subpiece pointer          */
}


void RexxTarget::search(
    RexxString *needle)                /* target search string              */
/******************************************************************************/
/* Arguments:  target location string                                         */
/******************************************************************************/
{
    /* start position for strings is the */
    start = pattern_end;     /* end of the last pattern           */
                                         /* search for the string trigger     */
    end = string->pos(needle, start);
    if (end == 0)                /* not found?                        */
    {
        end = string_length;   /* that is the end position          */
                                           /* next pattern is end also          */
        pattern_start = string_length;
        /* and the end pattern is also there */
        pattern_end = string_length;
    }
    else
    {
        end--;                       /* convert to origin zero            */
        pattern_start = end;   /* this is the starting point        */
                                           /* end is start + trigger length     */
        pattern_end = pattern_start + needle->getLength();
    }
    subcurrent = start;      /* set the subpiece pointer          */
}

void RexxTarget::caselessSearch(
    RexxString *needle)                /* target search string              */
/******************************************************************************/
/* Arguments:  target location string                                         */
/******************************************************************************/
{
    /* start position for strings is the */
    start = pattern_end;     /* end of the last pattern           */
                                         /* search for the string trigger     */
    end = string->caselessPos(needle, start);
    if (end == 0)                /* not found?                        */
    {
        end = string_length;   /* that is the end position          */
                                           /* next pattern is end also          */
        pattern_start = string_length;
        /* and the end pattern is also there */
        pattern_end = string_length;
    }
    else
    {
        end--;                       /* convert to origin zero            */
        pattern_start = end;   /* this is the starting point        */
                                           /* end is start + trigger length     */
        pattern_end = pattern_start + needle->getLength();
    }
    subcurrent = start;      /* set the subpiece pointer          */
}

RexxString *RexxTarget::getWord()
/******************************************************************************/
/*  Returned:  Next word extracted from parsed substring                      */
/******************************************************************************/
{
    RexxString *word;                    /* extracted word                    */
    size_t  length;                      /* word length                       */
    const char *scan;                    /* scan pointer                      */
    const char *endScan;                 /* end of string location            */

    if (subcurrent >= end)   /* already used up?                  */
    {
        word = OREF_NULLSTRING;            /* just return a null string         */
    }
    else                               /* need to scan off a word           */
    {
        /* point to the current position     */
        scan = string->getStringData() + subcurrent;
        /* and the scan end point            */
        endScan = string->getStringData() + end;
        /* NOTE:  All string objects have a terminating NULL, so the */
        /* scan for nonblanks is guaranteed to stop before getting into */
        /* trouble, which eliminates the need to check against the */
        /* length */
        while (*scan == ' ' || *scan == '\t')
        {
            scan++;                          /* step for each match found         */
        }
        /* set the new location              */
        subcurrent = scan - string->getStringData();
        if (subcurrent >= end) /* already used up?                  */
        {
            word = OREF_NULLSTRING;          /* just return a null string         */
        }
        else                             /* have a real word                  */
        {
            /* look for the next blank           */
            endScan = NULL;
            const char *scanner = scan;
            const char *endPosition = string->getStringData() + end;
            while (scanner < endPosition)
            {
                if (*scanner == ' ' || *scanner == '\t')
                {
                    endScan = scanner;
                    break;
                }
                scanner++;
            }
            if (endScan == NULL)           /* no match?                         */
            {
                /* calculate the length              */
                length = end - subcurrent;
                subcurrent = end;  /* use the rest of it                */
            }
            else
            {
                /* set the new location              */
                subcurrent = endScan - string->getStringData();
                length = endScan - scan;       /* calculate from the pointers       */
            }
            /* step past terminating blank...note*/
            /* that this is done unconditionally,*/
            /* but safely, since the check at the*/
            /* start will catch the out of bounds*/
            subcurrent++;              /* condition                         */
                                             /* this the entire string?           */
            if (length == string_length)
            {
                word = string;           /* just return it directly           */
            }
            else
            {
                /* extract the subpiece              */
                word = new_string(scan, length);
            }
        }
    }
    return word;                         /* give this word back               */
}

void RexxTarget::skipWord()
/******************************************************************************/
/*  Returned:  Next word extracted from parsed substring                      */
/******************************************************************************/
{
    const char *scan;                    /* scan pointer                      */
    const char *endScan;                 /* end of string location            */

    if (subcurrent < end)  /* something left?                   */
    {
        /* point to the current position     */
        scan = string->getStringData() + subcurrent;
        /* and the scan end point            */
        endScan = string->getStringData() + end;
        /* NOTE:  All string objects have a terminating NULL, so the */
        /* scan for nonblanks is guaranteed to stop before getting into */
        /* trouble, which eliminates the need to check against the */
        /* length */
        while (*scan == ' ' || *scan == '\t')
        {
            scan++;                          /* step for each match found         */
        }
        /* set the new location              */
        subcurrent = scan - string->getStringData();
        if (subcurrent < end)/* something left over?              */
        {
            /* look for the next blank           */
            endScan = NULL;
            const char *scanner = scan;
            const char *endPosition = string->getStringData() + end;
            while (scanner < endPosition)
            {
                if (*scanner == ' ' || *scanner == '\t')
                {
                    endScan = scanner;
                    break;
                }
                scanner++;
            }
            if (endScan == NULL)             /* no match?                         */
            {
                subcurrent = end;  /* use the rest of it                */
            }
            else
            {
                /* set the new location              */
                subcurrent = endScan - string->getStringData();
            }
            /* step past terminating blank...note*/
            /* that this is done unconditionally,*/
            /* but safely, since the check at the*/
            /* start will catch the out of bounds*/
            subcurrent++;              /* condition                         */
        }
    }
}

RexxString *RexxTarget::remainder()
/******************************************************************************/
/*  Returned:  Remaining portion of parsed substring                          */
/******************************************************************************/
{
    RexxString *word;                    /* extracted word                    */
    size_t  length;                      /* length to extract                 */

    if (subcurrent >= end)   /* already used up?                  */
    {
        word = OREF_NULLSTRING;            /* just return a null string         */
    }
    else                               /* extract the remaining piece       */
    {
        /* calculate the length              */
        length = end - subcurrent;
        if (length == string_length) /* this the entire string?           */
        {
            word = string;             /* just return it directly           */
        }
        else                               /* need to extract a piece           */
        {
            word = string->extract(subcurrent, length);
        }
        subcurrent = end;      /* eat the remainder piece           */
    }
    return word;                         /* give this word back               */
}
