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
/* REXX Kernel                                         RexxNativeMethod.c     */
/*                                                                            */
/* Primitive Method Class                                                     */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "DirectoryClass.hpp"
#include "LibraryManager.hpp"
#include "RexxNativeActivation.hpp"
#include <ctype.h>



RexxNativeCode::RexxNativeCode(
    PNMF        _entry)                /* Entry point address for method    */
/****************************************************************************/
/* Function:  Initialize a REXX native code object                          */
/****************************************************************************/
{
    // save the entry point
    this->entry = _entry;
    // no procedure information here.
    this->index = 0;
    OrefSet(this, this->library, OREF_NULL);
    OrefSet(this, this->procedure, OREF_NULL);
}


RexxNativeCode::RexxNativeCode(
    PNMF        _entry,                /* Entry point address for method    */
     size_t      _index )               /* internal method index             */
/****************************************************************************/
/* Function:  Initialize a REXX native code object                          */
/****************************************************************************/
{
    // save the index and entry point
    this->index = _index;
    this->entry = _entry;
    // no procedure information here.
    OrefSet(this, this->library, OREF_NULL);
    OrefSet(this, this->procedure, OREF_NULL);
}


RexxNativeCode::RexxNativeCode(
     RexxString *_procedure,            /* procedure to load                 */
     RexxString *_library,              /* library to load from              */
     PNMF        _entry)                /* Entry point address for method    */
/****************************************************************************/
/* Function:  Initialize a REXX native code object                          */
/****************************************************************************/
{
  this->entry = _entry;                 /* no resolved entry point yet       */
  this->index = 0;                      // no index value
                                       /* save the library name             */
  OrefSet(this, this->library, _library);
                                       /* save the procedure name           */
  OrefSet(this, this->procedure, _procedure);
}

void RexxNativeCode::reinit(           /* reinitialize the nmethod entry    */
     RexxPointer *handle )             /* library handle information        */
/****************************************************************************/
/* Function:  Reinitialize a REXX native method                             */
/****************************************************************************/
{
    if (this->procedure != OREF_NULL)    /* in another library?               */
    {
                                           /* and resolve the function address  */
        this->entry = (PNMF)SysLoadProcedure(handle, this->procedure);
    }
    else
    {
        // resolve the internal version
        this->entry = LibraryManager::resolveInternalMethod(index);
    }
}


void RexxNativeCode::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  memory_mark(this->library);
  memory_mark(this->procedure);
}


void RexxNativeCode::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  memory_mark_general(this->library);
  memory_mark_general(this->procedure);

  // if we're restoring the image, we need to fix up the entry point for any
  // internal methods.
  if (memoryObject.restoringImage())
  {
      if (this->procedure == OREF_NULL)
      {
          // resolve the internal version
          this->entry = LibraryManager::resolveInternalMethod(index);
      }
  }
}

void RexxNativeCode::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxNativeCode)

   flatten_reference(newThis->library, envelope);
   flatten_reference(newThis->procedure, envelope);
                                       /* Set entry to NUll for 2 reasons   */
                                       /* 1st force branch to 0 is not      */
                                       /*restored, 2 used to indicated if   */
                                       /* the method has bee unflattened    */
   newThis->entry = NULL;
  cleanUpFlatten
}

RexxObject * RexxNativeCode::unflatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  unflatten an object                                             */
/******************************************************************************/
{
   if (this->entry == NULL)
   {
       if (procedure == OREF_NULL)
       {
           // resolve the internal version
           this->entry = LibraryManager::resolveInternalMethod(index);
       }
       else
       {
           // resolve the internal version
           this->entry = LibraryManager::resolveExternalMethod(library, procedure);
       }
   }
   return (RexxObject *)this;          /* return ourself.                   */
}


/**
 * Run a method call (vs a straight program call).
 *
 * @param activity The current activity.
 * @param method   The method we're attached to.
 * @param receiver The method receiver object (the "self" object).
 * @param messageName
 *                 The name of the message used to invoke the method.
 * @param count    The count of arguments.
 * @param argPtr   The pointer to the arguments.
 * @param result   The protected object used to return the result.
 */
void RexxNativeCode::run(RexxActivity *activity, RexxMethod *method, RexxObject *receiver, RexxString *messageName,
    size_t count, RexxObject **argPtr, ProtectedObject &result)
{
    // create a new native activation
    RexxNativeActivation *newNActa = new RexxNativeActivation(activity, method, this);
    activity->pushStackFrame(newNActa);   /* push it on the activity stack     */
                                       /* and go run it                     */
    newNActa->run(receiver, messageName, count, argPtr, result);
}


void * RexxNativeCode::operator new(
     size_t      size)                 /* object size                       */
/****************************************************************************/
/* Function:  Create a new Native method                                    */
/****************************************************************************/
{
  RexxObject *newMethod;               /* new object                        */

  newMethod = new_object(size);        /* Get new object                    */
                                       /* Give new object its behaviour     */
  newMethod->setBehaviour(TheNativeCodeBehaviour);
  return newMethod;                    /* and return the new method         */
}
