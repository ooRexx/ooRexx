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
/* REXX Kernel                                                  Token.c    */
/*                                                                            */
/* Primitive Translator Token Class                                           */
/*                                                                            */
/******************************************************************************/
#include <ctype.h>
#include <string.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "Token.hpp"
#include "SourceFile.hpp"

RexxToken::RexxToken(
    int            classId,            /* class of token                    */
    int            subclass,           /* token subclass                    */
    RexxString     *value,             /* token value                       */
    PLOCATIONINFO  location)           /* token location descriptor         */
/******************************************************************************/
/* Function:  Complete set up of a TOKEN object                               */
/******************************************************************************/
{
  ClearObject(this);                   /* initialize the object             */
  this->hashvalue = HASHOREF(this);    /* fill in the hash value            */
  OrefSet(this, this->value, value);   /* use the provided string value     */
  this->classId = classId;             /* no assigned token class           */
  this->subclass = subclass;           /* no specialization yet             */
  if (location != NULL)                /* have location data?               */
    this->location = *location;        /* copy it over                      */
}

void RexxToken::live()
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  setUpMemoryMark
  memory_mark(this->value);
  cleanUpMemoryMark
}

void RexxToken::liveGeneral()
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  setUpMemoryMarkGeneral
  memory_mark_general(this->value);
  cleanUpMemoryMarkGeneral
}

void RexxToken::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxToken)

  flatten_reference(newThis->value, envelope);

  cleanUpFlatten
}

void RexxToken::setStart(
    size_t  line,                       /* starting line number              */
    size_t offset)                      /* starting line offset              */
/******************************************************************************/
/* Function:  Set a token's starting position as a line/offset pair           */
/******************************************************************************/
{
  this->location.line = line;          /* set the starting line             */
  this->location.offset = offset;      /* and the starting character        */
}

void RexxToken::setEnd(
    size_t  line,                      /* ending line number                */
    size_t  offset)                    /* ending line offset                */
/******************************************************************************/
/* Function:  Set a token's ending position as a line/offset pair             */
/******************************************************************************/
{
  /* Def.2714: set ending position only if it is larger than beginning! CHM */
  if ( (line > this->location.line) ||
       ((line == this->location.line) && (offset > this->location.offset)) )
  {
    this->location.endline = line;       /* set the starting line             */
    this->location.endoffset = offset;   /* and the ending character          */
  } /* endif */
}


/**
 * Check and update this token for the special assignment forms
 * (+=, -=, etc.).
 *
 * @param source The source for the original operator token.
 */
void RexxToken::checkAssignment(RexxSource *source, RexxString *newValue)
{
    // check if the next character is a special assignment shortcut
    if (source->nextSpecial('=', &location))
    {
        // this is a special type, which uses the same subtype.
        classId = TOKEN_ASSIGNMENT;
        // this is the new string value of the token
        value = newValue;
    }
}


void *RexxToken::operator new(size_t size)
/******************************************************************************/
/* Function:  Create a new token object                                       */
/******************************************************************************/
{
  RexxObject *newToken;                /* newly created object              */

                                       /* Get new object                    */
  newToken = new_object(sizeof(RexxToken));
                                       /* Give new object its behaviour     */
  BehaviourSet(newToken, TheTokenBehaviour);
  return newToken;                     /* and return                        */
}

