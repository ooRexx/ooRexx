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
/* REXX Kernel                                                  MethodClass.c    */
/*                                                                            */
/* Primitive Method Class                                                     */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "RexxCode.hpp"
#include "RexxNativeMethod.hpp"
#include "RexxActivity.hpp"
#include "RexxActivation.hpp"
#include "RexxNativeActivation.hpp"
#include "MethodClass.hpp"
#include "SourceFile.hpp"
#include "DirectoryClass.hpp"
#include <ctype.h>

extern PCPPM ExportedMethods[];        /* table of exported methods         */

RexxMethod::RexxMethod(
    size_t method,                     /* method table index                */
    PCPPM entry,                       /* method entry point                */
    size_t argCount,                   /* arguments number/type             */
    RexxInternalObject *codeObj)       /* associated method code            */
/******************************************************************************/
/* Function:  Initialize a method object                                      */
/******************************************************************************/
{

  this->clearObject();                 /* start out fresh                   */
  this->methodFlags = 0;               /* clear all of the flags            */
  this->setMethodIndex(method);        /* save the method code number       */
  this->cppEntry = entry;              /* set the entry point               */
  this->setArgumentCount(argCount);    /* and the arguments                 */
                                       /* get the argument information      */
  OrefSet(this, this->code, codeObj);  /* store the code                    */
  if (code != OREF_NULL) {             /* have some sort of code?           */
    if (isOfClass(RexxCode, code))         /* written in REXX?                  */
      this->setRexxMethod();           /* turn on the REXX flag             */
    else
      this->setNativeMethod();    ;    /* this is a native method           */
  }
}

void RexxMethod::live()
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  setUpMemoryMark
  memory_mark(this->scope);
  memory_mark(this->code);
  memory_mark(this->objectVariables);
  memory_mark(this->attribute);
  cleanUpMemoryMark
}

void RexxMethod::liveGeneral()
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  setUpMemoryMarkGeneral
  memory_mark_general(this->scope);
  memory_mark_general(this->code);
  memory_mark_general(this->objectVariables);
  memory_mark_general(this->attribute);
  cleanUpMemoryMarkGeneral
                                       /* part of the saved image?          */
  if (memoryObject.restoringImage()) { /* restoring the image?              */
    this->setInternal();               /* mark as an image method           */
                                       /* reset the method entry point      */
    this->cppEntry = ExportedMethods[this->getMethodIndex()];
  }
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
   flatten_reference(newThis->attribute, envelope);

  cleanUpFlatten
}

RexxObject * RexxMethod::unflatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  unflatten an object                                             */
/******************************************************************************/
{
                                       /* Does the entry have a value?      */
                                       /* if not then we haven't unflattened*/
  if (this->code == OREF_NULL)         /* is this a kernel method?          */
                                       /* reset the method entry point      */
    this->cppEntry = ExportedMethods[this->getMethodIndex()];
  return (RexxObject *)this;           /* return ourself.                   */
}

RexxObject *RexxMethod::run(
    RexxActivity *activity,            /* activity running under            */
    RexxObject *receiver,              /* object receiving the message      */
    RexxString *msgname,               /* message to be run                 */
    size_t count,                      /* count of arguments                */
    RexxObject **argPtr)               /* arguments to the method           */
/******************************************************************************/
/* Function:  Run a method on an object                                       */
/******************************************************************************/
{
  RexxObject*  result = OREF_NULL;     /* result of the activation run      */
  size_t i;                            /* loop counter                      */
  RexxObject * argument_list[7];       /* arguments removed from the array  */
  RexxActivation *newacta;             /* newly created activation          */
  RexxNativeActivation *newNActa;      /* newly created Native activation   */
  PCPPM methodEntry;                   /* kernel method entry point         */
  RexxActivation * Parent;

  if (this->code == OREF_NULL) {       /* directly to a kernel method?      */
    methodEntry = this->cppEntry;      /* get the entry point               */
                                       /* expecting an array?               */
                                       /* expecting a pointer/count pair?   */
    if (this->getArgumentCount() == A_COUNT) {
                                       /* we can pass this right on         */
      result = (receiver->*((PCPPMC1)methodEntry))(argPtr, count);
    }
    else {                             /* receiver expects fixed arguments  */
      if (count > this->getArgumentCount()) /* too many arguments?               */
        reportException(Error_Incorrect_method_maxarg, this->getArgumentCount());
      if (count < this->getArgumentCount()) { /* need to pad these out?            */
        for (i = 0; i < count; i++)    /* copy over the arguments so we     */
                                       /* don't clobber things in the caller*/
          argument_list[i] = argPtr[i];
                                       /* null out any missing arguments    */
        for (i = count; i < this->getArgumentCount(); i++)
          argument_list[i] = OREF_NULL;
        argPtr = &argument_list[0];    /* point at the new argument list    */
      }

      switch (this->getArgumentCount()) { /* switch based on number of args    */

        case 0:                        /* zero                              */
          result = (receiver->*((PCPPM0)methodEntry))();
          break;

        case 1:
          result = (receiver->*((PCPPM1)methodEntry))(argPtr[0]);
          break;

        case 2:
          result = (receiver->*((PCPPM2)methodEntry))(argPtr[0], argPtr[1]);
          break;

        case 3:
          result = (receiver->*((PCPPM3)methodEntry))(argPtr[0], argPtr[1], argPtr[2]);
          break;

        case 4:
          result = (receiver->*((PCPPM4)methodEntry))(argPtr[0], argPtr[1], argPtr[2], argPtr[3]);
          break;

        case 5:
          result = (receiver->*((PCPPM5)methodEntry))(argPtr[0], argPtr[1], argPtr[2],
              argPtr[3], argPtr[4]);
          break;

        case 6:
          result = (receiver->*((PCPPM6)methodEntry))(argPtr[0], argPtr[1], argPtr[2],
              argPtr[3], argPtr[4], argPtr[5]);
          break;

        case 7:
          result = (receiver->*((PCPPM7)methodEntry))(argPtr[0], argPtr[1], argPtr[2],
              argPtr[3], argPtr[4], argPtr[5], argPtr[6]);
          break;

        default:
          logic_error("too many args for kernel kmethod");
          break;
      }
    }
    return result;                     /* and return the result             */
  }
  else if (this->isRexxMethod()) {     /* written in REXX?                  */

    newacta = TheActivityClass->newActivation(receiver, this, activity, msgname, (RexxActivation *)TheNilObject, METHODCALL);
                                       /* add to the activity stack         */

    activity->push(newacta);

    /* moved before new argarray because otherwise argarray might be collected
       while dbg exit is processed (because of SendMessage in workbench)
       Another possibility would be to save and later discard argarray */
    Parent = newacta->sender;
    Parent->dbgEnterSubroutine();
    newacta->dbgPrepareMethod(Parent);

                                       /* run the method and return result  */
    result = newacta->run(argPtr, count, OREF_NULL);
    if (Parent) Parent->dbgLeaveSubroutine();
    newacta->dbgPassTrace2Parent(Parent);
    CurrentActivity->yield(NULL);    /* yield control now */ /* NULL instead of result */
      /* yield stores the argument but result is already saved in run so we don't need to save again */
    if (result != OREF_NULL) discard(result);
    return result;                     /* and return it                     */
  }
  else {                               /* native activation                 */
                                       /* create a new native activation    */
    newNActa = new (receiver, this, activity, msgname, (RexxActivation *)TheNilObject) RexxNativeActivation;
    activity->push(newNActa);          /* push it on the activity stack     */
                                       /* and go run it                     */
    return newNActa->run(count, argPtr);
  }
}

RexxObject *RexxMethod::call(
    RexxActivity *activity,            /* activity running under            */
    RexxObject *receiver,              /* object receiving the message      */
    RexxString *msgname,               /* message to be run                 */
    RexxObject**argPtr,                /* arguments to the method           */
    size_t      argcount,              /* the count of arguments            */
    RexxString *calltype,              /* COMMAND/ROUTINE/FUNCTION          */
    RexxString *environment,           /* initial command environment       */
    int   context)                     /* type of context                   */
/******************************************************************************/
/* Function:  Call a method as a top level program or external function call  */
/******************************************************************************/
{
  RexxActivation *newacta;             /* newly created activation          */
  RexxObject * returnObject;
  RexxActivation * Parent;
  static int rnd = 0;

  CurrentActivity->stackSpace();       /* have enough stack space?          */
  if (this->isRexxMethod()) {          /* this written in REXX?             */
    hold(this);                        /* keep it around                    */
                                       /* add to the activity stack         */
    newacta = TheActivityClass->newActivation(receiver, this, activity, msgname, (RexxActivation *)TheNilObject, context);

    activity->push(newacta);


    if (calltype != OREF_NULL)         /* have a a calltype override?       */
                                       /* set the call type information     */
      newacta->setCallType(calltype);
    if (environment != OREF_NULL)      /* have an default environment?      */
                                       /* set it also                       */
      newacta->setDefaultAddress(environment);

    Parent = newacta->sender;
    Parent->dbgEnterSubroutine();
    newacta->dbgPrepareMethod(Parent);
    // the random seed is copied from the calling activity, this led
    // to reproducable random sequences even though no specific seed was given!
    // see feat. 900 for example program.
    newacta->random_seed += (++rnd);
                /* run the method and return result  */
    returnObject = newacta->run(argPtr, argcount, OREF_NULL);
    if (Parent) Parent->dbgLeaveSubroutine();
    newacta->dbgPassTrace2Parent(Parent);
    return returnObject;

  }
  else                                 /* kernel/native method              */
                                       /* pass on the call                  */
    return this->run(activity, receiver, msgname, argcount, argPtr);
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
  if (this->isRexxMethod())            /* this written in REXX?             */
                                       /* return associated source          */
    return this->rexxCode->sourceRexx();
  else                                 /* kernel/native code                */
                                       /* this is always a null array       */
    return (RexxArray *)TheNullArray->copy();
}

RexxObject *RexxMethod::setSecurityManager(
    RexxObject *manager)               /* supplied security manager         */
/******************************************************************************/
/* Function:  Associate a security manager with a method's source             */
/******************************************************************************/
{
  if (this->isRexxMethod()) {          /* this written in REXX?             */
                                       /* return associated source          */
    this->getSource()->setSecurityManager(manager);
    return TheTrueObject;              /* this worked ok                    */
  }
  else                                 /* kernel/native code                */
    return TheFalseObject;             /* nothing to set security on        */
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
  RexxSmartBuffer   *envelopeBuffer;   /* enclosing buffer                  */

                                       /* Get new envelope object           */
  envelope = new RexxEnvelope;
  save(envelope);
                                       /* now pack up the envelope for      */
                                       /* saving.                           */
  envelope->pack(this);
                                       /* pull out the buffer               */
  envelopeBuffer = envelope->getBuffer();
  discard_hold(envelope);              /* release memory lock on envelope   */
  return envelopeBuffer;               /* return the buffer                 */
}

void *RexxMethod::operator new (size_t size)
/******************************************************************************/
/* Function:  create a new method instance                                    */
/******************************************************************************/
{
  RexxObject * newMethod;              /* newly created method              */

                                       /* get a new method object           */
  newMethod = new_object(size);
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
  RexxMethod *newMethod;               /* newly created method              */

                                       /* create a new method object        */
  newMethod = (RexxMethod *)source->method();
  if (scope != OREF_NULL)              /* given a scope too?                */
    newMethod->setScope(scope);        /* set the scope                     */
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
  if (newSourceArray == (RexxArray *)TheNilObject) {
                                       /* get the string representation     */
    sourceString = source->makeString();
                                       /* got back .nil?                    */
    if (sourceString == (RexxString *)TheNilObject)
                                       /* raise an error                    */

      reportException(Error_Incorrect_method_no_method, position);
                                       /* wrap an array around the value    */
    newSourceArray = new_array(sourceString);
  }
  else {                               /* have an array, make sure all      */
                                       /* is it single dimensional?         */
    if (newSourceArray->getDimension() != 1)
                                       /* raise an error                    */
      reportException(Error_Incorrect_method_noarray, position);
                                       /*  element are strings.             */
                                       /* Make a source array safe.         */
    save(newSourceArray);
                                       /* Make sure all elements in array   */
    for (counter = 1; counter <= newSourceArray->size(); counter++) {
                                       /* Get element as string object      */
      sourceString = newSourceArray ->get(counter)->makeString();
                                       /* Did it convert?                   */
      if (sourceString == (RexxString *)TheNilObject) {
                                       /* nope, release source array.       */
        discard(newSourceArray);
                                       /* and report the error.             */
        reportException(Error_Incorrect_method_nostring_inarray, IntegerTwo);
      }
      else {
                                       /* itsa string add to source array   */
        newSourceArray ->put(sourceString, counter);
      }
    }
    discard_hold(newSourceArray);      /* release newSOurce obj.            */
  }
                                       /* create a source object            */
  newSource = new RexxSource (pgmname, newSourceArray);
                                       /* now complete method creation      */
  save(newSource);                     /* needed because newRexxMethod calls method() which discards this */
//  return this->newRexxMethod(newSource, OREF_NULL);
  if (option != OREF_NULL) {
    if (isOfClass(Method, option)) {
      result = this->newRexxMethod(newSource, OREF_NULL);
      result->setLocalRoutines(((RexxMethod*) option)->getLocalRoutines());
      result->setPublicRoutines(((RexxMethod*) option)->getPublicRoutines());
    } else {
      if (!isOfClass(String, option))
        reportException(Error_Incorrect_method_argType, IntegerThree, "Method/String object");
      else {
        // default given? set option to NULL (see code below)
        if (!stricmp("PROGRAMSCOPE",((RexxString*) option)->getStringData()))
          option = NULL;
        else
          reportException(Error_Incorrect_call_list, "NEW", IntegerThree, "\"PROGRAMSCOPE\", Method object", option);
      }
    }
  }
  // option NULL => set default: Program Scope.
  else if (option == NULL) {
    result = this->newRexxMethod(newSource, OREF_NULL);
    // new default: insert program scope into method object
    result->setLocalRoutines(CurrentActivity->currentActivation->getSource()->getLocalRoutines());
    result->setPublicRoutines(CurrentActivity->currentActivation->getSource()->getPublicRoutines());
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
  save(newMethod);
                                       /* Give new object its behaviour     */
  newMethod->setBehaviour(this->getInstanceBehaviour());
   if (this->hasUninitDefined()) {        /* does object have an UNINT method  */
     newMethod->hasUninit();              /* Make sure everyone is notified.   */
   }
                                       /* now send an INIT message          */
  newMethod->sendMessage(OREF_INIT, init_args, initCount);
  discard_hold(newMethod);
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
  save(source);
                                       /* finish up processing of this      */
  RexxMethod * newMethod = this->newRexxMethod(source, (RexxClass *)TheNilObject);
  save(newMethod);
  discard_hold(source);
                                       /* Give new object its behaviour     */
  newMethod->setBehaviour(this->getInstanceBehaviour());
   if (this->hasUninitDefined()) {     /* does object have an UNINT method  */
     newMethod->hasUninit();           /* Make sure everyone is notified.   */
   }
                                       /* now send an INIT message          */
  newMethod->sendMessage(OREF_INIT);
  discard_hold(newMethod);
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
  save(newSource);
                                       /* now complete method creation      */
  RexxMethod *newMethod = this->newRexxMethod(newSource, scope);
  // end of the protected section
  discard_hold(newSource);
  return newMethod;
}

RexxMethod *RexxMethodClass::newNative(
       RexxString *procedure,          /* procedure to load                 */
       RexxString *libName,            /* library to load from              */
       RexxClass  *scope)              /* variable scope information        */
/******************************************************************************/
/* Function:  Create a new native method with the given procedure, library    */
/*            and scope                                                       */
/******************************************************************************/
{
  RexxMethod *newMethod;               /* newly created method              */
  RexxNativeCode *newCode;             /* associated REXX code object       */

                                       /* create a new code object          */
  newCode = new RexxNativeCode (procedure, libName, NULL, 0);
                                       /* get a new method object           */
  newMethod = new_method(0, (PCPPM)NULL, 0, (RexxInternalObject *)newCode);
  if (scope != OREF_NULL)              /* given a scope too?                */
    newMethod->setScope(scope);        /* set the associated scope          */
  return newMethod;
}


RexxMethod *RexxMethodClass::newEntry( PFN entry)
                        /* routine entry point               */
/******************************************************************************/
/* Function:  Create a native method from an entry point                      */
/******************************************************************************/
{
  RexxMethod *newMethod;               /* newly created method              */
  RexxNativeCode *newCode;             /* associated REXX code object       */

                                       /* create a new code object          */
  newCode = new RexxNativeCode (OREF_NULL, OREF_NULL, entry, 0);
                                       /* get a new method object           */
  newMethod =new_method(0, (PCPPM)NULL, 0, (RexxInternalObject *)newCode);
  return newMethod;
}


RexxMethod *RexxMethodClass::restore(
    RexxBuffer *buffer,                /* buffer containing the method      */
    char *startPointer)                /* first character of the method     */
/******************************************************************************/
/* Function: Unflatten a translated method.  Passed a buffer object containing*/
/*           the method                                                       */
/******************************************************************************/
{

  RexxEnvelope *envelope;              /* containing envelope               */

                                       /* Get new envelope object           */
  envelope  = new_envelope();
  save(envelope);
                                       /* now puff up the method object     */
  envelope->puff(buffer, startPointer);
  discard_hold(envelope);              /* release the envelope now          */
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
  RexxSource *source;                  /* created source object             */

                                       /* create a source object            */
  source = ((RexxSource *)TheNilObject)->classNewFile(filename);
  save(source);
                                       /* finish up processing of this      */
  RexxMethod * newMethod = this->newRexxMethod(source, (RexxClass *)TheNilObject);
  discard_hold(source);
  return newMethod;
}

