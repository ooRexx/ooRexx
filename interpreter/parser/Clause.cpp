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
/* Primitive Translator Clause Class                                          */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "ArrayClass.hpp"
#include "Clause.hpp"
#include "ProtectedObject.hpp"


/**
 * Allocate storage for a new Clause object.
 *
 * @param size   The base object size.
 *
 * @return Storage for a new clause object.
 */
void *RexxClause::operator new(size_t size)
{
    return new_object(size, T_Clause);
}


/**
 * Initialize a RexxClause object.
 */
RexxClause::RexxClause()
{
    // make sure we are protected while we are creating this.
    ProtectedObject p(this);
    // allocate an entire array of tokens for use.  We reuse these
    tokens = new_array(INITIAL_SIZE);
    for (size_t i = 0; i < INITIAL_SIZE; i++)
    {
        tokens->append(new RexxToken());
    }

    first = 1;                     // first token is the start
    current = 1;                   // no current token
    size = INITIAL_SIZE;           // set the token cache size
    free = 1;                      // we have a free token
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxClause::live(size_t liveMark)
{
    memory_mark(tokens);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxClause::liveGeneral(MarkReason reason)
{
    memory_mark_general(tokens);
}


/**
 * Set the start location for a clause.
 *
 * @param line   The starting line number.
 * @param offset The line offset.
 */
void RexxClause::setStart(size_t line, size_t offset)
{
    clauseLocation.setStart(line, offset);
}

/**
 * Set the clause end location
 *
 * @param line   The ending line.
 * @param offset The ending offset.
 */
void RexxClause::setEnd(size_t line, size_t offset)
{
    clauseLocation.setEnd(line, offset);
}


/**
 * Remove all tokens that precede the current token from the
 * clause.  Used to break a physical clause into mulitple logical
 * clauses (such as a "label: procedure", which is two clauses.
 */
void RexxClause::trim()
{
    first = current;         // set the first item to the current one
    // update the clause start location to be that of the first token.
    SourceLocation l = ((RexxToken *)((tokens)->get(current)))->getLocation();
    clauseLocation.setStart(l);
}


/**
 * Reset to a new clause.  this just resets
 * the pointers and reuses all of the previous
 * clause tokens.
 */
void RexxClause::newClause()
{
    first = 1;
    current = 1;
    free = 1;
}


/**
 * Return a new token object, with information appropriately
 * filled in
 *
 * @param classId  The token class identifier.
 * @param subclass The token subclass
 * @param value    The (optional) string value.
 * @param l        The token location.
 *
 * @return A filled in token object.
 */
RexxToken *RexxClause::newToken(TokenClass classId, TokenSubclass subclass, RexxString *value, SourceLocation &l)
{

    //. do we need to extend our cache?
    if (free > size)
    {
        // make sure the token array is large enough for the additional token's we're adding.
        tokens->ensureSpace(tokens->size() + EXTEND_SIZE);

        for (size_t i = 0; i < EXTEND_SIZE; i++)
        {
            tokens->append(new RexxToken());
        }
        size += EXTEND_SIZE;
    }

    // get the first free token and call the in-memory constructor for this
    /* get the first free token          */
    RexxToken *token = (RexxToken *)tokens->get(free++);
    // construct the token and return
    ::new ((void *)token) RexxToken(classId, l, subclass, value);
    return token;
}

/**
 * Return the next non-while space token in the clause
 * list.
 *
 * @return The next meaningful token.
 */
RexxToken *RexxClause::nextRealToken()
{
    // loop until we hit a non-blank
    RexxToken *token = next();
    while (token->isBlank())
    {
        token = next();
    }
    return token;
}

