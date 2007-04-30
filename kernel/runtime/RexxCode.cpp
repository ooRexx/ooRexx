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
/* REXX Kernel                                                  RexxCode.c     */
/*                                                                            */
/* Primitive Method Class                                                     */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "RexxCore.h"
#include "RexxCode.hpp"
#include "ArrayClass.hpp"
#include "DirectoryClass.hpp"
#include "RexxInstruction.hpp"
#include "SourceFile.hpp"
#include <ctype.h>

RexxCode::RexxCode(
     RexxSource      * source,         /* source object                     */
     RexxInstruction * start,          /* start of the code tree            */
     RexxDirectory   * labels,         /* method labels                     */
     size_t            maxstack,       /* max operator stack size           */
     size_t            variable_index) /* save of the vdict                 */
/******************************************************************************/
/* Function:  Initialize a rexxmethod code object                             */
/******************************************************************************/
{
  OrefSet(this, this->u_source, source); /* save the program source         */
  OrefSet(this, this->start, start);   /* the parse tree                    */
  OrefSet(this, this->labels, labels); /* the method's labels               */
  /* save the stack info               */
  this->maxStack = (unsigned short)maxstack;
  this->vdictSize = variable_index;    /* save the initial vdict size       */
}

void RexxCode::live()
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  setUpMemoryMark
  memory_mark(this->u_source);
  memory_mark(this->start);
  memory_mark(this->labels);
  cleanUpMemoryMark
}

void RexxCode::liveGeneral()
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  setUpMemoryMarkGeneral
  memory_mark_general(this->u_source);
  memory_mark_general(this->start);
  memory_mark_general(this->labels);
  cleanUpMemoryMarkGeneral
}

void RexxCode::flatten(RexxEnvelope * envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxCode)

   flatten_reference(newThis->u_source, envelope);
   flatten_reference(newThis->start, envelope);
   flatten_reference(newThis->labels, envelope);

  cleanUpFlatten
}

RexxArray * RexxCode::sourceRexx()
/******************************************************************************/
/* Function:  Extract the source from a method from the source object as an   */
/*            array of strings.                                               */
/******************************************************************************/
{
  LOCATIONINFO     location;           /* location information              */
  LOCATIONINFO     end_location;       /* ending location                   */
  RexxInstruction *current;            /* current instruction               */

  if (this->start == OREF_NULL)        /* empty method?                     */
    return new_array(0);               /* just return an empty array        */
  this->start->getLocation(&location); /* get its location info             */
  current = this->start;               /* point to the beginning            */
                                       /* while not at the last one         */
  while (current->nextInstruction != OREF_NULL) {
    current = current->nextInstruction;/* step to the next one              */
  }

  current->getLocation(&end_location); /* get the end location              */
                                       /* copy over the ending position     */
  location.endline = end_location.endline;
  location.endoffset = end_location.endoffset;
                                       /* go extract the source array       */
  return this->u_source->extractSource(&location);
}

RexxString * RexxCode::getProgramName()
/******************************************************************************/
/* Function:  Return the name of the program that contains this method.       */
/**REXX****************************************************************************/
{
                                       /* retrieve this from the source     */
  return this->u_source->getProgramName();
}

void * RexxCode::operator new(size_t size)
/******************************************************************************/
/* Function:  Create a new rexx method code instance                          */
/******************************************************************************/
{
  RexxObject * newMethod;              /* newly created object              */

  newMethod = new_object(size);        /* Get new object                    */
                                       /* Give new object its behaviour     */
  BehaviourSet(newMethod, TheRexxCodeBehaviour);
  return newMethod;                    /* return the new method             */
}

