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
/* REXX Translator                                                Clause.c    */
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

#define INITIAL_SIZE  50               /* initial size of the token cache   */
#define EXTEND_SIZE   25               /* size to extend when table fills   */

RexxClause::RexxClause()
/******************************************************************************/
/* Function:  Finish initialization of a REXX clause object                   */
/******************************************************************************/
{
                                         /* an array for the tokens           */
    OrefSet(this, this->tokens, new_arrayOfObject(sizeof(RexxToken), INITIAL_SIZE, T_Token));
    this->first = 1;                     /* first token is the start          */
    this->current = 1;                   /* no current token                  */
    this->size = INITIAL_SIZE;           /* set the token cache size          */
    this->free = 1;                      /* we have a free token              */
}

void RexxClause::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
    memory_mark(this->tokens);
}

void RexxClause::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
    memory_mark_general(this->tokens);
}

void RexxClause::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxClause);

    flatten_reference(newThis->tokens, envelope);

  cleanUpFlatten
}

void RexxClause::setStart(
    size_t  line,                      /* starting line number              */
    size_t  offset)                    /* starting line offset              */
/******************************************************************************/
/* Function:  Set a clause's starting position as a line/offset pair          */
/******************************************************************************/
{
    this->clauseLocation.setStart(line, offset);
}

void RexxClause::setEnd(
    size_t  line,                      /* ending line number                */
    size_t  offset)                    /* ending line offset                */
/******************************************************************************/
/* Function:  Set a clause's ending position as a line/offset pair            */
/******************************************************************************/
{
    clauseLocation.setEnd(line, offset);
}

void RexxClause::trim()
/******************************************************************************/
/* Function:  Remove all tokens that precede the current token from the       */
/*            clause.  Used to break a physical clause into mulitple logical  */
/*            clauses (such as a "label: procedure", which is two clauses.    */
/******************************************************************************/
{
    this->first = this->current;         /* set first item to current         */
                                         /* get first token location          */
    SourceLocation l = ((RexxToken *)((this->tokens)->get(this->current)))->getLocation();
    /* update the clause location info   */
    clauseLocation.setStart(l);
}

void RexxClause::newClause()
/******************************************************************************/
/* Function :  Reset a clause object for the "next" clause of the program.    */
/*             This involves resetting all of the caching information for     */
/*             token allocation.                                              */
/******************************************************************************/
{
    this->first = 1;                     /* first token is the start          */
    this->current = 1;                   /* no current token                  */
    this->free = 1;                      /* we have a free token              */
}

RexxToken *RexxClause::newToken(
    int            classId,            /* class of the token                */
    int            subclass,           /* subclass of the token             */
    RexxString    *value,              /* associated string value           */
    SourceLocation &l)                 /* location of the token             */
/******************************************************************************/
/* Function :  Return a new token object, with information appropriately      */
/*             filled in                                                      */
/******************************************************************************/
{
    if (this->free > this->size)
    {       /* need to extend our cache?         */
            /* allocate a larger array  */
            /* first a bulk array of tokens      */
        RexxArray *newTokens = new_arrayOfObject(sizeof(RexxToken), EXTEND_SIZE, T_Token);
        ProtectedObject p(newTokens);
        RexxArray *newarray = (RexxArray *)this->tokens->join(newTokens);
        this->size += EXTEND_SIZE;         /* bump the cache size               */
                                           /* replace the old array             */
        OrefSet(this, this->tokens, newarray);
    }
    /* get the first free token          */
    RexxToken *token = (RexxToken *)this->tokens->get(this->free);
    this->free++;                        /* step the free location            */
                                         /* fill in the token                 */
    new ((void *)token) RexxToken(classId, subclass, value, l);
    return token;                        /* send the token back               */
}

RexxToken *RexxClause::nextRealToken()
/******************************************************************************/
/*  Function:  Return next non-blank token in the clause list                 */
/******************************************************************************/
{
    RexxToken *token = this->next();                /* get the next token                */
    while (token->classId == TOKEN_BLANK)/* now loop until get a non-blank    */
    {
        token = this->next();              /* get the next token                */
    }
    return token;                        /* return retrieved token            */
}

void *RexxClause::operator new(size_t size)
/******************************************************************************/
/* Function:  Create a new translator object                                  */
/******************************************************************************/
{
    return new_object(size, T_Clause);
}

