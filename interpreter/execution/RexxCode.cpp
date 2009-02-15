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
/* REXX Kernel                                                 RexxCode.cpp   */
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
#include "ActivityManager.hpp"
#include "RexxActivation.hpp"
#include <ctype.h>



RexxCode::RexxCode(
     RexxSource      * _source,        /* source object                     */
     RexxInstruction * _start,         /* start of the code tree            */
     RexxDirectory   * _labels,        /* method labels                     */
     size_t            maxstack,       /* max operator stack size           */
     size_t            variable_index) /* save of the vdict                 */
/******************************************************************************/
/* Function:  Initialize a rexxmethod code object                             */
/******************************************************************************/
{
  OrefSet(this, this->source, _source); /* save the program source         */
  OrefSet(this, this->start, _start);   /* the parse tree                    */
  OrefSet(this, this->labels, _labels); /* the method's labels               */
  /* save the stack info               */
  this->maxStack = maxstack;
  this->vdictSize = variable_index;    /* save the initial vdict size       */
}


/**
 * Process a detached ::requires type call.
 *
 * @param activity The current activity,
 * @param routine  The routine object we're executing.
 * @param msgname  The name this was invoked under.
 * @param argPtr   The pointer to the call arguments,
 * @param argcount The count of arguments,
 * @param result   The returned result.
 */
void RexxCode::call(RexxActivity *activity, RoutineClass *routine, RexxString *msgname, RexxObject**argPtr, size_t argcount, ProtectedObject &result)
{
    // just forward to the more general method
    this->call(activity, routine, msgname, argPtr, argcount, OREF_SUBROUTINE, OREF_NULL, EXTERNALCALL, result);
}


void RexxCode::call(
    RexxActivity *activity,            /* activity running under            */
    RoutineClass *routine,             // top level routine instance
    RexxString *msgname,               /* message to be run                 */
    RexxObject**argPtr,                /* arguments to the method           */
    size_t      argcount,              /* the count of arguments            */
    RexxString *calltype,              /* COMMAND/ROUTINE/FUNCTION          */
    RexxString *environment,           /* initial command environment       */
    int   context,                     /* type of context                   */
    ProtectedObject &result)           // the method result
/******************************************************************************/
/* Function:  Call a method as a top level program or external function call  */
/******************************************************************************/
{
    // check the stack space before proceeding
    activity->checkStackSpace();       /* have enough stack space?          */
                                       /* add to the activity stack         */
    RexxActivation *newacta = ActivityManager::newActivation(activity, routine, this, calltype, environment, context);
    activity->pushStackFrame(newacta);
                /* run the method and return result  */
    newacta->run(OREF_NULL, msgname, argPtr, argcount, OREF_NULL, result);
}


void RexxCode::run(
    RexxActivity *activity,            /* activity running under            */
    RexxMethod *method,                // the method object getting invoked
    RexxObject *receiver,              /* object receiving the message      */
    RexxString *msgname,               /* message to be run                 */
    RexxObject**argPtr,                /* arguments to the method           */
    size_t      argcount,              /* the count of arguments            */
    ProtectedObject &result)           // the method result
/******************************************************************************/
/* Function:  Call a method as a top level program or external function call  */
/******************************************************************************/
{
    RexxActivation *newacta = ActivityManager::newActivation(activity, method, this);
                                       /* add to the activity stack         */
    activity->pushStackFrame(newacta);
                                       /* run the method and return result  */
    newacta->run(receiver, msgname, argPtr, argcount, OREF_NULL, result);
    activity->relinquish();            /* yield control now */
}


void RexxCode::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  memory_mark(this->source);
  memory_mark(this->start);
  memory_mark(this->labels);
}

void RexxCode::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  memory_mark_general(this->source);
  memory_mark_general(this->start);
  memory_mark_general(this->labels);
}

void RexxCode::flatten(RexxEnvelope * envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxCode)

   flatten_reference(newThis->source, envelope);
   flatten_reference(newThis->start, envelope);
   flatten_reference(newThis->labels, envelope);

  cleanUpFlatten
}

RexxArray *RexxCode::getSource()
/******************************************************************************/
/* Function:  Extract the source from a method from the source object as an   */
/*            array of strings.                                               */
/******************************************************************************/
{
  SourceLocation   location;           /* location information              */
  SourceLocation   end_location;       /* ending location                   */
  RexxInstruction *current;            /* current instruction               */

  if (this->start == OREF_NULL)        /* empty method?                     */
    return new_array((size_t)0);       /* just return an empty array        */
  location = start->getLocation();     /* get its location info             */
  current = this->start;               /* point to the beginning            */
                                       /* while not at the last one         */
  while (current->nextInstruction != OREF_NULL) {
    current = current->nextInstruction;/* step to the next one              */
  }

  end_location = current->getLocation(); /* get the end location              */
                                       /* copy over the ending position     */
  location.setEndLine(end_location.getEndLine());
  location.setEndOffset(end_location.getEndOffset());
                                       /* go extract the source array       */
  return this->source->extractSource(location);
}

RexxString * RexxCode::getProgramName()
/******************************************************************************/
/* Function:  Return the name of the program that contains this method.       */
/**REXX****************************************************************************/
{
                                       /* retrieve this from the source     */
  return this->source->getProgramName();
}


void * RexxCode::operator new(size_t size)
/******************************************************************************/
/* Function:  Create a new rexx method code instance                          */
/******************************************************************************/
{
    return new_object(size, T_RexxCode);        /* Get new object                    */
}


RexxObject *RexxCode::setSecurityManager(RexxObject *manager)
/******************************************************************************/
/* Function:  Associate a security manager with a method's source             */
/******************************************************************************/
{
    source->setSecurityManager(manager);
    return TheTrueObject;
}

