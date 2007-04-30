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
/****************************************************************************/
/* REXX Kernel                                                  RexxVariable.c     */
/*                                                                          */
/* Primitive Variable Class                                                 */
/*                                                                          */
/****************************************************************************/
#include "RexxCore.h"
#include "RexxVariable.hpp"
#include "RexxActivity.hpp"

void RexxVariable::live()
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  setUpMemoryMark
  memory_mark(this->variableValue);
  memory_mark(this->variable_name);
  memory_mark(this->dependents);
  cleanUpMemoryMark
}

void RexxVariable::liveGeneral()
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  setUpMemoryMarkGeneral
  memory_mark_general(this->variableValue);
  memory_mark_general(this->variable_name);
  memory_mark_general(this->dependents);
  cleanUpMemoryMarkGeneral
}

void RexxVariable::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxVariable)

   flatten_reference(newThis->variableValue, envelope);
   flatten_reference(newThis->variable_name, envelope);
   flatten_reference(newThis->dependents, envelope);

  cleanUpFlatten
}

void RexxVariable::inform(
     RexxActivity *informee)           /* activity to inform of changes     */
/****************************************************************************/
/* Function:  Set up an activity notification for a variable change         */
/****************************************************************************/
{
  if (this->dependents == OREF_NULL)   /* no dependents yet?                */
                                       /* set this up as an object table    */
    OrefSet(this, this->dependents, new_object_table());
                                       /* add this to the table             */
  this->dependents->put(TheNilObject, (RexxObject *)informee);
}

void RexxVariable::uninform(
     RexxActivity *informee)           /* activity to inform of changes     */
/****************************************************************************/
/* Function:  Remove a dependent from the notification list                 */
/****************************************************************************/
{
                                       /* remove the entry                  */
  this->dependents->remove((RexxObject *)informee);
  if (this->dependents->items() == 0)  /* last one?                         */
                                       /* drop the dependents list          */
    OrefSet(this, this->dependents, OREF_NULL);
}

void RexxVariable::drop()
/****************************************************************************/
/* Function:  Drop a variable                                               */
/****************************************************************************/
{
                                       /* clear out the value               */
  OrefSet(this, this->variableValue, OREF_NULL);
  if (this->dependents != OREF_NULL)   /* have notifications to process?    */
    this->notify();                    /* notify any dependents             */
}

void RexxVariable::notify()
/****************************************************************************/
/* Function:  Process all variable notifications                            */
/****************************************************************************/
{
  LONG           i;                    /* loop variable                     */
  RexxActivity *activity;              /* current activity                  */

  if (this->dependents != OREF_NULL) { /* any dependents?                   */
                                       /* loop through the table            */
    for (i = this->dependents->first(); this->dependents->available(i); i = this->dependents->next(i)) {
                                       /* post the event to the dependent   */
      ((RexxActivity *)this->dependents->index(i))->guardPost();
    }
                                       /* yield control and allow the       */
                                       /* waiting guard to run too          */
    activity = CurrentActivity;        /* get the current activity          */
    ReleaseKernelAccess(activity);     /* release the lock                  */
    RequestKernelAccess(activity);     /* get it back again                 */
  }
}


RexxVariable *RexxMemory::newVariable(
    RexxString *name)                  /* the name of the variable          */
/****************************************************************************/
/* Function:  Create a new REXX variable object                             */
/****************************************************************************/
{
  RexxVariable *newObject;             /* created variable object           */

                                       /* Get new object                    */
  newObject = (RexxVariable *)new_object(sizeof(RexxVariable));
                                       /* Give new object its behaviour     */
  BehaviourSet(newObject, TheVariableBehaviour);
                                       /* set the virtual function table    */
  setVirtualFunctions(newObject, T_variable);
  newObject->variableValue = OREF_NULL;/* clear out the hash value          */
  newObject->creator = OREF_NULL;      /* clear out creator field           */
  newObject->variable_name = name;     /* fill in the name                  */
  newObject->dependents = OREF_NULL;   /* and the dependents                */
  return newObject;                    /* return the new object             */
}

