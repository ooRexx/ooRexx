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
#include "RexxCore.h"
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "RexxActivation.hpp"
#include "ParseTarget.hpp"
#include "ParseInstruction.hpp"
#include "MethodArguments.hpp"

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
    FlagSet<ParseFlags, 32>_translate, bool multiple, RexxActivation *context, ExpressionStack *s)
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
{
    // if we have an argument list and we have an argument value in that
    // position, grab it.  Otherwise, our target string is just ""
    if (arglist != OREF_NULL)
    {
        if (next_argument > argcount)
        {
            string = GlobalNames::NULLSTRING;
        }
        else
        {
            // get the next element
            string = (RexxString *)arglist[next_argument - 1];
            // omitted argument? use a null string
            if (string == OREF_NULL)
            {
                string = GlobalNames::NULLSTRING;
            }
        }
    }
    else
    {
        // not PARSE ARG, so beyond the first template is always a null string
        if (next_argument != 1)
        {
            string = GlobalNames::NULLSTRING;
        }
    }
    next_argument++;
    // make sure we have a string
    string = string->requestString();
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
    // set starting point for breaking up into words
    subcurrent = start;
}


/**
 * Move the parse pointer forward by a relative offset
 * (unsigned numeric trigger)
 *
 * @param offset The offset to move.
 */
void RexxTarget::forward(size_t offset)
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
    if (end <= start)
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
    // the section we parse into words starts with the last match position.
    subcurrent = start;
}


/**
 * Move the parsing position forward by a relative offset
 * (>numeric trigger), which uses different rules for end <=
 * start.
 *
 * @param offset The offset amount to move.
 */
void RexxTarget::forwardLength(size_t offset)
{
    // the start position is our last start position (which will
    // be the beginning of the last string match.
    start = pattern_start;
    end = start + offset;

    // perform movement past the string end tests.
    if (end >= string_length)
    {
        end = string_length;
    }

    // The match pattern is the end position and is a zero-length
    // string.
    pattern_start = end;
    pattern_end = pattern_start;
    // The piece we parse into words begins at the last match position.
    subcurrent = start;
}


/**
 * Move to an absolute string postion (unsigned numeric
 * trigger).
 *
 * @param offset The target string position.
 */
void RexxTarget::absolute(size_t offset)
{
    // make the offset origin zero (but be careful of a 0 offset already)
    if (offset > 0)
    {
        offset--;
    }

    // our parsing start position is the end of the previous match
    start = pattern_end;
    // if we are moving backward, we match to the end of the string
    // and reset things.
    if (offset <= start)
    {
        end = string_length;
        pattern_start = offset;
    }
    // forward movement.  Use the string between the end of the last
    // match and the current match position
    else
    {
        end = offset;
        // cap at the end of the string
        if (end >= string_length)
        {
            end = string_length;
        }

        pattern_start = end;
    }
    // this is a zero length match pattern.
    pattern_end = pattern_start;
    subcurrent = start;
}

/**
 * Move backward by an offset.  This is the -n form, which
 * employs the backward movement rule to cause a match to the
 * end of the string.
 *
 * @param offset The offset amount to move.
 */
void RexxTarget::backward(size_t offset)
{
    // the current string starts with the last pattern and goes to
    // the end
    start = pattern_start;
    end = string_length;

    // relative movement is from start of last match.  If
    // this would take us past the beginning, just go to the start.
    if (offset > pattern_start)
    {
        pattern_start = 0;
    }
    else
    {
        pattern_start -= offset;
    }
    // all numeric patterns are zero length
    pattern_end = pattern_start;
    subcurrent = start;
}


/**
 * Backward movement for a <n pattern.  This does not
 * use the "match to the end of the string" semantics
 * that the -n pattern used.
 *
 * @param offset The offset to move.
 */
void RexxTarget::backwardLength(size_t offset)
{
    // the start position will be the final movement postion.
    // the end position will be the last match position, which
    // allows parsing characters immediately before a match position.

    // move relative to the last match position, but we cannot
    // go past the beginning.
    if (offset > pattern_start)
    {
        start = 0;
    }
    else
    {
        start = pattern_start - offset;
    }
    // the end is the old last match position
    end = pattern_start;
    // the new match position is the current start
    pattern_start = start;
    // the pattern end is the old pattern start
    pattern_end = pattern_start;
    // and set the location for word parsing.
    subcurrent = start;
}


/**
 * Seach the string from the end of the last match.
 *
 * @param needle The search needle.
 */
void RexxTarget::search(RexxString *needle)
{
    // all searches start from the end of the last match position
    // and so does the section to be parsed up
    start = pattern_end;
    // just use pos to search
    end = string->pos(needle, start);
    // if not found, we match to the end of the string
    if (end == 0)
    {
        end = string_length;
        pattern_start = string_length;
        pattern_end = string_length;
    }
    else
    {
        // make origin zero
        end--;
        // mark the pattern start
        pattern_start = end;
        // the pattern end is start + needle length
        pattern_end = pattern_start + needle->getLength();
    }
    // again, prepare for word operations
    subcurrent = start;
}


/**
 * Perform a caseless search for pattern (used with
 * PARSE CASELESS).
 *
 * @param needle The target needle.
 */
void RexxTarget::caselessSearch(RexxString *needle)
{
    // all searches start from the end of the last match position
    // and so does the section to be parsed up
    start = pattern_end;
    // just use pos to search
    end = string->caselessPos(needle, start);
    // if not found, we match to the end of the string
    if (end == 0)
    {
        end = string_length;
        pattern_start = string_length;
        pattern_end = string_length;
    }
    else
    {
        // make origin zero
        end--;
        // mark the pattern start
        pattern_start = end;
        // the pattern end is start + needle length
        pattern_end = pattern_start + needle->getLength();
    }
    // again, prepare for word operations
    subcurrent = start;
}


/**
 * Get the next word from the parsed substring.
 *
 * @return The next blank delimited word, or a NULLSTRING if
 *         we've used up the string section.
 */
RexxString *RexxTarget::getWord()
{
    // already moved past the end of the string?  This is a NULLSTRING result
    if (subcurrent >= end)
    {
        return GlobalNames::NULLSTRING;
    }
    else                               /* need to scan off a word           */
    {
        // get some pointers for scanning
        const char *scan = string->getStringData() + subcurrent;
        const char *endScan = string->getStringData() + end;
        // NOTE:  All string objects have a terminating NULL, so the
        // scan for nonblanks is guaranteed to stop before getting into
        // trouble, which eliminates the need to check against the
        // length */

        // we recognize both blanks and tabs as whitespace
        while (*scan == ' ' || *scan == '\t')
        {
            scan++;
        }
        // set the new location
        subcurrent = scan - string->getStringData();
        // scanned past the end?  just return a null string.
        if (subcurrent >= end)
        {
            return GlobalNames::NULLSTRING;
        }
        // we have the start of a real word here...
        else
        {
            // now we're looking for a blank for the end of the string.
            // scan is the start of the word, endScan remains the same,
            // endPosition will be the end of the word
            const char *scanner = scan;
            const char *endPosition = NULL;
            // scan for a blank
            while (scanner < endScan)
            {
                if (*scanner == ' ' || *scanner == '\t')
                {
                    endPosition = scanner;
                    break;
                }
                scanner++;
            }

            size_t length = 0;
            // if no blank was found, we use the rest of the pattern piece.
            if (endPosition == NULL)
            {
                length = end - subcurrent;
                // we've used up this string section
                subcurrent = end;
            }
            // consume that piece of the string
            else
            {
                // set the new location parsing location (NOTE: we also consume this
                // blank character for doing our next scan bit).
                subcurrent = endPosition - string->getStringData() + 1;
                length = endPosition - scan;
            }

            // is this the entire string?  Just return it directly rather than
            // create a new string object.
            if (length == string_length)
            {
                return string;
            }
            else
            {
                // extract a subpiece from the string
                return new_string(scan, length);
            }
        }
    }
    // should never get here.
    return GlobalNames::NULLSTRING;
}

/**
 * Skip a word in the parse string section.  This is
 * used for the placeholder '.' pattern.
 */
void RexxTarget::skipWord()
{
    // something left to process?
    if (subcurrent < end)
    {
        // get some scanning pointers
        const char *scan = string->getStringData() + subcurrent;
        const char *endScan = string->getStringData() + end;
        // NOTE:  All string objects have a terminating NULL, so the
        // scan for nonblanks is guaranteed to stop before getting into
        // trouble, which eliminates the need to check against the
        // length
        while (*scan == ' ' || *scan == '\t')
        {
            scan++;
        }
        // set the new location
        subcurrent = scan - string->getStringData();
        const char *endPosition = NULL;
        // something left?  Then we need to establish the word end position
        if (subcurrent < end)
        {
            // look for the next blank
            const char *endPosition = NULL;
            while (scan < endScan)
            {
                if (*scan == ' ' || *scan == '\t')
                {
                    endPosition = scan;
                    break;
                }
                scan++;
            }
            // if we did not found anything, this is used up
            if (endPosition == NULL)
            {
                subcurrent = end;
            }
            // we know the end position...also consume the terminating white space
            else
            {
                subcurrent = endPosition - string->getStringData() + 1;
            }
        }
    }
}

/**
 * Return the remainder of the parsed section of string.
 *
 * @return The string remainder.
 */
RexxString *RexxTarget::remainder()
{
    // already used up?  this is ""
    if (subcurrent >= end)
    {
        return GlobalNames::NULLSTRING;
    }
    // get the rest of the current piece

    // get the length, and then clear the string section
    size_t length = end - subcurrent;
    size_t offset = subcurrent;
    // consume the rest
    subcurrent = end;
    // if this is the entire string, we can just return
    // the string without making a new string object
    if (length == string_length)
    {
        return string;
    }

    // extract a new string piece
    RexxString *word = string->extract(offset, length);
    return word;
}
