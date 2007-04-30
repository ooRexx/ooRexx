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
/* REXX Kernel                                                     RexxInstruction.c */
/*                                                                            */
/* Primitive Translator Abstract Instruction Code                             */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include "RexxCore.h"
#include "RexxInstruction.hpp"
#include "Clause.hpp"

RexxInstruction::RexxInstruction(
    RexxClause *clause,                /* associated clause object          */
    int type)                          /* type of instruction               */
/******************************************************************************/
/* Function:  Common initialization for instruction objects                   */
/******************************************************************************/
{
  LOCATIONINFO clauseLocation;         /* clause location information       */

  ClearObject(this);                   /* start out clean                   */
  this->hashvalue = 0;                 /* clear out the instruction info    */
                                       /* record the instruction type       */
  this->instructionInfo.type = (UCHAR)type;
  if (clause != OREF_NULL) {           /* have a clause object?             */
                                       /* fill in default location info     */
    clause->getLocation(&clauseLocation);
                                       /* copy the location information     */
    this->lineNumber = clauseLocation.line;
    this->offset = clauseLocation.offset;
    this->endLine = clauseLocation.endline;
    this->endOffset = clauseLocation.endoffset;
  }
  else
    this->lineNumber = 0;              /* zero the line number              */
}

void RexxInstruction::live(void)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  setUpMemoryMark
  memory_mark(this->nextInstruction);
  cleanUpMemoryMark
}

void RexxInstruction::liveGeneral(void)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  setUpMemoryMarkGeneral
                                       /* do common marking                 */
  memory_mark_general(this->nextInstruction);
  cleanUpMemoryMarkGeneral
}

void RexxInstruction::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxInstruction)

  flatten_reference(newThis->nextInstruction, envelope);

  cleanUpFlatten
}

void * RexxInstruction::operator new(size_t size)
/******************************************************************************/
/* Function:  Create a new translator object                                  */
/******************************************************************************/
{
  RexxObject *newObject;
  newObject = new_object(size);        /* Get new object                    */
                                       /* object parse_assignment behaviour */
  OrefSet(newObject,newObject->behaviour, TheInstructionBehaviour);
  return newObject;
}

void RexxInstructionExpression::live(void)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  setUpMemoryMark
  memory_mark(this->nextInstruction);
  memory_mark(this->expression);
  cleanUpMemoryMark
}

void RexxInstructionExpression::liveGeneral(void)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  setUpMemoryMarkGeneral
                                       /* do common marking                 */
  memory_mark_general(this->nextInstruction);
  memory_mark_general(this->expression);
  cleanUpMemoryMarkGeneral
}

void RexxInstructionExpression::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxInstructionExpression)

  flatten_reference(newThis->nextInstruction, envelope);
  flatten_reference(newThis->expression, envelope);

  cleanUpFlatten
}

void RexxInstruction::getLocation(
    PLOCATIONINFO outLocation)         /* returned location                 */
/******************************************************************************/
/* Function:  Retrieve an instructions location information                   */
/******************************************************************************/
{
  outLocation->line = this->lineNumber;/* copy each field over              */
  outLocation->offset = this->offset;
  outLocation->endline = this->endLine;
  outLocation->endoffset = this->endOffset;
}

void RexxInstruction::setLocation(
    PLOCATIONINFO newLocation)         /* new location information          */
/******************************************************************************/
/* Function:  Set an instructions location information                        */
/******************************************************************************/
{
  this->lineNumber = newLocation->line;/* copy each field over              */
  this->offset = newLocation->offset;
  this->endLine = newLocation->endline;
  this->endOffset = newLocation->endoffset;
}
