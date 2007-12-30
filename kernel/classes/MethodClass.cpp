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
/* REXX Kernel                                               MethodClass.c    */
/*                                                                            */
/* Primitive Method Class                                                     */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "RexxCode.hpp"
#include "RexxNativeCode.hpp"
#include "RexxActivity.hpp"
#include "RexxActivation.hpp"
#include "RexxNativeActivation.hpp"
#include "MethodClass.hpp"
#include "SourceFile.hpp"
#include "DirectoryClass.hpp"
#include "ProtectedObject.hpp"
#include <ctype.h>


// singleton class instance
RexxMethodClass *RexxMethod::classInstance = OREF_NULL;

RexxMethod::RexxMethod(BaseCode *codeObj)
/******************************************************************************/
/* Function:  Initialize a method object                                      */
/******************************************************************************/
{
    this->clearObject();                 /* start out fresh                   */
    this->methodFlags = 0;               /* clear all of the flags            */
    OrefSet(this, this->code, codeObj);  /* store the code                    */
}

void RexxMethod::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
    memory_mark(this->scope);
    memory_mark(this->code);
    memory_mark(this->objectVariables);
}

void RexxMethod::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
    memory_mark_general(this->scope);
    memory_mark_general(this->code);
    memory_mark_general(this->objectVariables);
}

void RexxMethod::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxMethod)

   flatten_reference(newThis->scope, envelope);
   flatten_reference(newThis->code, envelope);
   flatten_reference(newThis->objectVariables, envelope);

  cleanUpFlatten
}


void RexxMethod::run(
    RexxActivity *activity,            /* activity running under            */
    RexxObject *receiver,              /* object receiving the message      */
    RexxString *msgname,               /* message to be run                 */
    size_t count,                      /* count of arguments                */
    RexxObject **argPtr,               /* arguments to the method           */
    ProtectedObject &result)           // the returned result
/******************************************************************************/
/* Function:  Run a method on an object                                       */
/******************************************************************************/
{
    ProtectedObject p(this);           // belt-and-braces to make sure this is protected
    // just forward this to the code object
    code->run(activity, this, receiver, msgname, count, argPtr, result);
}


void RexxMethod::call(
    RexxActivity *activity,            /* activity running under            */
    RexxObject *receiver,              /* object receiving the message      */
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
    ProtectedObject p(this);           // belt-and-braces to make sure this is protected
    // just forward this to the code object
    code->call(activity, this, receiver, msgname, argPtr, argcount, calltype, environment, context, result);
}


RexxMethod *RexxMethod::newScope(
    RexxClass  *_scope)                 /* new method scope                  */
/******************************************************************************/
/* Function:  Create a new method with a given scope                          */
/******************************************************************************/
{
  RexxMethod *newMethod;               /* the copied method                 */

  if (this->scope == OREF_NULL) {      /* nothing set yet?                  */
    OrefSet(this, this->scope, _scope); /* just set it directly              */
    return this;                       /* and pass back unchanged           */
  }
  else {
                                       /* copy the method                   */
    newMethod= (RexxMethod *)this->copy();
                                       /* give the method the new scope     */
    OrefSet(newMethod, newMethod->scope, _scope);
    return newMethod;                  /* and return it                     */
  }
}

RexxArray  *RexxMethod::source()
/******************************************************************************/
/* Function:  Return an array of source strings that represent this method    */
/******************************************************************************/
{
    return code->getSource();
}

RexxObject *RexxMethod::setSecurityManager(
    RexxObject *manager)               /* supplied security manager         */
/******************************************************************************/
/* Function:  Associate a security manager with a method's source             */
/******************************************************************************/
{
    return code->setSecurityManager(manager);
}

void RexxMethod::setScope(
    RexxClass  *_scope)                /* scope for the method              */
/******************************************************************************/
/* Function:  Set a scope for a method without making a copy of the method    */
/*            object.                                                         */
/******************************************************************************/
{
  OrefSet(this, this->scope, _scope);  /* just set the scope                */
}

RexxObject *RexxMethod::setUnGuardedRexx()
/******************************************************************************/
/* Function:  Flag a method as being an unguarded method                      */
/******************************************************************************/
{
  this->setUnGuarded();                /* turn on the UNGUARDED state     */
  return OREF_NULL;                    /* return nothing                    */
}

RexxObject *RexxMethod::setGuardedRexx( )
/******************************************************************************/
/* Function:  Flag a method as being a guarded method (the default)           */
/******************************************************************************/
{
  this->setGuarded();                  /* flip back to the GUARDED state    */
  return OREF_NULL;                    /* return nothing                    */
}

RexxObject *RexxMethod::setPrivateRexx()
/******************************************************************************/
/* Function:  Flag a method as being a private method                         */
/******************************************************************************/
{
  this->setPrivate();                  /* turn on the PRIVATE flag          */
  return OREF_NULL;                    /* always return nothing             */
}

RexxObject *RexxMethod::setProtectedRexx()
/******************************************************************************/
/* Function:  Flag a method as being a private method                         */
/******************************************************************************/
{
  this->setProtected();                /* turn on the PROTECTED flag        */
  return OREF_NULL;                    /* always return nothing             */
}

/**
 * Return the Guarded setting for a method object.
 *
 * @return .true if the method is guarded.  .false otherwise.
 */
RexxObject *RexxMethod::isGuardedRexx( )
{
    return isGuarded() ? TheTrueObject : TheFalseObject;
}

/**
 * Return the Private setting for a method object.
 *
 * @return .true if the method is private.  .false otherwise.
 */
RexxObject *RexxMethod::isPrivateRexx( )
{
    return isPrivate() ? TheTrueObject : TheFalseObject;
}

/**
 * Return the Protected setting for a method object.
 *
 * @return .true if the method is protected.  .false otherwise.
 */
RexxObject *RexxMethod::isProtectedRexx( )
{
    return isProtected() ? TheTrueObject : TheFalseObject;
}

RexxSmartBuffer *RexxMethod::saveMethod()
/******************************************************************************/
/* Function: Flatten translated method into a buffer for storage into EA's etc*/
/******************************************************************************/
{
  RexxEnvelope *envelope;              /* envelope for flattening           */
                                       /* Get new envelope object           */
  envelope = new RexxEnvelope;
  ProtectedObject p(envelope);
                                       /* now pack up the envelope for      */
                                       /* saving.                           */
  envelope->pack(this);
  return envelope->getBuffer();        /* return the buffer                 */
}

void *RexxMethod::operator new (size_t size)
/******************************************************************************/
/* Function:  create a new method instance                                    */
/******************************************************************************/
{
                                         /* get a new method object           */
    RexxObject *newMethod = new_object(size);
                                         /* Give new object method behaviour  */
    newMethod->setBehaviour(TheMethodClass->getInstanceBehaviour());
    return newMethod;                    /* Initialize this new method        */
}


RexxMethod *RexxMethodClass::newRexxMethod(
    RexxSource *source,                /* source object for the method      */
    RexxClass  *scope)                 /* scope to use                      */
/******************************************************************************/
/* Function:  Convert a new source object to a method with the given scope    */
/******************************************************************************/
{
                                       /* create a new method object        */
  RexxMethod *newMethod = (RexxMethod *)source->method();
  if (scope != OREF_NULL)              /* given a scope too?                */
  {
      newMethod->setScope(scope);        /* set the scope                     */
  }
  return newMethod;                    /* return the new method object      */
}

RexxMethod *RexxMethodClass::newRexxCode(
    RexxString *pgmname,               /* name of the program               */
    RexxObject *source,                /* string or array source            */
    RexxObject *position,              /* argument position                 */
    RexxObject *option)
/******************************************************************************/
/* Function:  Create a new method from REXX code contained in a string or an  */
/*            array                                                           */
/******************************************************************************/
{
  RexxArray  *newSourceArray;          /* created source object             */
  RexxSource *newSource;               /* created source object             */
  RexxString *sourceString;
  RexxMethod *result = OREF_NULL;
  size_t counter;

                                       /* request an array version          */
  newSourceArray = source->requestArray();
                                       /* couldn't convert?                 */
  if (newSourceArray == (RexxArray *)TheNilObject)
  {
                                       /* get the string representation     */
    sourceString = source->makeString();
                                       /* got back .nil?                    */
    if (sourceString == (RexxString *)TheNilObject)
                                       /* raise an error                    */

      reportException(Error_Incorrect_method_no_method, position);
                                       /* wrap an array around the value    */
    newSourceArray = new_array(sourceString);
  }
  else                                 /* have an array, make sure all      */
  {
                                       /* is it single dimensional?         */
    if (newSourceArray->getDimension() != 1)
                                       /* raise an error                    */
      reportException(Error_Incorrect_method_noarray, position);
                                       /*  element are strings.             */
                                       /* Make a source array safe.         */
    ProtectedObject p(newSourceArray);
                                       /* Make sure all elements in array   */
    for (counter = 1; counter <= newSourceArray->size(); counter++) {
                                       /* Get element as string object      */
      sourceString = newSourceArray ->get(counter)->makeString();
                                       /* Did it convert?                   */
      if (sourceString == (RexxString *)TheNilObject) {
                                       /* and report the error.             */
        reportException(Error_Incorrect_method_nostring_inarray, IntegerTwo);
      }
      else {
                                       /* itsa string add to source array   */
        newSourceArray ->put(sourceString, counter);
      }
    }
  }
                                       /* create a source object            */
  newSource = new RexxSource (pgmname, newSourceArray);

  ProtectedObject p(newSource);
                                       /* now complete method creation      */
  if (option != OREF_NULL)
  {
    if (isOfClass(Method, option))
    {
      result = this->newRexxMethod(newSource, OREF_NULL);
      RexxCode *resultCode = (RexxCode *)result->getCode();
      BaseCode *code = result->getCode();
      if (isOfClass(RexxCode, code))
      {
          resultCode->setLocalRoutines(((RexxCode *) code)->getLocalRoutines());
          resultCode->setPublicRoutines(((RexxCode *) code)->getPublicRoutines());
      }
    }
    else
    {
      if (!isOfClass(String, option))
        reportException(Error_Incorrect_method_argType, IntegerThree, "Method/String object");
      else
      {
        // default given? set option to NULL (see code below)
        if (!stricmp("PROGRAMSCOPE",((RexxString*) option)->getStringData()))
          option = NULL;
        else
          reportException(Error_Incorrect_call_list, "NEW", IntegerThree, "\"PROGRAMSCOPE\", Method object", option);
      }
    }
  }
  // option NULL => set default: Program Scope.
  else if (option == NULL)
  {
    result = this->newRexxMethod(newSource, OREF_NULL);
    // new default: insert program scope into method object
    RexxCode *resultCode = (RexxCode *)result->getCode();
    resultCode->setLocalRoutines(ActivityManager::currentActivity->getCurrentActivation()->getSource()->getLocalRoutines());
    resultCode->setPublicRoutines(ActivityManager::currentActivity->getCurrentActivation()->getSource()->getPublicRoutines());
  }

  return result;
}


RexxMethod *RexxMethodClass::newRexx(
    RexxObject **init_args,            /* subclass init arguments           */
    size_t       argCount)             /* number of arguments passed        */
/******************************************************************************/
/* Function:  Create a new method from REXX code contained in a string or an  */
/*            array                                                           */
/******************************************************************************/
{
  RexxObject *pgmname;                 /* method name                       */
  RexxObject *source;                  /* Array or string object            */
  RexxMethod *newMethod;               /* newly created method object       */
  RexxObject *option = OREF_NULL;
  size_t initCount = 0;                /* count of arguments we pass along  */

                                       /* break up the arguments            */

  process_new_args(init_args, argCount, &init_args, &initCount, 2, (RexxObject **)&pgmname, (RexxObject **)&source);
                                       /* get the method name as a string   */
  RexxString *nameString = REQUIRED_STRING(pgmname, ARG_ONE);
  required_arg(source, TWO);           /* make sure we have the second too  */
  // retrieve extra parameter if exists
  if (initCount != 0)
    process_new_args(init_args, initCount, &init_args, &initCount, 1, (RexxObject **)&option, NULL);
                                       /* go create a method                */
  newMethod = this->newRexxCode(nameString, source, IntegerTwo, option);
  ProtectedObject p(newMethod);
                                       /* Give new object its behaviour     */
  newMethod->setBehaviour(this->getInstanceBehaviour());
   if (this->hasUninitDefined()) {        /* does object have an UNINT method  */
     newMethod->hasUninit();              /* Make sure everyone is notified.   */
   }
                                       /* now send an INIT message          */
  newMethod->sendMessage(OREF_INIT, init_args, initCount);
  return newMethod;                    /* return the new method             */
}


RexxMethod *RexxMethodClass::newFileRexx(
    RexxString *filename)              /* name of the target file           */
/******************************************************************************/
/* Function:  Create a method from a fully resolved file name                 */
/******************************************************************************/
{
  RexxSource *source;                  /* created source object             */

                                       /* get the method name as a string   */
  filename = REQUIRED_STRING(filename, ARG_ONE);
                                       /* create a source object            */
  source = ((RexxSource *)TheNilObject)->classNewFile(filename);
  ProtectedObject p(source);
                                       /* finish up processing of this      */
  RexxMethod * newMethod = this->newRexxMethod(source, (RexxClass *)TheNilObject);
  ProtectedObject p2(newMethod);
                                       /* Give new object its behaviour     */
  newMethod->setBehaviour(this->getInstanceBehaviour());
   if (this->hasUninitDefined()) {     /* does object have an UNINT method  */
     newMethod->hasUninit();           /* Make sure everyone is notified.   */
   }
                                       /* now send an INIT message          */
  newMethod->sendMessage(OREF_INIT);
  return newMethod;
}


RexxMethod *RexxMethodClass::newRexxBuffer(
      RexxString *pgmname,             /* file name to process              */
      RexxBuffer *source,              /* String or buffer with source      */
      RexxClass  *scope)               /* Scope for this method             */
/******************************************************************************/
/* Function:  Build a new method object from buffered REXX source             */
/******************************************************************************/
{
  RexxSource *newSource;               /* new source object                 */

  if (source == OREF_NULL)             /* didn't get source?                */
                                       /* raise an error                    */
    reportException(Error_Incorrect_method_noarg, IntegerTwo);
                                       /* create a source object            */
  newSource = (RexxSource *)((RexxSource *)TheNilObject)->classNewBuffered(pgmname, source);
  // we need to protect this source object until parsing is complete
  ProtectedObject p(newSource);
                                       /* now complete method creation      */
  RexxMethod *newMethod = this->newRexxMethod(newSource, scope);
  return newMethod;
}


RexxMethod *RexxMethodClass::newEntry(PNMF entry)
                        /* routine entry point               */
/******************************************************************************/
/* Function:  Create a native method from an entry point                      */
/******************************************************************************/
{
                                       /* get a new method object           */
    return new_method(new RexxNativeCode(entry));
}


RexxMethod *RexxMethodClass::restore(
    RexxBuffer *buffer,                /* buffer containing the method      */
    char *startPointer)                /* first character of the method     */
/******************************************************************************/
/* Function: Unflatten a translated method.  Passed a buffer object containing*/
/*           the method                                                       */
/******************************************************************************/
{
                                       /* Get new envelope object           */
  RexxEnvelope *envelope  = new RexxEnvelope;
  ProtectedObject p(envelope);
                                       /* now puff up the method object     */
  envelope->puff(buffer, startPointer);
                                       /* The receiver object is an envelope*/
                                       /* whose receiver is the actual      */
                                       /* method object we're restoring     */
  return (RexxMethod *)envelope->getReceiver();
}

RexxMethod *RexxMethodClass::newFile(
    RexxString *filename)              /* name of the target file           */
/******************************************************************************/
/* Function:  Create a method from a fully resolved file name                 */
/******************************************************************************/
{
                                       /* create a source object            */
  RexxSource *source = ((RexxSource *)TheNilObject)->classNewFile(filename);
  ProtectedObject p(source);
                                       /* finish up processing of this      */
  return this->newRexxMethod(source, (RexxClass *)TheNilObject);
}



void BaseCode::run(RexxActivity *activity, RexxMethod *method, RexxObject *receiver, RexxString *msgname, size_t argCount, RexxObject **arguments, ProtectedObject &result)
{
    // this is a NOP for the base
}


void BaseCode::call(RexxActivity *activity, RexxMethod *method, RexxObject *receiver,  RexxString *msgname,
   RexxObject **arguments, size_t argcount, RexxString *ct, RexxString *env, int context, ProtectedObject &result)
{
    run(activity, method, receiver, msgname, argcount, arguments, result);
}


RexxArray *BaseCode::getSource()
{
                                       /* this is always a null array       */
    return (RexxArray *)TheNullArray->copy();
}


RexxObject *BaseCode::setSecurityManager(RexxObject *manager)
/******************************************************************************/
/* Function:  Associate a security manager with a method's source             */
/******************************************************************************/
{
    // the default is just to return a failure
    return TheFalseObject;
}
