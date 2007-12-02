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
/* REXX Kernel                                               RexxActivity     */
/*                                                                            */
/* Primitive Activity Class                                                   */
/*                                                                            */
/* NOTE:  activities are an execution time only object.  They are never       */
/*        flattened or saved in the image, and hence will never be in old     */
/*        space.  Because of this, activities "cheat" and do not use          */
/*        OrefSet to assign values to get better performance.  Care must be   */
/*        used to maintain this situation.                                    */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include <ctype.h>
#if defined(OPSYS_SUN)
#include <sched.h>
#endif
#include "RexxCore.h"
#include "StringClass.hpp"
#include "StackClass.hpp"
#include "RexxActivity.hpp"
#include "RexxActivation.hpp"
#include "RexxNativeActivation.hpp"
#include "MessageClass.hpp"
#include "ArrayClass.hpp"
#include "TableClass.hpp"
#include "DirectoryClass.hpp"
#include "SourceFile.hpp"
#include "RexxVariableDictionary.hpp"
#include "RexxCode.hpp"
#include "RexxInstruction.hpp"
#include "RexxMemory.hpp"
#include "RexxVariableDictionary.hpp"
#include "ProtectedObject.hpp"
#define INCL_RXSYSEXIT
#define INCL_RXOBJECT
#include SYSREXXSAA

const size_t ACT_STACK_SIZE = 10;

extern SMTX rexx_resource_semaphore;   /* global kernel semaphore           */
extern SMTX rexx_start_semaphore;      /* startup semaphore                 */

extern "C" void activity_thread (RexxActivity *objp);

void activity_thread (
  RexxActivity *objp)                  /* activity assciated with a thread  */
/******************************************************************************/
/* Function:  Main "entry" routine for a newly created thread                 */
/*                                                                            */
/* Remark:    For AIX the pthread_exit is called whenever a thread terminates.*/
/*            This should clear storage.                                      */
/*                                                                            */
/******************************************************************************/
{
    objp->runThread();
}


/**
 * The main entry point for spawned activities.
 */
void RexxActivity::runThread()
{
    SYSEXCEPTIONBLOCK exreg;             /* system specific exception info    */

    SysInitializeThread();               /* system specific thread init       */
                                         /* establish the stack base pointer  */
    this->nestedInfo.stackptr = SysGetThreadStackBase(TOTAL_STACK_SIZE);
    SysRegisterExceptions(&exreg);       /* create needed exception handlers  */
    for (;;)
    {
        try
        {
            EVWAIT(this->runsem);            /* wait for run permission           */
            if (this->exit)                  /* told to exit?                     */
            {
                break;                       /* we're out of here                 */
            }
            /* set our priority appropriately    */
#ifdef THREADHANDLE
            SysSetThreadPriority(this->threadid, this->hThread, this->priority);
#else
            SysSetThreadPriority(this->threadid, this->priority);
#endif

            this->requestAccess();           /* now get the kernel lock           */
                                             /* get the top activation            */
            this->topActivation->dispatch(); /* go dispatch it                    */

        }
        catch (ActivityException)    // we've had a termination event, raise an error
        {
            this->error(0);
        }


        memoryObject.runUninits();         /* run any needed UNINIT methods now */

        EVSET(this->runsem);               /* reset the run semaphore and the   */
        EVSET(this->guardsem);             /* guard semaphore                   */

        // try to pool this.  If the ActivityManager doesn't take, we go into termination mode
        if (!ActivityManager::poolActivity(this))
        {
            this->releaseAccess();
            break;
        }
        // release the kernel lock and go wait for more work
        this->releaseAccess();
    }

    this->requestAccess();               /* get the kernel access             */

    SysDeregisterExceptions(&exreg);     /* remove exception trapping         */
    // tell the activity manager we're going away
    ActivityManager::activityEnded(this);
    SysTerminateThread((TID)this->threadid);  /* system specific thread termination*/
    return;                              /* finished                          */
}


/**
 * Do cleanup of activity resources when an activity is completely
 * shutdown and discarded.
 */
void RexxActivity::terminateActivity()
{
    EVCLOSE(this->runsem);
    EVCLOSE(this->guardsem);
#ifdef THREADHANDLE
    EVCLOSE(this->hThread);
#endif
}



void *RexxActivity::operator new(size_t size)
/******************************************************************************/
/* Function:  Create a new activity                                           */
/******************************************************************************/
{
   RexxActivity  *newActivity;

                                       /* get the new activity storage      */
   newActivity  = (RexxActivity *)new_object(size);
                                       /* Give new object its behaviour     */
   newActivity->setBehaviour(TheActivityBehaviour);
   return newActivity;                 /* and return it                     */
}

RexxActivity::RexxActivity(
    bool recycle,                      /* activity is being reused          */
    int  _priority)                    /* activity priority                 */
/******************************************************************************/
/* Function:  Initialize an activity object                                   */
/*  Returned:  Nothing                                                        */
/******************************************************************************/
{
  if (!recycle) {                      /* if this is the first time         */
    this->clearObject();               /* globally clear the object         */
                                       /* create an activation stack        */
    this->activations = new_internalstack(ACT_STACK_SIZE);
    this->frameStack.init();           /* initialize the frame stack        */
    EVCR(this->runsem);                /* create the run and                */
    EVCR(this->guardsem);              /* guard semaphores                  */
    this->size = ACT_STACK_SIZE;       /* set the activation stack size     */
    this->stackcheck = true;           /* start with stack checking enabled */
                                       /* use default settings set          */
    this->exitObjects = false;         // ENG023A: behaviour of exits: classic REXX
                                       /* set up current usage set          */
    this->numericSettings = Numerics::getDefaultSettings();

    if (_priority != NO_THREAD) {      /* need to create a thread?          */
#ifdef FIXEDTIMERS
          /* start the control thread the first time a concurrent thread is used */
      SysStartTimeSlice();   /* Start a new timeSlice                       */
#endif

      EVSET(this->runsem);             /* set the run semaphore             */
                                       /* create a thread                   */
      this->threadid = SysCreateThread((PTHREADFN)activity_thread,C_STACK_SIZE,this);
      this->priority = _priority;      /* and the priority                  */
    }
    else {                             /* thread already exists             */
                                       /* query the thread id               */
      this->threadid = SysQueryThreadID();
#ifdef THREADHANDLE
      this->hThread = SysQueryThread();
#endif
      this->priority = MEDIUM_PRIORITY;/* switch to medium priority         */
                                       /* establish the stack base pointer  */
      this->nestedInfo.stackptr = SysGetThreadStackBase(TOTAL_STACK_SIZE);
    }
    this->generateRandomNumberSeed();  /* get a fresh random seed           */
                                       /* Create table for progream being   */
    this->requiresTable = new_table(); /*installed vial ::REQUIRES          */
                                       /* clear out the top activation      */
    this->topActivation = (RexxActivationBase *)TheNilObject;
                                       /* and the current REXX activation   */
    this->currentActivation = (RexxActivation *)TheNilObject;

  }                                    /* recycling                         */
  else {
    this->priority = _priority;        /* just set the priority             */
                                       /* Make sure any left over           */
                                       /* ::REQUIRES is cleared out.        */
    this->resetRunningRequires();
  }
}

void RexxActivity::generateRandomNumberSeed()
/******************************************************************************/
/* Function:  Generate a fresh random number seed.                            */
/******************************************************************************/
{
  RexxDateTime  timestamp;             /* current timestamp                 */
  LONG          i;                     /* loop counter                      */
  static int rnd = 0;

  rnd++;
  SysGetCurrentTime(&timestamp);       /* get a fresh time stamp            */
                                       /* take the seed from the time       */
  this->nestedInfo.randomSeed = rnd + (((timestamp.hours * 60 + timestamp.minutes) * 60 + timestamp.seconds) * 1000) + timestamp.microseconds/1000;
  for (i = 0; i < 13; i++) {           /* randomize the seed number a bit   */
                                       /* scramble the seed a bit           */
    this->nestedInfo.randomSeed = RANDOMIZE(this->nestedInfo.randomSeed);
  }
}

long RexxActivity::error(size_t startDepth)
/******************************************************************************/
/* Function:  Force error termination on an activity, returning the resulting */
/*            REXX error code.                                                */
/******************************************************************************/
{
  LONG   rc;                           /* REXX error return code            */

  while (this->depth > startDepth) {   /* while still have activations      */
                                       /* if we have a real activation      */
    if ((RexxObject *)(this->topActivation) != TheNilObject) {
                                       /* force activation termination      */
        this->topActivation->termination();
    }
    this->pop(false);                  /* pop the activation off            */
  }
  rc = Error_Interpretation/1000;      /* set default return code           */
                                       /* did we get a condtion object?     */
  if (this->conditionobj != OREF_NULL) {
                                       /* force it to display               */
    this->display(this->conditionobj);
                                       /* get the failure return code       */
    rc = this->conditionobj->at(OREF_RC)->longValue(Numerics::DEFAULT_DIGITS);
  }
  return rc;                           /* return the error code             */
}

bool RexxActivity::raiseCondition(
    RexxString    *condition,          /* condition to raise                */
    RexxObject    *rc,                 /* return code value                 */
    RexxString    *description,        /* description information           */
    RexxObject    *additional,         /* additional information            */
    RexxObject    *result,             /* result value                      */
    RexxDirectory *exobj)              /* exception object                  */
/******************************************************************************/
/* Function:   Raise an actual condition, causing termination for untrapped   */
/*             conditions                                                     */
/******************************************************************************/
{
  RexxActivationBase *activation;      /* current activation                */
  bool                handled;         /* this condition has been handled   */
  RexxDirectory      *conditionObj;    /* object for created condition      */


  handled = false;                     /* condition not handled yet         */
  if (exobj == OREF_NULL) {            /* need to create a condition object?*/
    conditionObj = new_directory();    /* get a new directory               */
                                       /* put in the condition name         */
    conditionObj->put(condition, OREF_CONDITION);
                                       /* fill in default description       */
    conditionObj->put(OREF_NULLSTRING, OREF_DESCRIPTION);
                                       /* fill in the propagation status    */
    conditionObj->put(TheFalseObject, OREF_PROPAGATED);
  }
  else
    conditionObj = exobj;              /* use the existing object           */
  if (rc != OREF_NULL)                 /* have an RC value?                 */
    conditionObj->put(rc, OREF_RC);    /* add to the condition argument     */
  if (description != OREF_NULL)        /* any description to add?           */
    conditionObj->put(description, OREF_DESCRIPTION);
  if (additional != OREF_NULL)         /* or additional information         */
    conditionObj->put(additional, OREF_ADDITIONAL);
  if (result != OREF_NULL)             /* given a return result?            */
    conditionObj->put(result, OREF_RESULT);

                                       /* invoke the error traps, on all    */
                                       /*  nativeacts until reach 1st       */
                                       /*  also give 1st activation a shot. */
  for (activation = this->current() ; activation != (RexxActivation *)TheNilObject; activation = this->sender(activation)) {
    handled = activation->trap(condition, conditionObj);
    if (isOfClass(Activation, activation)) /* reached our 1st activation yet.   */
      break;                           /* yes, break out of loop            */
  }

  /* Control will not return here if the condition was trapped via*/
  /* SIGNAL ON SYNTAX.  For CALL ON conditions, handled will be   */
  /* true if a trap is pending.                                   */

  return handled;                      /* this has been handled             */
}

void RexxActivity::reportAnException(
    wholenumber_t errcode )            /* REXX error code                   */
/******************************************************************************/
/* Function:  Forward on an exception condition                               */
/******************************************************************************/
{
                                       /* send along with nothing           */
  this->raiseException(errcode, NULL, OREF_NULL, OREF_NULL, OREF_NULL, OREF_NULL);
}

void RexxActivity::reportAnException(
    wholenumber_t errcode,             /* REXX error code                   */
    RexxObject *substitution1 )        /* substitution information          */
/******************************************************************************/
/* Function:  Forward on an exception condition                               */
/******************************************************************************/
{
  this->raiseException(errcode, NULL, OREF_NULL, OREF_NULL, new_array(substitution1), OREF_NULL);
}

void RexxActivity::reportAnException(
    wholenumber_t errcode,             /* REXX error code                   */
    RexxObject *substitution1,         /* substitution information          */
    RexxObject *substitution2 )        /* substitution information          */
/******************************************************************************/
/* Function:  Forward on an exception condition                               */
/******************************************************************************/
{
  this->raiseException(errcode, NULL, OREF_NULL, OREF_NULL, new_array(substitution1, substitution2), OREF_NULL);
}

void RexxActivity::reportAnException(
    wholenumber_t errcode,             /* REXX error code                   */
    RexxObject *substitution1,         /* substitution information          */
    RexxObject *substitution2,         /* substitution information          */
    RexxObject *substitution3 )        /* substitution information          */
/******************************************************************************/
/* Function:  Forward on an exception condition                               */
/******************************************************************************/
{
  this->raiseException(errcode, NULL, OREF_NULL, OREF_NULL, new_array(substitution1, substitution2, substitution3), OREF_NULL);
}

void RexxActivity::reportAnException(
    wholenumber_t errcode,             /* REXX error code                   */
    RexxObject *substitution1,         /* substitution information          */
    RexxObject *substitution2,         /* substitution information          */
    RexxObject *substitution3,         /* substitution information          */
    RexxObject *substitution4 )        /* substitution information          */
/******************************************************************************/
/* Function:  Forward on an exception condition                               */
/******************************************************************************/
{
  this->raiseException(errcode, NULL, OREF_NULL, OREF_NULL, new_array(substitution1, substitution2, substitution3, substitution4), OREF_NULL);
}

void RexxActivity::reportAnException(
    wholenumber_t errcode,             /* REXX error code                   */
    const char *substitution1,         /* substitution information          */
    RexxObject *substitution2,         /* substitution information          */
    const char *substitution3,         /* substitution information          */
    RexxObject *substitution4 )        /* substitution information          */
/******************************************************************************/
/* Function:  Forward on an exception condition                               */
/******************************************************************************/
{
    this->raiseException(errcode, NULL, OREF_NULL, OREF_NULL, new_array(new_string(substitution1), substitution2, new_string(substitution3), substitution4), OREF_NULL);
}

void RexxActivity::reportAnException(
    wholenumber_t errcode,             /* REXX error code                   */
    const char *string )               /* single string sustitution parm    */
/******************************************************************************/
/* Function:  Raise an error using a single REXX character string only        */
/*            as a substitution parameter                                     */
/******************************************************************************/
{
                                       /* convert and forward along         */
  this->reportAnException(errcode, new_string(string));
}

void RexxActivity::reportAnException(
    wholenumber_t errcode,             /* REXX error code                   */
    const char *string,                /* single string sustitution parm    */
    wholenumber_t  integer )           /* single integer substitution parm  */
/******************************************************************************/
/* Function:  Raise an error using a single REXX character string only        */
/*            as a substitution parameter                                     */
/******************************************************************************/
{
                                       /* convert and forward along         */
  this->reportAnException(errcode, new_string(string), new_integer(integer));
}

void RexxActivity::reportAnException(
    wholenumber_t errcode,             /* REXX error code                   */
    const char *string,                /* string value  sustitution parm    */
    wholenumber_t integer,             /* single integer substitution parm  */
    RexxObject   *obj)                 /* and object sub parm               */
/******************************************************************************/
/* Function:  Raise an error using a single REXX character string only        */
/*            as a substitution parameter                                     */
/******************************************************************************/
{
                                       /* convert and forward along         */
  this->reportAnException(errcode, new_string(string), new_integer(integer), obj);
}

void RexxActivity::reportAnException(
    wholenumber_t errcode,             /* REXX error code                   */
    const char *string,                /* string value  sustitution parm    */
    RexxObject   *obj,                 /* and object sub parm               */
    wholenumber_t integer)             /* single integer substitution parm  */
/******************************************************************************/
/* Function:  Raise an error using a single REXX character string only        */
/*            as a substitution parameter                                     */
/******************************************************************************/
{
                                       /* convert and forward along         */
  this->reportAnException(errcode, new_string(string), obj, new_integer(integer));
}

void RexxActivity::reportAnException(
    wholenumber_t errcode,             /* REXX error code                   */
    RexxObject   *obj,                 /* and object sub parm               */
    wholenumber_t integer)             /* single integer substitution parm  */
/******************************************************************************/
/* Function:  Raise an error using a single REXX character string only        */
/*            as a substitution parameter                                     */
/******************************************************************************/
{
                                       /* convert and forward along         */
  this->reportAnException(errcode, obj, new_integer(integer));
}

void RexxActivity::reportAnException(
    wholenumber_t errcode,             /* REXX error code                   */
    RexxObject   *obj,                 /* and object sub parm               */
    const char *string)                /* string value  sustitution parm    */
/******************************************************************************/
/* Function:  Raise an error using a single REXX character string only        */
/*            as a substitution parameter                                     */
/******************************************************************************/
{
                                       /* convert and forward along         */
  this->reportAnException(errcode, obj, new_string(string));
}

void RexxActivity::reportAnException(
    wholenumber_t errcode,             /* REXX error code                   */
    const char *string,                /* string value  sustitution parm    */
    RexxObject   *obj)                 /* and object sub parm               */
/******************************************************************************/
/* Function:  Raise an error using a single REXX character string only        */
/*            as a substitution parameter                                     */
/******************************************************************************/
{
                                       /* convert and forward along         */
  this->reportAnException(errcode, new_string(string), obj);
}

void RexxActivity::reportAnException(
    wholenumber_t errcode,             /* REXX error code                   */
    wholenumber_t  integer )           /* single integer substitution parm  */
/******************************************************************************/
/* Function:  Raise an error using a single REXX integer value only           */
/*            as a substitution parameter                                     */
/******************************************************************************/
{
                                       /* convert and forward along         */
  this->reportAnException(errcode, new_integer(integer));
}

void RexxActivity::reportAnException(
    wholenumber_t errcode,             /* REXX error code                   */
    wholenumber_t  integer,            /* single integer substitution parm  */
    wholenumber_t  integer2)           /* single integer substitution parm  */
/******************************************************************************/
/* Function:  Raise an error using a single REXX integer value only           */
/*            as a substitution parameter                                     */
/******************************************************************************/
{
                                       /* convert and forward along         */
  this->reportAnException(errcode, new_integer(integer), new_integer(integer2));
}

void RexxActivity::reportAnException(
    wholenumber_t errcode,             /* REXX error code                   */
    wholenumber_t  a1,                 /* single integer substitution parm  */
    RexxObject *   a2)                 /* single object substitution parm   */
/******************************************************************************/
/* Function:  Raise an error using a single REXX integer value only           */
/*            as a substitution parameter                                     */
/******************************************************************************/
{
                                       /* convert and forward along         */
  this->reportAnException(errcode, new_integer(a1), a2);
}

void RexxActivity::raiseException(
    wholenumber_t  errcode,            /* REXX error code                   */
    SourceLocation *location,          /* location information              */
    RexxSource    *source,             /* source file to process            */
    RexxString    *description,        /* descriptive information           */
    RexxArray     *additional,         /* substitution information          */
    RexxObject    *result )            /* result information                */
/******************************************************************************/
/* This routine is used for SYNTAX conditions only.                           */
/*                                                                            */
/* The inserts are passed as objects because that happens to be more          */
/* convenient for the calling code in the two cases where this facility       */
/* is used.                                                                   */
/*                                                                            */
/* NOTE: The building of the excepption obejct (EXOBJ)  has been re-arranged  */
/*  so that a garbage collection in the middle of building traceBack/etc      */
/*  doesn't clean up the newly built objects.  SO we create exobj early on    */
/*  save it, and add newlly built objects to exobj as we go.                  */
/******************************************************************************/
{
  RexxDirectory   *exobj;              /* exception object                  */
  RexxList        *traceback;          /* traceback list                    */
  RexxInteger     *position;           /* exception source line             */
  RexxString      *programname;        /* current program name              */
  RexxInteger     *rc;                 /* integer return code               */
  RexxString      *code;               /* error code in decimal form        */
  RexxActivation  *activation;         /* current activation                */
  RexxActivation  *poppedActivation;   /* activation popped from the stack  */
  RexxString      *errortext;          /* primary error message             */
  RexxString      *message;            /* secondary error message           */
  int              primary;            /* primary message code              */
  char             work[10];           /* temp buffer for formatting        */
  int              newVal;

  // during error processing, we need to request the string value of message
  // substitution objects.  It is possible that the string process will also
  // cause a syntax error, resulting in a recursion loop.  We snip that off here,
  // by disallowing a nested error condition.
  if (requestingString)
  {
      throw RecursiveStringError;
  }

  activation = this->getCurrentActivation();     /* get the current activation        */
  while (isOfClass(Activation, activation) && activation->isForwarded()) {
    activation->termination();         /* do activation termiantion process */
    this->pop(false);                  /* pop the top activation off        */
    activation = this->getCurrentActivation();   /* and get the new current one       */
  }
  primary = (errcode / 1000) * 1000;   /* get the primary message number    */
                                       /* format the number (string) into   */
                                       /*  work buffer.                     */
  sprintf(work,"%d.%1d", errcode/1000, errcode - primary);
  code = new_string(work);            /* get the formatted code            */
  newVal = primary/1000;
  rc = new_integer(newVal);            /* get the primary message number    */
                                       /* get the primary message text      */
  errortext = SysMessageText(primary);
  if (errortext == OREF_NULL)          /* no corresponding message          */
                                       /* this is an error                  */
    reportException(Error_Execution_error_condition, code);
  if (primary != errcode) {            /* have a secondary message to issue?*/
                                       /* retrieve the secondary message    */
    message = SysMessageText(errcode);
    if (message == OREF_NULL)          /* no corresponding message          */
                                       /* this is an error                  */
      reportException(Error_Execution_error_condition, code);
  }
  else
                                       /* don't give a secondary message    */
    message = (RexxString *)TheNilObject;

                                       /* All error detection done. we can  */
                                       /*  build and save exobj now.        */
                                       /* get an exception directory        */
  exobj = (RexxDirectory *)new_directory();
  this->conditionobj = exobj;          /* save this in the activity         */
  exobj->put(rc, OREF_RC);             /* add the return code               */
                                       /* fill in the decimal error code    */
  exobj->put(code, OREF_CODE);
  exobj->put(errortext, OREF_ERRORTEXT);
  exobj->put(message, OREF_NAME_MESSAGE);

  if (additional == OREF_NULL)         /* no description given?             */
                                       /* use a zero size array             */
    additional = (RexxArray *)TheNullArray->copy();
  if (description == OREF_NULL)        /* no description?                   */
    description = OREF_NULLSTRING;     /* use a null string                 */

                                       /* fill in the arguments             */
                                       /* fill in the arguments             */
  if (description != OREF_NULL)
  {
      exobj->put(description, OREF_DESCRIPTION);
  }
  if (additional != OREF_NULL)
  {
      exobj->put(additional, OREF_ADDITIONAL);
  }
  if (source != OREF_NULL)
  {
      exobj->put((RexxObject *)source, OREF_SOURCENAME);
  }
  if (result != OREF_NULL)
  {
      exobj->put(result, OREF_RESULT);
  }

  traceback = OREF_NULL;               /* no traceback info                 */

  if (location != NULL) {              /* have clause information available?*/
                                       /* add the line number information   */
    position = new_integer(location->getLineNumber());
  }
                                       /* have an activation?               */
  else if (activation != (RexxActivation *)TheNilObject) {
                                       /* get the activation position       */
    newVal = activation->currentLine();
    position = new_integer(newVal);
  }
  else
    position = OREF_NULL;              /* no position information available */
  exobj->put(position, OREF_POSITION); /* add position to exobj.            */

  traceback = new_list();              /* create a traceback list           */
                                       /* add to the exception object       */
  exobj->put(traceback, OREF_TRACEBACK);
  if (source != OREF_NULL)             /* have source for this?             */
                                       /* extract and add clause in error   */
    traceback->addLast(source->traceBack(*location, 0, false));
                                       /* have predecessors?                */
  if (activation != (RexxActivation *)TheNilObject)
    activation->traceBack(traceback);  /* have them add lines to the list   */
  if (source != OREF_NULL)             /* have source for this?             */
    programname = source->getProgramName(); /* extract program name              */
                                       /* have predecessors?                */
  else if (activation != (RexxActivation *)TheNilObject)
                                       /* extract program name from activa. */
    programname = activation->getProgramName();
  else
    programname = OREF_NULLSTRING;     /* have no program name              */
  exobj->put(programname, OREF_PROGRAM);
  exobj->put(OREF_SYNTAX, OREF_CONDITION);
                                       /* fill in the propagation status    */
  exobj->put(TheFalseObject, OREF_PROPAGATED);

  if (message != TheNilObject) {       /* have a secondary message?         */
                                       /* do required substitutions         */
    message = this->messageSubstitution(message, additional);
                                       /* replace the original message text */
    exobj->put(message, OREF_NAME_MESSAGE);
  }
                                       /* process as common condition       */
  if (!this->raiseCondition(OREF_SYNTAX, OREF_NULL, OREF_NULL, OREF_NULL, OREF_NULL, exobj)) {
                                       /* fill in the propagation status    */
    exobj->put(TheTrueObject, OREF_PROPAGATED);
                                       /* unwind the activation stack       */
    while ((poppedActivation = (RexxActivation *)this->current()) != activation) {
      poppedActivation->termination(); /* do activation termiantion process */
      this->pop(false);                /* pop the activation off the stack  */
    }

        if ((activation != (RexxActivation *)TheNilObject) &&
                (activation->getIndent() > MAX_TRACEBACK_LIST))
                traceback->addLast(new_string("     >...<"));

                                       /* actually have an activation?      */
    if (activation != (RexxActivation *)TheNilObject) {
      activation->termination();       /* do activation termiantion process */
      this->pop(false);                /* pop the top activation off        */
    }
    this->raisePropagate(exobj);       /* pass on down the chain            */
  }
}

RexxString *RexxActivity::messageSubstitution(
    RexxString *message,               /* REXX error message                */
    RexxArray  *additional )           /* substitution information          */
/******************************************************************************/
/* Function:  Perform any required message substitutions on the secondary     */
/*            error message.                                                  */
/******************************************************************************/
{
  LONG        substitutions;           /* number of substitutions           */
  LONG        subposition;             /* substitution position             */
  LONG        i;                       /* loop counter                      */
  LONG        selector;                /* substitution position             */
  RexxString *newmessage;              /* resulting new error message       */
  RexxString *front;                   /* front message part                */
  RexxString *back;                    /* back message part                 */
  RexxObject *value;                   /* substituted message value         */
  RexxString *stringVal;               /* converted substitution value      */

  substitutions = additional->size();  /* get the substitution count        */
  newmessage = OREF_NULLSTRING;        /* start with a null string          */
                                       /* loop through and substitute values*/
  for (i = 1; i <= substitutions; i++) {
                                       /* search for a substitution         */
    subposition = message->pos(OREF_AND, 0);
    if (subposition == 0)              /* not found?                        */
      break;                           /* get outta here...                 */
                                       /* get the leading part              */
    front = message->extract(0, subposition - 1);
                                       /* pull off the remainder            */
    back = message->extract(subposition + 1, message->getLength() - (subposition + 1));
                                       /* get the descriptor position       */
    selector = message->getChar(subposition);
                                       /* not a good number?                */
    if (selector < '0' || selector > '9')
                                       /* use a default message             */
      stringVal = new_string("<BAD MESSAGE>"); /* must be stringValue, not value, otherwise trap */
    else {
      selector -= '0';                 /* convert to a number               */
      if (selector > substitutions)    /* out of our range?                 */
        stringVal = OREF_NULLSTRING;   /* use a null string                 */
      else {                           /* get the indicated selector value  */
        value = additional->get(selector);
        if (value != OREF_NULL) {      /* have a value?                     */
                                       /* set the reentry flag              */
          this->requestingString = true;
          this->stackcheck = false;    /* disable the checking              */
                                       /* now protect against reentry       */
          try
          {
                                       /* force to character form           */
              stringVal = value->stringValue();
          }
          catch (ActivityException)
          {
              stringVal = value->defaultName();
          }
                                       /* we're safe again                  */
          this->requestingString = false;
          this->stackcheck = true;     /* disable the checking              */
        }
        else
                                       /* use a null string                 */
          stringVal = OREF_NULLSTRING;
      }
    }
                                       /* accumulate the front part         */
    newmessage = newmessage->concat(front->concat(stringVal));
    message = back;                    /* replace with the remainder        */
  }
                                       /* add on any remainder              */
  newmessage = newmessage->concat(message);
  return newmessage;                   /* return the message                */
}

void RexxActivity::reraiseException(
    RexxDirectory *exobj )             /* previously created exception obj. */
/******************************************************************************/
/* Function:  Propagate a syntax condition and trapping to earlier levels     */
/******************************************************************************/
{
  RexxActivation *activation;          /* current activation                */
  RexxArray      *additional;          /* passed on information             */
  RexxObject     *errorcode;           /* full error code                   */
  RexxString     *message;             /* secondary error message           */
  int             errornumber;         /* binary error number               */
  int             primary;             /* primary message code              */
  char            work[10];            /* temp buffer for formatting        */
  int             newVal;

  activation = this->getCurrentActivation();/* get the current activation        */
                                       /* have a target activation?         */
  if (activation != (RexxActivation *)TheNilObject)  {
    newVal = activation->currentLine();/* get the activation position       */
    exobj->put(new_integer(newVal), OREF_POSITION);
                                       /* extract program name from activa. */
    exobj->put(activation->getProgramName(), OREF_PROGRAM);
  }
  else {
    exobj->remove(OREF_POSITION);      /* remove the position               */
    exobj->remove(OREF_PROGRAM);       /* remove the program name           */
  }

  errorcode = exobj->at(OREF_CODE);    /* get the error code                */
                                       /* convert to a decimal              */
  errornumber = message_number((RexxString *)errorcode);
                                       /* get the primary message number    */
  primary = (errornumber / 1000) * 1000;
  if (errornumber != primary) {        /* have an actual secondary message? */
                                       /* format the number (string) into   */
                                       /*  work buffer.                     */
    sprintf(work,"%1d%3.3d", errornumber/1000, errornumber - primary);
    errornumber = atol(work);          /* convert to a long value           */
                                       /* retrieve the secondary message    */
    message = SysMessageText(errornumber);
                                       /* Retrieve any additional parameters*/
    additional = (RexxArray *)exobj->at(OREF_ADDITIONAL);
                                       /* do required substitutions         */
    message = this->messageSubstitution(message, additional);
                                       /* replace the original message text */
    exobj->put(message, OREF_NAME_MESSAGE);
  }
  this->raisePropagate(exobj);         /* pass on down the chain            */
}

void RexxActivity::raisePropagate(
    RexxDirectory *conditionObj )      /* condition descriptive information */
/******************************************************************************/
/* Function:   Propagate a condition down the chain of activations            */
/******************************************************************************/
{
  RexxActivationBase *activation;      /* current activation                */
  RexxList           *traceback;       /* traceback information             */
  RexxString         *condition;       /* condition to propagate            */

                                       /* get the condition                 */
  condition = (RexxString *)conditionObj->at(OREF_CONDITION);
                                       /* propagating syntax errors?        */
  if (condition->strCompare(CHAR_SYNTAX))
                                       /* get the traceback                 */
    traceback = (RexxList *)conditionObj->at(OREF_TRACEBACK);
  else
    traceback = OREF_NULL;             /* no trace back to process          */
  activation = this->current();        /* get the current activation        */

                                       /* loop to the top of the stack      */
  while (activation != (RexxActivationBase *)TheNilObject) {
                                       /* give this one a chance to trap    */
                                       /* (will never return for trapped    */
                                       /* PROPAGATE conditions)             */
    activation->trap(condition, conditionObj);
                                       /* this is a propagated condition    */
    conditionObj->put(TheTrueObject, OREF_PROPAGATED);
    if ((traceback != TheNilObject)
                && (((RexxActivation*)activation)->getIndent() < MAX_TRACEBACK_LIST))  /* have a traceback? */
      activation->traceBack(traceback);/* add this to the traceback info    */
    activation->termination();         /* do activation termiantion process */
    this->pop(false);                  /* pop top nativeact/activation      */
    activation = this->current();      /* get the sender's sender           */
  }
                                       /* now kill the activity, using the  */
  this->kill(conditionObj);            /* imbedded description object       */
}

RexxObject *RexxActivity::display(RexxDirectory *exobj)
                                       /* display exception object info     */
                                       /* target exception object           */
/******************************************************************************/
/* Function:  Display information from an exception object                    */
/******************************************************************************/
{
  RexxArray  *trace_back;              /* traceback information             */
  RexxList   *trace_backList;          /* traceback information             */
  RexxString *secondary;               /* secondary message                 */
  RexxString *text;                    /* constructed final message         */
  RexxObject *position;                /* position in program error occurred*/
  RexxString *programname;             /* name of program being run         */
  size_t    i;                         /* loop counter                      */
  int       errorCode;                 /* error message code                */
  RexxObject *rc;

                                       /* get the traceback info            */
  trace_backList = (RexxList *)exobj->at(OREF_TRACEBACK);
  if (trace_backList != OREF_NULL) {   /* have a traceback?                 */
                                       /* convert to an array               */
    trace_back = trace_backList->makeArray();
    ProtectedObject p(trace_back);
                                       /* get the traceback size            */
    size_t tracebackSize = trace_back->size();

    for (i = 1; i <= tracebackSize; i++) {   /* loop through the traceback starttrc */
      text = (RexxString *)trace_back->get(i);
                                       /* have a real line?                 */
      if (text != OREF_NULL && text != TheNilObject)
                                       /* write out the line                */
        this->traceOutput(this->getCurrentActivation(), text);
    }
  }
  rc = exobj->at(OREF_RC);             /* get the error code                */
                                       /* get the message number            */
  errorCode = message_number((RexxString *)rc);
                                       /* get the header                    */
  text = (RexxString *)SysMessageHeader(errorCode);
  if (text == OREF_NULL)               /* no header available?              */
                                       /* get the leading part              */
    text = (RexxString *)SysMessageText(Message_Translations_error);
  else                                 /* add to the message text           */
    text = text->concat(SysMessageText(Message_Translations_error));
                                       /* get the name of the program       */
  programname = (RexxString *)exobj->at(OREF_PROGRAM);
                                       /* add on the error number           */
  text = text->concatWith(REQUEST_STRING(rc), ' ');
                                       /* if program exists, add the name   */
                                       /* of the program to the message     */
  if (programname != (RexxString *)OREF_NULL && programname != OREF_NULLSTRING) {
                                       /* add on the "running" part         */
    text = text->concatWith(SysMessageText(Message_Translations_running), ' ');
                                       /* add on the program name           */
    text = text->concatWith(programname, ' ');
                                       /* Get the position/Line number info */
    position = exobj->at(OREF_POSITION);
    if (position != OREF_NULL) {       /* Do we have position/Line no info? */
                                       /* Yes, add on the "line" part       */
      text = text->concatWith(SysMessageText(Message_Translations_line), ' ');
                                       /* add on the line number            */
      text = text->concatWith(REQUEST_STRING(exobj->at(OREF_POSITION)), ' ');
                                       /* add on the ":  "                  */
    }
  }
  text = text->concatWithCstring(":  ");
                                       /* and finally the error message     */
  text = text->concat((RexxString *)exobj->at(OREF_ERRORTEXT));
                                       /* write out the line                */
  this->traceOutput(this->getCurrentActivation(), text);
                                       /* get the secondary message         */
  secondary = (RexxString *)exobj->at(OREF_NAME_MESSAGE);
                                       /* have a real message?              */
  if (secondary != OREF_NULL && secondary != (RexxString *)TheNilObject) {
    rc = exobj->at(OREF_CODE);         /* get the error code                */
                                       /* get the message number            */
    errorCode = message_number((RexxString *)rc);
                                       /* get the header                    */
    text = (RexxString *)SysMessageHeader(errorCode);
    if (text == OREF_NULL)             /* no header available?              */
                                       /* get the leading part              */
      text = (RexxString *)SysMessageText(Message_Translations_error);
    else                               /* add to the message text           */
      text = text->concat(SysMessageText(Message_Translations_error));
                                       /* add on the error number           */
    text = text->concatWith((RexxString *)rc, ' ');
                                       /* add on the ":  "                  */
    text = text->concatWithCstring(":  ");
                                       /* and finally the error message     */
    text = text->concat(secondary);
                                       /* write out the line                */
    this->traceOutput(this->getCurrentActivation(), text);
  }
  return TheNilObject;                 /* just return .nil                  */
}

RexxObject *RexxActivity::displayDebug(RexxDirectory *exobj)
                                       /* display exception object info     */
                                       /* target exception object           */
/******************************************************************************/
/* Function:  Display information from an exception object                    */
/******************************************************************************/
{
  RexxString *secondary;               /* secondary message                 */
  RexxString *text;                    /* constructed final message         */

                                       /* get the leading part              */
  text = (RexxString *)SysMessageText(Message_Translations_debug_error);
                                       /* add on the error number           */
  text = text->concatWith(REQUEST_STRING(exobj->at(OREF_RC)), ' ');
                                       /* add on the ":  "                  */
  text = text->concatWithCstring(":  ");
                                       /* and finally the error message     */
  text = text->concatWith((RexxString *)exobj->at(OREF_ERRORTEXT), ' ');
                                       /* write out the line                */
  this->traceOutput(this->getCurrentActivation(), text);
                                       /* get the secondary message         */
  secondary = (RexxString *)exobj->at(OREF_NAME_MESSAGE);
                                       /* have a real message?              */
  if (secondary != OREF_NULL && secondary != (RexxString *)TheNilObject) {
                                       /* get the leading part              */
    text = (RexxString *)SysMessageText(Message_Translations_debug_error);
                                       /* add on the error number           */
    text = text->concatWith((RexxString *)exobj->at(OREF_CODE), ' ');
                                       /* add on the ":  "                  */
    text = text->concatWithCstring(":  ");
                                       /* and finally the error message     */
    text = text->concat(secondary);
                                       /* write out the line                */
    this->traceOutput(this->getCurrentActivation(), text);
  }
  return TheNilObject;                 /* just return .nil                  */
}

void RexxActivity::live()
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  size_t  i;                           /* loop counter                      */

  setUpMemoryMark
  memory_mark(this->activations);
  memory_mark(this->topActivation);
  memory_mark(this->getCurrentActivation());
  memory_mark(this->saveValue);
  memory_mark(this->conditionobj);
  memory_mark(this->requiresTable);
  memory_mark(this->nextWaitingActivity);
  memory_mark(this->waitingObject);
  memory_mark(this->nestedInfo.currentExit);
  memory_mark(this->nestedInfo.shvexitvalue);
  for (i = 0; i < LAST_EXIT; i++)
    memory_mark(this->nestedInfo.sysexits[i]);

  /* have the frame stack do its own marking. */
  frameStack.live();
  // mark any protected objects we've been watching over

  ProtectedObject *p = protectedObjects;
  while (p != NULL)
  {
      memory_mark(p->protectedObject);
      p = p->next;
  }

  cleanUpMemoryMark
}
void RexxActivity::liveGeneral()
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  size_t  i;                           /* loop counter                      */

  setUpMemoryMarkGeneral
  memory_mark_general(this->activations);
  memory_mark_general(this->topActivation);
  memory_mark_general(this->currentActivation);
  memory_mark_general(this->saveValue);
  memory_mark_general(this->conditionobj);
  memory_mark_general(this->requiresTable);
  memory_mark_general(this->nextWaitingActivity);
  memory_mark_general(this->waitingObject);
  memory_mark_general(this->nestedInfo.currentExit);
  memory_mark_general(this->nestedInfo.shvexitvalue);

  for (i = 0; i < LAST_EXIT; i++)
    memory_mark_general(this->nestedInfo.sysexits[i]);

  /* have the frame stack do its own marking. */
  frameStack.liveGeneral();

  ProtectedObject *p = protectedObjects;
  while (p != NULL)
  {
      memory_mark_general(p->protectedObject);
      p = p->next;
  }

  cleanUpMemoryMarkGeneral
}

void RexxActivity::flatten(RexxEnvelope* envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
                                       /* Activities don't get moved,       */
                                       /*  we just return OREF_NULL. we may */
                                       /*  create a special proxy for this  */
                                       /*  to re-establish an activation on */
                                       /*  system.                          */
  return;
}

void RexxActivity::run()
/******************************************************************************/
/* Function:  Release an activity to run                                      */
/******************************************************************************/
{
  EVPOST(this->guardsem);              /* and the guard semaphore           */
  EVPOST(this->runsem);                /* post the run semaphore            */
  SysThreadYield();                    /* yield the thread                  */
}

void RexxActivity::push(
    RexxActivationBase *new_activation)/* activation to add                 */
/******************************************************************************/
/* Function:  Push an activation onto the activity stack                      */
/******************************************************************************/
{
  RexxInternalStack *newstack;         /* replacement activation stack      */
  int  i;                              /* loop counter                      */

  if (this->depth == this->size) {     /* reached the end?                  */
                                       /* get a larger stack                */
    newstack = new_internalstack(this->size*2);
    for (i = this->size; i; i--)       /* loop through the old stack        */
                                       /* copying onto the new stack        */
      newstack->push(this->activations->peek((long)(i-1)));
    this->activations = newstack;      /* replace the old stack             */
    this->size *= 2;                   /* size is twice as big              */
  }

                                       /* add to the stack                  */
  this->activations->push((RexxObject *)new_activation);
  this->topActivation = new_activation;/* set this as the top one           */
                                       /* new REXX activation?              */
  if (isOfClass(Activation, new_activation)) {
                                       /* this is the top REXX one too      */
    this->currentActivation = (RexxActivation *)new_activation;
                                       /* get the activation settings       */
    this->numericSettings = ((RexxActivation *)new_activation)->getNumericSettings();
    if (ActivityManager::currentActivity == this)       /* this the active activity?         */
    {
                                       /* update the active values          */
        Numerics::setCurrentSettings(this->numericSettings);
    }
  }
  this->depth++;                       /* bump the depth to count this      */
}

void RexxActivity::pushNil()
/******************************************************************************/
/* Function:  Push an empty activaiton marker on the activity stack           */
/******************************************************************************/
{
  RexxInternalStack *newstack;         /* replacement activation stack      */
  int  i;                              /* loop counter                      */

  if (this->depth == this->size) {     /* reached the end?                  */
                                       /* get a larger stack                */
    newstack = new_internalstack(this->size*2);
    for (i = this->size; i; i--)       /* loop through the old stack        */
                                       /* copying onto the new stack        */
      newstack->push(this->activations->peek((long)(i-1)));
    this->activations = newstack;      /* replace the old stack             */
    this->size *= 2;                   /* size is twice as big              */
  }
                                       /* add to the stack                  */
  this->activations->push(TheNilObject);
                                       /* clear out the cached values       */
  this->topActivation = (RexxActivationBase *)TheNilObject;
                                       /* both of them                      */
  this->currentActivation = (RexxActivation *)TheNilObject;
                                       /* use the default settings          */
  this->numericSettings = Numerics::getDefaultSettings();
  this->depth++;                       /* bump the depth to count this      */
}


void RexxActivity::pop(
    bool  reply)                       /* popping for REPLY purposes        */
/******************************************************************************/
/* Function:  Remove an activation from the activity stack                    */
/******************************************************************************/
{
  RexxActivationBase *top_activation;  /* removed activation                */
  RexxActivationBase *old_activation;  /* removed activation                */
  RexxActivationBase *tempAct = OREF_NULL; /* current loop activation           */
  RexxInternalStack *activationStack;  /* activation stack                  */
  size_t i;                            /* loop counter                      */

  if (0 == this->depth)                /* At the very top of stack?         */
    return;                            /* just return;                      */

  activationStack = this->activations; /* get a local copy                  */
                                       /* pop it off the stack              */
  top_activation = (RexxActivationBase *)activationStack->fastPop();
  this->depth--;                       /* remove the depth                  */
  if (this->depth == 0) {              /* this the last one?                */
                                       /* clear out the cached values       */
    this->topActivation = (RexxActivationBase *)TheNilObject;
                                       /* both of them                      */
    this->currentActivation = (RexxActivation *)TheNilObject;
                                       /* use the default settings          */
    this->numericSettings = Numerics::getDefaultSettings();
  }
  else {                               /* probably have a previous one      */
                                       /* get the top item                  */
    old_activation = (RexxActivationBase *)activationStack->getTop();
                                       /* this is the top one               */
    this->topActivation = old_activation;
                                       /* popping a REXX activation?        */
    if (isOfClass(Activation, top_activation)) {
                                       /* clear this out                    */
      old_activation = (RexxActivationBase *)TheNilObject;
                                       /* spin down the stack               */
      for (i = 0; tempAct != (RexxActivationBase *)TheNilObject && i < this->depth; i++) {
                                       /* get the next item                 */
        tempAct = (RexxActivationBase *)activationStack->peek(i);
                                       /* find a REXX one?                  */
        if (isOfClass(Activation, tempAct)) {
          old_activation = tempAct; /* save this one                     */
          break;                       /* and exit the loop                 */
        }
      }
                                       /* set this as current               */
      this->currentActivation = (RexxActivation *)old_activation;
                                       /* last activation?                  */
      if (old_activation == (RexxActivationBase*)TheNilObject)
                                       /* use the default settings          */
        this->numericSettings = Numerics::getDefaultSettings();
      else
                                       /* get the activation settings       */
        this->numericSettings = ((RexxActivation *)old_activation)->getNumericSettings();
      if (ActivityManager::currentActivity == this)     /* this the active activity?         */
      {
          Numerics::setCurrentSettings(this->numericSettings);
      }
      if (!reply)                      /* not a reply removal?              */
                                       /* add this to the cache             */
        ActivityManager::cacheActivation((RexxActivation *)top_activation);
    }
                                       /* did we pop off .NIL?              */
    else if (top_activation == (RexxActivationBase *)TheNilObject) {
      activationStack->push(TheNilObject); /* Yes, force back on.               */
      this->depth++;                   /* step the depth back up            */
    }
  }
}

void RexxActivity::popNil()
/******************************************************************************/
/* Function:  Remove an activation marker from the activity stack             */
/******************************************************************************/
{
  RexxActivationBase *old_activation;  /* removed activation                */
  RexxActivationBase *tempAct = OREF_NULL; /* current loop activation,      */
  RexxInternalStack *activationStack;  /* activation stack                  */
  size_t i;                            /* loop counter                      */

  activationStack = this->activations; /* get a local copy                  */
  activationStack->fastPop();          /* pop it off the stack              */
  this->depth--;                       /* remove the depth                  */
  if (this->depth <= 0) {              /* this the last one?                */
                                       /* clear out the cached values       */
    this->topActivation = (RexxActivationBase *)TheNilObject;
                                       /* both of them                      */
    this->currentActivation = (RexxActivation *)TheNilObject;
                                       /* use the default settings          */
    this->numericSettings = Numerics::getDefaultSettings();
    this->depth = 0;                   /* make sure this is zero            */
  }
  else {                               /* probably have a previous one      */
                                       /* get the top item                  */
    old_activation = (RexxActivationBase *)activationStack->getTop();
                                       /* this is the top one               */
    this->topActivation = old_activation;
                                       /* clear this out                    */
    old_activation = (RexxActivationBase *)TheNilObject;
                                       /* spin down the stack               */
    for (i = 0; tempAct != (RexxActivationBase *)TheNilObject && i < this->depth; i++) {
                                       /* get the next item                 */
      tempAct = (RexxActivationBase *)activationStack->peek(i);
                                       /* find a REXX one?                  */
      if (isOfClass(Activation, tempAct)) {
        old_activation = tempAct;   /* save this one                     */
        break;                         /* and exit the loop                 */
      }
    }
                                       /* set this as current               */
    this->currentActivation = (RexxActivation *)old_activation;
                                       /* last activation?                  */
    if (old_activation == (RexxActivationBase *)TheNilObject)
                                       /* use the default settings          */
        this->numericSettings = Numerics::getDefaultSettings();
    else
                                       /* get the activation settings       */
      this->numericSettings = ((RexxActivation *)old_activation)->getNumericSettings();
  }
}

void RexxActivity::exitKernel(
  RexxActivation *activation,          /* activation going external on      */
  RexxString     *message_name,        /* reason for going external         */
  bool            enable )             /* variable pool enabled flag        */
/******************************************************************************/
/*  Function:  Release the kernel access because code is going to "leave"     */
/*             the kernel.  This prepares for this by adding a native         */
/*             activation on to the stack to act as a server for the          */
/*             external call.  This way new native methods do not need to     */
/*             be created just to get an activation created                   */
/******************************************************************************/
{
  RexxNativeActivation *new_activation;/* new native activation             */

                                       /* create a new native activation    */
  new_activation = new ((RexxObject *)activation, (RexxMethod *)OREF_NULL, this, message_name, (RexxActivationBase *)activation) RexxNativeActivation;
                                       /* push it on the activity stack     */
  this->push((RexxActivationBase *)new_activation);
  if (enable)                          /* need variable pool access?        */
                                       /* turn it on now                    */
    new_activation->enableVariablepool();
  releaseAccess();                     /* now give up control               */
}
void RexxActivity::enterKernel()
/******************************************************************************/
/*  Function:  Recover the kernel access and pop the native activation        */
/*             created by activity_exit_kernel from the activity stack.       */
/******************************************************************************/
{
  requestAccess();                     /* get the kernel lock back          */
  ((RexxNativeActivation *)this->topActivation)->disableVariablepool();
  this->pop(false);                    /* pop the top activation            */
}

void RexxActivity::checkDeadLock(
  RexxActivity *targetActivity)        /* activity currently reserving      */
/******************************************************************************/
/* Function:  Check for a circular wait dead lock error                       */
/******************************************************************************/
{
  RexxActivity *owningActivity;        /* activity owning the resource      */
                                       /* currently waiting on something?   */
  if (this->waitingObject != OREF_NULL) {
                                       /* waiting on a message object?      */
    if (isOfClass(Message, this->waitingObject))
                                       /* get the activity message is on    */
      owningActivity = ((RexxMessage *)this->waitingObject)->startActivity;
    else
                                       /* get the locking activity for vdict*/
      owningActivity = ((RexxVariableDictionary *)this->waitingObject)->getReservingActivity();
                                       /* have a circular wait              */
    if (owningActivity == targetActivity)
                                       /* have a deaklock                   */
      reportException(Error_Execution_deadlock);
    if (owningActivity != OREF_NULL)   /* have a real activity?             */
                                       /* pass it along the chain           */
      owningActivity->checkDeadLock(targetActivity);
  }
}

void RexxActivity::waitReserve(
  RexxObject *resource )               /* resource we are waiting on        */
/******************************************************************************/
/* Function:  Wait for a new run event to occur                               */
/******************************************************************************/
{
  EVSET(this->runsem);                 /* clear the run semaphore           */
  this->waitingObject = resource;      /* save the waiting resource         */
  releaseAccess();                     /* release the kernel access         */
  EVWAIT(this->runsem);                /* wait for the run to be posted     */
  requestAccess();                     /* reaquire the kernel access        */
}

void RexxActivity::guardWait()
/******************************************************************************/
/* Function:  Wait for a guard post event                                     */
/******************************************************************************/
{
  releaseAccess();                     /* release kernel access             */
  EVWAIT(this->guardsem);              /* wait on the guard semaphore       */
  requestAccess();                     /* reaquire the kernel lock          */
}

void RexxActivity::guardPost()
/******************************************************************************/
/* Function:  Post a guard expression wake up notice                          */
/******************************************************************************/
{
                                       /* make sure we have access to sem   */
  EVOPEN(this->guardsem);              /* may be called from another process*/
  EVPOST(this->guardsem);              /* OK for it to already be posted    */
  EVCL(this->guardsem);                /* release access to sem.            */
}

void RexxActivity::guardSet()
/******************************************************************************/
/* Function:  Clear a guard expression semaphore in preparation to perform a  */
/*            guard wait                                                      */
/******************************************************************************/
{
  EVSET(this->guardsem);               /* set up for guard call             */
}

void RexxActivity::postRelease()
/******************************************************************************/
/* Function:  Post an activities run semaphore                                */
/******************************************************************************/
{
  this->waitingObject = OREF_NULL;     /* no longer waiting                 */
                                       /* make sure we have access to sem   */
  EVOPEN(this->runsem);                /* may be called from another process*/
  EVPOST(this->runsem);                /* OK for it to already be posted    */
  EVCL(this->runsem);                  /* release access to sem.            */
}

void RexxActivity::kill(
    RexxDirectory *conditionObj)       /* associated "kill" object          */
/******************************************************************************/
/* Function:  Kill a running activity,                                        */
/******************************************************************************/
{
  this->conditionobj = conditionObj;   /* save the condition object         */
  throw UnhandledCondition;            // we have a fatal error condition
}

RexxActivationBase *RexxActivity::sender(
    RexxActivationBase *activation)    /* target activation                 */
/******************************************************************************/
/* Function:  Return a reference to the activation prior to the target        */
/*            activation                                                      */
/******************************************************************************/
{
  size_t i;                            /* loop counter                      */

  for (i = 0; i < this->depth &&       /* loop to the target activation     */
      (RexxActivationBase *)this->activations->peek(i) != activation; i++) ;
  if (i < this->depth-1)               /* anything left over?               */
                                       /* return the previous version       */
    return (RexxActivation *)this->activations->peek(i+1);
  else
                                       /* just return OREF_NIL              */
    return (RexxActivation *) TheNilObject;
}

void RexxActivity::relinquish()
/******************************************************************************/
/*  Function: Relinquish to other system processes                            */
/******************************************************************************/
{
    ActivityManager::relinquish(this);
}

void RexxActivity::yield(RexxObject *result)
/******************************************************************************/
/* Function:  Yield control so some other activity can run                    */
/******************************************************************************/
{
  //TODO:  Use protected object here
                                       /* other's waiting to go?            */
  if (ActivityManager::hasWaiters()) {
    this->saveValue = result;          /* save the result value             */
                                       /* now join the line                 */
    ActivityManager::addWaitingActivity(this, true);
    holdObject(result);                /* hold the result                   */
    this->saveValue = OREF_NULL;       /* release the saved value           */
  }
}


/**
 * Tap the current running activation on this activity to
 * give up control at the next reasonsable boundary.
 */
void RexxActivity::yield()
{
                                       /* get the current activation        */
    RexxActivation *activation = currentActivation;
                                       /* got an activation?                */
    if ((activation != NULL) && (activation != (RexxActivation *)TheNilObject))
    {
                                       /* tell it to yield                  */
        activation->yield();
    }
}


/**
 * Tap the current running activation on this activity to halt
 * as soon as possible.
 *
 * @param d      The description string for the halt.
 *
 * @return true if we have an activation to tell to stop, false if the
 *         activity's not really working.
 */
bool RexxActivity::halt(RexxString *d)
{
                                       /* get the current activation        */
    RexxActivation *activation = currentActivation;
                                       /* got an activation?                */
    if ((activation != NULL) && (activation != (RexxActivation *)TheNilObject))
    {
        // please make it stop :-)
        activation->halt(d);
        return true;
    }
    return false;
}


/**
 * Tap the current running activation on this activity to halt
 * as soon as possible.
 *
 * @param d      The description string for the halt.
 *
 * @return true if we have an activation to tell to stop, false if the
 *         activity's not really working.
 */
bool RexxActivity::setTrace(bool on)
{
                                       /* get the current activation        */
    RexxActivation *activation = currentActivation;
                                       /* got an activation?                */
    if ((activation != NULL) && (activation != (RexxActivation *)TheNilObject))
    {
        if (on)                        /* turning this on?                  */
        {
                                       /* turn tracing on                   */
            activation->externalTraceOn();
        }
        else
        {
                                       /* turn tracing off                  */
            activation->externalTraceOff();
        }
        return true;
    }
    return false;
}

void RexxActivity::releaseAccess()
/******************************************************************************/
/* Function:  Release exclusive access to the kernel                          */
/******************************************************************************/
{
    ActivityManager::unlockKernel();
    // reset the numeric settings
    Numerics::setDefaultSettings();
}

void RexxActivity::requestAccess()
/******************************************************************************/
/* Function:  Acquire exclusive access to the kernel                          */
/******************************************************************************/
{
                                       /* only one there?                   */
    if (ActivityManager::lockKernelImmediate())
    {
        ActivityManager::currentActivity = this;          /* set new current activity          */
        /* and new active settings           */
        Numerics::setCurrentSettings(numericSettings);
        return;                          /* get out if we have it             */
    }
    /* can't get it, go stand in line    */
    ActivityManager::addWaitingActivity(this, false);
}

void RexxActivity::checkStackSpace()
/******************************************************************************/
/* Function:  Make sure there is enough stack space to run a method           */
/******************************************************************************/
{
#ifdef STACKCHECK
  long temp;                           /* if checking and there isn't room  */
  if (PTRSUB2(&temp,this->nestedInfo.stackptr) < MIN_C_STACK && this->stackcheck == true)
                                       /* go raise an exception             */
    reportException(Error_Control_stack_full);
#endif
}

RexxObject *RexxActivity::localMethod()
/******************************************************************************/
/* Function:  Retrive the activities local environment                        */
/******************************************************************************/
{
  return ActivityManager::localEnvironment; // just return the .local directory
}

long  RexxActivity::threadIdMethod()
/******************************************************************************/
/* Function:  Retrieve the activities threadid                                */
/******************************************************************************/
{
  return (long)this->threadid;         /* just return the thread id info    */
}

void  RexxActivity::setShvVal(
    RexxString *retval)
/******************************************************************************/
/* Function:  Set a return value as a result of a call to the variable pool   */
/*            with the RXSHV_EXIT shvcode specified                           */
/******************************************************************************/
{
                                       /* set the ret'd value from var pool */
  this->nestedInfo.shvexitvalue = retval;
}

void RexxActivity::queryTrcHlt()
/******************************************************************************/
/* Function:  Determine if Halt or Trace System exits are set                 */
/*            and set a flag to indicate this.                                */
/******************************************************************************/
{                                      /* is HALT sys exit set              */
  if (this->nestedInfo.sysexits[RXHLT - 1] != OREF_NULL) {
    this->nestedInfo.clauseExitUsed = true;   /* set flag to indicate one is found */
    return;                            /* and return                        */
  }
                                       /* is TRACE sys exit set             */
  if (this->nestedInfo.sysexits[RXTRC - 1] != OREF_NULL) {
    this->nestedInfo.clauseExitUsed = true;   /* set flag to indicate one is found */
    return;                            /* and return                        */
  }

  this->nestedInfo.clauseExitUsed = false;    /* remember that none are set        */
}

void RexxActivity::sysExitInit(
    RexxActivation *activation)        /* sending activation                */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the initialization system exit.                                */
/******************************************************************************/
{
  RexxString *exitname;                /* Exit routine name                 */

                                       /* get the exit handler              */
  exitname = this->querySysExits(RXINI);
  if (exitname != OREF_NULL) {         /* exit enabled?                     */

                                       /* add the variable RXPROGRAMNAME to */
                                       /* the variable pool, it contains the*/
                                       /* script name that is currently run */
    RexxString   *varName = new_string("RXPROGRAMNAME");
    RexxString   *sourceStr = activation->getProgramName();
    RexxVariableDictionary *vdict = activation->getLocalVariables();
    RexxVariable *pgmName = vdict->createVariable(varName);
    pgmName->set(sourceStr);
                                       /* call the handler                  */
    SysExitHandler(this, activation, exitname, RXINI, RXINIEXT, NULL, true);
                                       /* if variable was not changed, then */
                                       /* remove it from the var. pool again*/
    if (sourceStr == pgmName->getVariableValue()) {
      vdict->remove(varName);
    }
  }
}

void RexxActivity::sysExitTerm(
    RexxActivation *activation)        /* sending activation                */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the termination system exit.                                   */
/******************************************************************************/
{
  RexxString *exitname;                /* Exit routine name                 */

                                       /* get the exit handler              */
  exitname = this->querySysExits(RXTER);
  if (exitname != OREF_NULL)           /* exit enabled?                     */
                                       /* call the handler                  */
    SysExitHandler(this, activation, exitname, RXTER, RXTEREXT, NULL, true);
}

bool  RexxActivity::sysExitSioSay(
    RexxActivation *activation,        /* sending activation                */
    RexxString     *sayoutput)         /* line to write out                 */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the I/O system exit.                                           */
/******************************************************************************/
{
  RexxString   *exitname;              /* Exit routine name                 */
  RXSIOSAY_PARM exit_parm;             /* exit parameters                   */

                                       /* get the exit handler              */
  exitname = this->querySysExits(RXSIO);
  if (exitname != OREF_NULL) {         /* exit enabled?                     */
                                       /* make into RXSTRING form           */
    MAKERXSTRING(exit_parm.rxsio_string, sayoutput->getWritableData(),  sayoutput->getLength());
                                       /* call the handler                  */
    return SysExitHandler(this, activation, exitname, RXSIO, RXSIOSAY, (PVOID)&exit_parm, false);
  }
  return true;                         /* exit didn't handle                */
}

bool RexxActivity::sysExitSioTrc(
    RexxActivation *activation,        /* sending activation                */
    RexxString     *traceoutput)       /* line to write out                 */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the I/O system exit.                                           */
/******************************************************************************/
{
  RexxString *  exitname;              /* Exit routine name                 */
  RXSIOTRC_PARM exit_parm;             /* exit parameters                   */

                                       /* get the exit handler              */
  exitname = this->querySysExits(RXSIO);
  if (exitname != OREF_NULL) {         /* exit enabled?                     */
                                       /* make into RXSTRING form           */
    MAKERXSTRING(exit_parm.rxsio_string, traceoutput->getWritableData(), traceoutput->getLength());
                                       /* call the handler                  */
    return SysExitHandler(this, activation, exitname, RXSIO, RXSIOTRC, (PVOID)&exit_parm, false);
  }
  return true;                         /* exit didn't handle                */
}

bool RexxActivity::sysExitSioTrd(
    RexxActivation *activation,        /* sending activation                */
    RexxString **inputstring)          /* returned input string             */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the I/O system exit.                                           */
/******************************************************************************/
{
  RexxString   *exitname;              /* Exit routine name                 */
  RXSIOTRD_PARM exit_parm;             /* exit parameters                   */
  char          retbuffer[DEFRXSTRING];/* Default result buffer             */

  *retbuffer = '\0';
                                       /* get the exit handler              */
  exitname = this->querySysExits(RXSIO);
  if (exitname != OREF_NULL) {         /* exit enabled?                     */
                                       /* Pass along default RXSTRING       */
/*     MAKERXSTRING(exit_parm.rxsiotrd_retc, retbuffer, DEFRXSTRING); */
    MAKERXSTRING(exit_parm.rxsiotrd_retc, retbuffer, 0);
                                       /* init shvexit return buffer        */
    this->nestedInfo.shvexitvalue = OREF_NULL;
                                       /* call the handler                  */
    if (SysExitHandler(this, activation, exitname, RXSIO, RXSIOTRD, (PVOID)&exit_parm, false))
      return true;                     /* this wasn't handled               */
                                       /* shv_exit return a value?          */
    if (this->nestedInfo.shvexitvalue != OREF_NULL) {
                                       /* return this information           */
      *inputstring = (RexxString *)this->nestedInfo.shvexitvalue;
      return false;                    /* return that request was handled   */
    }
                                       /* Get input string and return it    */
    *inputstring = (RexxString *)new_string(exit_parm.rxsiotrd_retc.strptr, exit_parm.rxsiotrd_retc.strlength);
                                       /* user give us a new buffer?        */
    if (exit_parm.rxsiotrd_retc.strptr != retbuffer)
                                       /* free it                           */
      SysReleaseResultMemory(exit_parm.rxsiotrd_retc.strptr);
    return false;                      /* this was handled                  */
  }
  return true;                         /* not handled                       */
}

bool RexxActivity::sysExitSioDtr(
    RexxActivation *activation,        /* sending activation                */
    RexxString    **inputstring)       /* returned input value              */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the I/O system exit.                                           */
/******************************************************************************/
{
  RexxString   *exitname;              /* Exit routine name                 */
  RXSIODTR_PARM exit_parm;             /* exit parameters                   */
  char          retbuffer[DEFRXSTRING];/* Default result buffer             */

  *retbuffer = '\0';
                                       /* get the exit handler              */
  exitname = this->querySysExits(RXSIO);
  if (exitname != OREF_NULL) {         /* exit enabled?                     */
                                       /* Pass along default RXSTRING       */
/*     MAKERXSTRING(exit_parm.rxsiodtr_retc, retbuffer, DEFRXSTRING); */
    MAKERXSTRING(exit_parm.rxsiodtr_retc, retbuffer, 0);
                                       /* init shvexit return buffer        */
    this->nestedInfo.shvexitvalue = OREF_NULL;
                                       /* call the handler                  */
    if (SysExitHandler(this, activation, exitname, RXSIO, RXSIODTR, (PVOID)&exit_parm, false))
      return true;                     /* this wasn't handled               */
                                       /* shv_exit return a value?          */
    if (this->nestedInfo.shvexitvalue != OREF_NULL) {
                                       /* return this information           */
      *inputstring = (RexxString *)this->nestedInfo.shvexitvalue;
      return false;                    /* return that request was handled   */
    }
                                       /* Get input string and return it    */
    *inputstring = (RexxString *)new_string(exit_parm.rxsiodtr_retc.strptr, exit_parm.rxsiodtr_retc.strlength);
                                       /* user give us a new buffer?        */
    if (exit_parm.rxsiodtr_retc.strptr != retbuffer)
                                       /* free it                           */
      SysReleaseResultMemory(exit_parm.rxsiodtr_retc.strptr);
    return false;                      /* this was handled                  */
  }
  return true;                         /* not handled                       */
}

bool RexxActivity::sysExitFunc(
    RexxActivation *activation,        /* calling activation                */
    RexxString     *rname,             /* routine name                      */
    RexxObject     *calltype,          /* type of call                      */
    RexxObject     **funcresult,       /* function result                   */
    RexxObject    **arguments,         /* argument array                    */
    size_t          argcount)          /* argument count                    */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the function system exit.                                      */
/******************************************************************************/
{
  RexxString   *exitname;              /* Exit routine name                 */
  RXFNCCAL_PARM exit_parm;             /* exit parameters                   */
  char          retbuffer[DEFRXSTRING];/* Default result buffer             */
  long          argindex;              /* Counter for arg array             */
  PRXSTRING     argrxarray;            /* Array of args in PRXSTRING form   */
  RexxString   *temp;                  /* temporary argument                */
  RexxString   *stdqueue;              /* current REXX queue                */
  RexxDirectory *securityArgs;         /* security check arguments          */
  bool         wasNotHandled;

                                       /* need to do a security check?      */
  if (activation->hasSecurityManager()) {
    securityArgs = new_directory();    /* get a directory item              */
                                       /* add the function name             */
    securityArgs->put(rname, OREF_NAME);
                                       /* and the arguments                 */
    securityArgs->put(new (argcount, arguments) RexxArray, OREF_ARGUMENTS);
                                       /* did manager handle this?          */
    if (activation->callSecurityManager(OREF_CALL, securityArgs)) {
                                       /* get the return code               */
      *funcresult = securityArgs->fastAt(OREF_RESULT);
      return false;                    /* we've handled this                */
    }
  }
                                       /* get the exit handler              */
  exitname = this->querySysExits(RXFNC);
#ifdef SCRIPTING
  if (exitname == OREF_NULL) {
    const char *data = ((RexxString*) calltype)->getStringData();
    // special "calltype"? make a real one out of it and check
    if (data[0] == 'A' && data[1] == 'X') {
      sscanf(((RexxString*) calltype)->getStringData()+2,"%p",&calltype);
      if (calltype)
        exitname = this->querySysExits(RXEXF); // external call (Script Engine?)
    }
  }
#endif
  if (exitname != OREF_NULL) {         /* exit enabled?                     */
                                       /* Start building the exit block  */
    exit_parm.rxfnc_flags.rxfferr = 0; /* Initialize error codes to zero */
    exit_parm.rxfnc_flags.rxffnfnd = 0;
    if (calltype == OREF_FUNCTIONNAME) /* function call?                    */
                                       /* set the function flag             */
      exit_parm.rxfnc_flags.rxffsub = 0;
    else
                                       /* this is the CALL instruction      */
      exit_parm.rxfnc_flags.rxffsub = 1;
                                       /* fill in the name parameter        */
    exit_parm.rxfnc_namel = rname->getLength();
    exit_parm.rxfnc_name = rname->getWritableData();

                                       /* Get current active queue name     */
    stdqueue = (RexxString *)SysGetCurrentQueue();
                                       /* fill in the name                  */
    exit_parm.rxfnc_que = stdqueue->getWritableData();
                                       /* and the length                    */
    exit_parm.rxfnc_quel = stdqueue->getLength();
                                       /* Build arg array of RXSTRINGs      */
                                       /* get number of args                */
    exit_parm.rxfnc_argc = argcount;


    /* allocate enough memory for all arguments.           */
    /* At least one item needs to be allocated in order to avoid an error   */
    /* in the sysexithandler!                                               */
    argrxarray = (PRXSTRING) SysAllocateResultMemory(
                    sizeof(RXSTRING) * max(exit_parm.rxfnc_argc,1));
    if (argrxarray == OREF_NULL)       /* memory error?                     */
      reportException(Error_System_resources);
                                       /* give the arg array pointer        */
    exit_parm.rxfnc_argv = argrxarray;
                                       /* construct the arg array           */
    for (argindex=0; argindex < exit_parm.rxfnc_argc; argindex++) {
                                       /* get the next argument             */
      if (this->exitObjects == true) {
        // store pointers to rexx objects
        argrxarray[argindex].strlength = 8; // pointer length in ASCII
        argrxarray[argindex].strptr = (char *)SysAllocateExternalMemory(16);
        sprintf(argrxarray[argindex].strptr,"%p",arguments[argindex]); // ptr to object
      } else {
        // classic REXX style interface
        temp = (RexxString *)arguments[argindex];
        if (temp != OREF_NULL) {         /* got a real argument?              */
                                         /* force the argument to a string    */
          temp = (RexxString *)REQUEST_STRING(temp);
                                         /* point to the string               */
          argrxarray[argindex].strlength = temp->getLength();
          argrxarray[argindex].strptr = temp->getWritableData();
        }
        else {
                                         /* empty argument                    */
          argrxarray[argindex].strlength = 0;
          argrxarray[argindex].strptr = (char *)NULL;
        }
      }
    }
                                       /* Pass default result RXSTRING      */
    MAKERXSTRING(exit_parm.rxfnc_retc, retbuffer, DEFRXSTRING);
                                       /* init shvexit return buffer        */
    this->nestedInfo.shvexitvalue = OREF_NULL;
                                       /* call the handler                  */
    wasNotHandled = SysExitHandler(this, activation, exitname, RXFNC, RXFNCCAL, (PVOID)&exit_parm, true);

    /* release the memory allocated for the arguments */
    // free memory that was needed to pass objects
    if (this->exitObjects == true)
    {
        for (argindex=0; argindex < exit_parm.rxfnc_argc; argindex++)
        {
            SysFreeExternalMemory(argrxarray[argindex].strptr);
        }
    }
    SysReleaseResultMemory(argrxarray);

    if (wasNotHandled)
      return true;                     /* this wasn't handled               */


    if (exit_parm.rxfnc_flags.rxfferr) /* function error?                   */
                                       /* raise an error                    */
      reportException(Error_Incorrect_call_external, rname);
                                       /* Did we find the function??        */
    else if (exit_parm.rxfnc_flags.rxffnfnd)
                                       /* also an error                     */
      reportException(Error_Routine_not_found_name,rname);
                                       /* shv_exit return a value?          */
    if (this->nestedInfo.shvexitvalue != OREF_NULL) {
                                       /* return this information           */
      *funcresult = this->nestedInfo.shvexitvalue;
      return false;                    /* return that request was handled   */
    }
                                       /* Was it a function call??          */
    if (exit_parm.rxfnc_retc.strptr == OREF_NULL && calltype == OREF_FUNCTIONNAME)
                                       /* Have to return data               */
      reportException(Error_Function_no_data_function,rname);

    if (exit_parm.rxfnc_retc.strptr) { /* If we have result, return it      */
      // is this a real object?
      if (this->exitObjects == true) {
        RexxObject *transfer = NULL;
        if (sscanf(exit_parm.rxfnc_retc.strptr,"%p",&transfer) == 1)
          *funcresult = transfer;
        else
          reportException(Error_Function_no_data_function,rname);
      } else
                                       /* Get input string and return it    */
        *funcresult = new_string((char *)exit_parm.rxfnc_retc.strptr, exit_parm.rxfnc_retc.strlength);
                                       /* user give us a new buffer?        */
      if (exit_parm.rxfnc_retc.strptr != retbuffer)
                                       /* free it                           */
        SysReleaseResultMemory(exit_parm.rxfnc_retc.strptr);
    }
    return false;                      /* this was handled                  */
  }
  return true;                         /* not handled                       */
}


bool RexxActivity::sysExitCmd(
     RexxActivation *activation,       /* issuing activation                */
     RexxString *cmdname,              /* command name                      */
     RexxString *environment,          /* environment                       */
     RexxString **conditions,          /* failure/error conditions status   */
     RexxObject **cmdresult)           /* function result                   */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the command system exit.                                       */
/******************************************************************************/
{
  RexxString    *exitname;             /* Exit routine name                 */
  RXCMDHST_PARM exit_parm;             /* exit parameters                   */
  char          retbuffer[DEFRXSTRING];/* Default result buffer             */
  RexxDirectory *securityArgs;         /* security check arguments          */

                                       /* need to do a security check?      */
  if (activation->hasSecurityManager()) {
    securityArgs = new_directory();    /* get a directory item              */
                                       /* add the command                   */
    securityArgs->put(cmdname, OREF_COMMAND);
                                       /* and the target                    */
    securityArgs->put(environment, OREF_ADDRESS);
                                       /* did manager handle this?          */
    if (activation->callSecurityManager(OREF_COMMAND, securityArgs)) {
                                       /* get the return code               */
      *cmdresult = securityArgs->fastAt(OREF_RC);
      if (*cmdresult == OREF_NULL)     /* no return code provide?           */
        *cmdresult = IntegerZero;      /* use a zero return code            */
                                       /* failure indicated?                */
      if (securityArgs->fastAt(OREF_FAILURENAME) != OREF_NULL)
        *conditions = OREF_FAILURENAME;/* raise a FAILURE condition         */
                                       /* how about an error condition?     */
      else if (securityArgs->fastAt(OREF_ERRORNAME) != OREF_NULL)
        *conditions = OREF_ERRORNAME;  /* raise an ERROR condition          */
      return false;                    /* we've handled this                */
    }
  }
                                       /* get the exit handler              */
  exitname = this->querySysExits(RXCMD);
  if (exitname != OREF_NULL) {         /* exit enabled?                     */
                                       /* Start building the exit block     */
    exit_parm.rxcmd_flags.rxfcfail = 0;/* Initialize failure/error to zero  */
    exit_parm.rxcmd_flags.rxfcerr = 0;
                                       /* fill in the environment parm      */
    exit_parm.rxcmd_addressl = environment->getLength();
    exit_parm.rxcmd_address = environment->getWritableData();
                                       /* make cmdaname into RXSTRING form  */
    MAKERXSTRING(exit_parm.rxcmd_command, cmdname->getWritableData(), cmdname->getLength());

    exit_parm.rxcmd_dll = NULL;        /* Currently no DLL support          */
    exit_parm.rxcmd_dll_len = 0;       /* 0 means .EXE style                */
                                       /* Pass default result RXSTRING      */
    MAKERXSTRING(exit_parm.rxcmd_retc, retbuffer, DEFRXSTRING);
                                       /* init shvexit return buffer        */
    this->nestedInfo.shvexitvalue = OREF_NULL;
                                       /* call the handler                  */
    if (SysExitHandler(this, activation, exitname, RXCMD, RXCMDHST, (PVOID)&exit_parm, true))
      return true;                     /* this wasn't handled               */
    if (exit_parm.rxcmd_flags.rxfcfail)/* need to raise failure condition?  */

      *conditions = OREF_FAILURENAME;  /* raise an FAILURE condition        */
                                       /* Did we find the function??        */
    else if (exit_parm.rxcmd_flags.rxfcerr)
                                       /* Need to raise error condition?    */
      *conditions = OREF_ERRORNAME;    /* raise an ERROR condition          */
                                       /* shv_exit return a value?          */
    if (this->nestedInfo.shvexitvalue != OREF_NULL) {
                                       /* return this information           */
      *cmdresult = this->nestedInfo.shvexitvalue;
      return false;                    /* return that request was handled   */
    }
                                       /* Get input string and return it    */
    *cmdresult = new_string(exit_parm.rxcmd_retc.strptr, exit_parm.rxcmd_retc.strlength);
                                       /* user give us a new buffer?        */
    if (exit_parm.rxcmd_retc.strptr != retbuffer)
                                       /* free it                           */
      SysReleaseResultMemory(exit_parm.rxcmd_retc.strptr);

    return false;                      /* this was handled                  */
  }
  return true;                         /* not handled                       */
}

bool  RexxActivity::sysExitMsqPll(
    RexxActivation *activation,        /* sending activation                */
    RexxString **inputstring)          /* returned input string             */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the External Data queue system exit.                           */
/******************************************************************************/
{
  RexxString   *exitname;              /* Exit routine name                 */
  RXMSQPLL_PARM exit_parm;             /* exit parameters                   */
  char          retbuffer[DEFRXSTRING];/* Default result buffer             */

                                       /* get the exit handler              */
  exitname = this->querySysExits(RXMSQ);
  if (exitname != OREF_NULL) {         /* exit enabled?                     */
                                       /* Pass along default RXSTRING       */
    MAKERXSTRING(exit_parm.rxmsq_retc, retbuffer, DEFRXSTRING);
                                       /* init shvexit return buffer        */
    this->nestedInfo.shvexitvalue = OREF_NULL;
                                       /* call the handler                  */
    if (SysExitHandler(this, activation, exitname, RXMSQ, RXMSQPLL, (PVOID)&exit_parm, false))
      return true;                     /* this wasn't handled               */
                                       /* Get input string and return it    */
                                       /* shv_exit return a value?          */
    if (this->nestedInfo.shvexitvalue != OREF_NULL) {
                                       /* return this information           */
      *inputstring = (RexxString *)this->nestedInfo.shvexitvalue;
      return false;                    /* return that request was handled   */
    }
    if (!(exit_parm.rxmsq_retc.strptr))/* if rxstring not null              */
                                       /* no value returned,                */
                                       /* return NIL to note empty stack    */
      *inputstring = (RexxString *)TheNilObject;
    else                               /* return resulting object           */
      *inputstring = (RexxString *)new_string(exit_parm.rxmsq_retc.strptr, exit_parm.rxmsq_retc.strlength);
                                       /* user give us a new buffer?        */
    if (exit_parm.rxmsq_retc.strptr != retbuffer)
                                       /* free it                           */
      SysReleaseResultMemory(exit_parm.rxmsq_retc.strptr);
    return false;                      /* this was handled                  */
  }
  return true;                         /* not handled                       */
}

bool  RexxActivity::sysExitMsqPsh(
    RexxActivation *activation,        /* sending activation                */
    RexxString *inputstring,           /* string to be placed on the queue  */
    int lifo_flag)                     /* flag to indicate LIFO or FIFO     */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the External Data queue system exit.                           */
/******************************************************************************/
{
  RexxString   *exitname;              /* Exit routine name                 */
  RXMSQPSH_PARM exit_parm;             /* exit parameters                   */

                                       /* get the exit handler              */
  exitname = this->querySysExits(RXMSQ);
  if (exitname != OREF_NULL) {         /* exit enabled?                     */
    if (lifo_flag == QUEUE_LIFO)       /* LIFO queuing requested?           */
                                       /* set the flag appropriately        */
      exit_parm.rxmsq_flags.rxfmlifo = 1;
    else
                                       /* this is a FIFO request            */
      exit_parm.rxmsq_flags.rxfmlifo = 0;
                                       /* make into RXSTRING form           */
    MAKERXSTRING(exit_parm.rxmsq_value, inputstring->getWritableData(), inputstring->getLength());
                                       /* call the handler                  */
    if (SysExitHandler(this, activation, exitname, RXMSQ, RXMSQPSH, (PVOID)&exit_parm, false))
      return true;                     /* this wasn't handled               */

    return false;                      /* this was handled                  */
  }
  return true;                         /* not handled                       */
}

bool  RexxActivity::sysExitMsqSiz(
    RexxActivation *activation,        /* sending activation                */
    RexxInteger **returnsize)          /* returned queue size               */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the External Data queue system exit.                           */
/******************************************************************************/
{
  RexxString   *exitname;              /* Exit routine name                 */
  RXMSQSIZ_PARM exit_parm;             /* exit parameters                   */
  long          tempSize;

                                       /* get the exit handler              */
  exitname = this->querySysExits(RXMSQ);
  if (exitname != OREF_NULL) {         /* exit enabled?                     */
                                       /* call the handler                  */
    if (SysExitHandler(this, activation, exitname, RXMSQ, RXMSQSIZ, (PVOID)&exit_parm, false))
      return true;                     /* this wasn't handled               */
                                       /* Get queue size and return it      */
    tempSize = exit_parm.rxmsq_size;   /* temporary place holder for new_integer */
    *returnsize = (RexxInteger *)new_integer(tempSize);

    return false;                      /* this was handled                  */
  }
  return true;                         /* not handled                       */
}

bool  RexxActivity::sysExitMsqNam(
    RexxActivation *activation,        /* sending activation                */
    RexxString    **inputstring )      /* name of external queue            */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the External Data queue system exit.                           */
/******************************************************************************/
{
  RexxString   *exitname;              /* Exit routine name                 */
  RXMSQNAM_PARM exit_parm;             /* exit parameters                   */
  char          retbuffer[DEFRXSTRING];/* Default result buffer             */

                                       /* get the exit handler              */
  exitname = this->querySysExits(RXMSQ);
  if (exitname != OREF_NULL) {         /* exit enabled?                     */
                                       /* make into RXSTRING form           */
    MAKERXSTRING(exit_parm.rxmsq_name, (*inputstring)->getWritableData(), (*inputstring)->getLength());
                                       /* call the handler                  */
    if (SysExitHandler(this, activation, exitname, RXMSQ, RXMSQNAM, (PVOID)&exit_parm, false))
      return true;                     /* this wasn't handled               */
    *inputstring = (RexxString *)new_string(exit_parm.rxmsq_name.strptr, exit_parm.rxmsq_name.strlength);
                                       /* user give us a new buffer?        */
    if (exit_parm.rxmsq_name.strptr != retbuffer)
                                       /* free it                           */
      SysReleaseResultMemory(exit_parm.rxmsq_name.strptr);
    return false;                      /* this was handled                  */
  }
  return true;                         /* not handled                       */
}

bool RexxActivity::sysExitHltTst(
    RexxActivation *activation)        /* sending activation                */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the Test Halt system exit.                                     */
/******************************************************************************/
{
  RexxString   *exitname;              /* Exit routine name                 */
  RexxString   *condition;             /* Condition(d) information          */
  RXHLTTST_PARM exit_parm;             /* exit parameters                   */

                                       /* get the exit handler              */
  exitname = this->querySysExits(RXHLT);
  if (exitname != OREF_NULL) {         /* exit enabled?                     */
                                       /* Clear halt so as not to be sloppy */
    exit_parm.rxhlt_flags.rxfhhalt = 0;
                                       /* init shvexit return buffer        */
    this->nestedInfo.shvexitvalue = OREF_NULL;
                                       /* call the handler                  */
    if (SysExitHandler(this, activation, exitname, RXHLT, RXHLTTST, (PVOID)&exit_parm, false))
      return true;                     /* this wasn't handled               */
                                       /* Was halt requested?               */
    if (exit_parm.rxhlt_flags.rxfhhalt == 1) {
                                       /* shv_exit return a value?          */
      if (this->nestedInfo.shvexitvalue == OREF_NULL) {
                                       /* return this information           */
        condition = this->nestedInfo.shvexitvalue;
      }
      else condition = OREF_NULL;
                                       /* Then honor the halt request       */
      activation->halt(condition);
    }

    return false;                      /* this was handled                  */
  }
  return true;                         /* not handled                       */
}

bool RexxActivity::sysExitHltClr(
    RexxActivation *activation)        /* sending activation                */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the Clear Halt system exit.                                    */
/******************************************************************************/
{
  RexxString   *exitname;              /* Exit routine name                 */
  RXHLTTST_PARM exit_parm;             /* exit parameters                   */

                                       /* get the exit handler              */
  exitname = this->querySysExits(RXHLT);
  if (exitname != OREF_NULL) {         /* exit enabled?                     */
                                       /* call the handler                  */
    if (SysExitHandler(this, activation, exitname, RXHLT, RXHLTCLR, (PVOID)&exit_parm, false))
      return true;                     /* this wasn't handled               */
    return false;                      /* this was handled                  */
  }
  return true;                         /* not handled                       */
}

bool  RexxActivity::sysExitTrcTst(
     RexxActivation *activation,       /* sending activation                */
     bool currentsetting)              /* sending activation                */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the Test external trace indicator system exit.                 */
/******************************************************************************/
{
  RexxString   *exitname;              /* Exit routine name                 */
  RXTRCTST_PARM exit_parm;             /* exit parameters                   */

                                       /* get the exit handler              */
  exitname = this->querySysExits(RXTRC);
  if (exitname != OREF_NULL) {         /* exit enabled?                     */
                                       /* Clear Trace bit before  call      */
    exit_parm.rxtrc_flags.rxftrace = 0;
                                       /* call the handler                  */
    if (SysExitHandler(this, activation, exitname, RXTRC, RXTRCTST, (PVOID)&exit_parm, false))
      return true;                     /* this wasn't handled               */
                                       /* if not tracing, and now it is     */
                                       /* requsted                          */
    if ((currentsetting == 0) &&  (exit_parm.rxtrc_flags.rxftrace == 1)) {
                                       /* call routine to handle this       */
      activation->externalTraceOn();
      return false;                    /* this was handled                  */
    }
    else                               /* if currently tracing, and now     */
                                       /* requsted to stop                  */
      if ((currentsetting != 0) &&  (exit_parm.rxtrc_flags.rxftrace != 1)) {
                                       /* call routine to handle this       */
        activation->externalTraceOff();
        return false;                  /* this was handled                  */
      }
  }
  return true;                         /* not handled                       */
}


/**
 * Test if the activity is currently running in a context that
 * requires a security manager call.
 *
 * @return true if there is an active security manager, false otherwise.
 */
bool RexxActivity::hasSecurityManager()
{
    return currentActivation->hasSecurityManager();
}


/**
 * Make a call to the current context security manager.
 *
 * @param name   The name of the check operation.
 * @param args   The arguments directory to the call.
 *
 * @return The return result from the call.
 */
bool RexxActivity::callSecurityManager(RexxString *name, RexxDirectory *args)
{
    return currentActivation->callSecurityManager(name, args);
}


void  RexxActivity::traceOutput(       /* write a line of trace information */
      RexxActivation *activation,      /* sending activation                */
      RexxString *line)                /* line to write out                 */
/******************************************************************************/
/* Function:  Write a line of trace output to the .ERROR stream               */
/******************************************************************************/
{
  RexxObject *stream;                  /* target stream                     */

  line = line->stringTrace();          /* get traceable form of this        */
                                       /* if exit declines the call         */
  if (this->sysExitSioTrc(activation, line)) {
                                       /* get the default output stream     */
    stream = ActivityManager::localEnvironment->at(OREF_ERRORNAME);
                                       /* have .local set up yet?           */
    if (stream != OREF_NULL && stream != TheNilObject)
                                       /* do the lineout                    */
      stream->sendMessage(OREF_LINEOUT, line);
    else                               /* use the "real" default stream     */
      this->lineOut(line);
  }
}

void RexxActivity::sayOutput(          /* write a line of say information   */
     RexxActivation *activation,       /* sending activation                */
     RexxString *line)                 /* line to write out                 */
/******************************************************************************/
/* Function:  Write a line of SAY output to the .OUTPUT stream                */
/******************************************************************************/
{
  RexxObject *stream;                  /* target stream                     */

                                       /* if exit declines the call         */
  if (this->sysExitSioSay(activation, line)) {
                                       /* get the default output stream     */
    stream = ActivityManager::localEnvironment->at(OREF_OUTPUT);
                                       /* have .local set up yet?           */
    if (stream != OREF_NULL && stream != TheNilObject)
                                       /* do the lineout                    */
      stream->sendMessage(OREF_SAY, line);
    else                               /* use the "real" default stream     */
      this->lineOut(line);
  }
}

RexxString *RexxActivity::traceInput(  /* read a line of trace input        */
     RexxActivation *activation)       /* sending activation                */
/******************************************************************************/
/* Function:  Read a line for interactive debug input                         */
/******************************************************************************/
{
  RexxString *value;                   /* returned value                    */
  RexxObject *stream;                  /* target stream                     */

                                       /* if exit declines the call         */
  if (this->sysExitSioDtr(activation, &value)) {
                                       /* get the input stream              */
    stream = ActivityManager::localEnvironment->at(OREF_INPUT);
    if (stream != OREF_NULL) {         /* have a stream?                    */
                                       /* read from it                      */
      value = (RexxString *)stream->sendMessage(OREF_LINEIN);
                                       /* get something real?               */
      if (value == (RexxString *)TheNilObject)
        value = OREF_NULLSTRING;       /* just us a null string if not      */
    }
    else
      value = OREF_NULLSTRING;         /* default to a null string          */
  }
  return value;                        /* return value from exit            */
}

RexxString *RexxActivity::pullInput(   /* read a line of pull input         */
     RexxActivation *activation)       /* sending activation                */
/******************************************************************************/
/* Function:  Read a line for the PULL instruction                            */
/******************************************************************************/
{
  RexxString *value;                   /* returned value                    */
  RexxObject *stream;                  /* target stream object              */

                                       /* if exit declines call             */
  if (this->sysExitMsqPll(activation, &value)) {
                                       /* get the external data queue       */
    stream = ActivityManager::localEnvironment->at(OREF_REXXQUEUE);
    if (stream != OREF_NULL) {         /* have a data queue?                */
                                       /* pull from the queue               */
      value = (RexxString *)stream->sendMessage(OREF_PULL);
                                       /* get something real?               */
      if (value == (RexxString *)TheNilObject)
                                       /* read from the linein stream       */
        value = this->lineIn(activation);
    }
  }
  return value;                        /* return the read value             */
}

RexxObject *RexxActivity::lineOut(
    RexxString *line)                  /* line to write out                 */
/******************************************************************************/
/* Function:  Write a line out to the real default I/O stream                 */
/******************************************************************************/
{
  size_t  length;                      /* length to write out               */
  const char *data;                    /* data pointer                      */

  length = line->getLength();          /* get the string length and the     */
  data = line->getStringData();        /* data pointer                      */
  printf("%.*s\n",length, data);       /* print it                          */
  return (RexxObject *)IntegerZero;    /* return on residual count          */
}

RexxString *RexxActivity::lineIn(
    RexxActivation *activation)        /* sending activation                */
/******************************************************************************/
/* Function:  Read a line for the PARSE LINEIN instruction                    */
/******************************************************************************/
{
  RexxObject *stream;                  /* input stream                      */
  RexxString *value;                   /* returned value                    */

                                       /* if exit declines call             */
  if (this->sysExitSioTrd(activation, &value)) {
                                       /* get the input stream              */
    stream = ActivityManager::localEnvironment->at(OREF_INPUT);
    if (stream != OREF_NULL) {         /* have a stream?                    */
                                       /* read from it                      */
      value = (RexxString *)stream->sendMessage(OREF_LINEIN);
                                       /* get something real?               */
      if (value == (RexxString *)TheNilObject)
        value = OREF_NULLSTRING;       /* just use a null string if not     */
    }
    else
      value = OREF_NULLSTRING;         /* default to a null string          */
  }
  return value;                        /* return value from exit            */
}

void RexxActivity::queue(              /* write a line to the queue         */
     RexxActivation *activation,       /* sending activation                */
     RexxString *line,                 /* line to write                     */
     int order)                        /* LIFO or FIFO order                */
/******************************************************************************/
/* Function:  Write a line to the external data queue                         */
/******************************************************************************/
{
  RexxObject *targetQueue;             /* target queue                      */

                                       /* if exit declines call             */
  if (this->sysExitMsqPsh(activation, line, order)) {
                                       /* get the default queue             */
    targetQueue = ActivityManager::localEnvironment->at(OREF_REXXQUEUE);
    if (targetQueue != OREF_NULL) {    /* have a data queue?                */
                                       /* pull from the queue               */
      if (order == QUEUE_LIFO)         /* push instruction?                 */
                                       /* push a line                       */
        targetQueue->sendMessage(OREF_PUSH, (RexxObject *)line);
      else
                                       /* queue a line                      */
        targetQueue->sendMessage(OREF_QUEUENAME, (RexxObject *)line);
     }
   }
}

void  RexxActivity::terminateMethod()
/******************************************************************************/
/* Function: Mark this FREE activity for termination.  Set its exit flag to 1 */
/*   and POST its run semaphore.                                              */
/******************************************************************************/
{
  this->exit = 1;                      /* Activity should exit          */
  EVPOST(this->runsem);                /* let him run so he knows to exi*/
}

void process_message_arguments(
  va_list  *arguments,                 /* variable argument list pointer    */
  const char *interfacedefn,           /* interface definition              */
  RexxList *argument_list )            /* returned list of arguments        */
/******************************************************************************/
/* Function:  Send a message to an object on behalf of an outside agent.      */
/*            Message arguments and return type are described by the          */
/*            interface string.                                               */
/******************************************************************************/
{
  LONG     i;                          /* loop counter/array index          */
  PVOID    tempPointer;                /* temp converted pointer            */
  RXSTRING tempRXSTRING;               /* temp argument rxstring            */
  RexxObject *tempOREF;                /* temp argument object reference    */
  LONG     tempLong;                   /* temp converted long               */
  ULONG    tempULong;                  /* temp converted long               */
  char     tempChar;                   /* temp character value              */
  double   tempDouble;                 /* temp double value                 */
  va_list *subArguments;               /* indirect argument descriptor      */
  const char *subInterface;            /* indirect interface definition     */

  while (*interfacedefn) {             /* process each argument             */
    switch (*interfacedefn++) {        /* process the next argument         */

      case '*':                        /* indirect reference                */
                                       /* get the real interface pointer    */
        subInterface = va_arg(*arguments, const char *);
                                       /* get the indirect pointer          */
        subArguments = va_arg(*arguments, va_list *);
                                       /* go process recursively            */
        process_message_arguments(subArguments, subInterface, argument_list);
        break;

      case 'b':                        /* BYTE                              */
      case 'c':                        /* CHARACTER                         */
                                       /* get the character                 */
        tempChar = (char) va_arg(*arguments, int);
                                       /* create a string object            */
        argument_list->addLast(new_string(&tempChar, 1));
        break;

      case 'i':                        /* int                               */
                                       /* get the number                    */
        tempLong = va_arg(*arguments, int);
                                       /* create an integer object          */
        argument_list->addLast(new_integer(tempLong));
        break;

      case 's':                        /* short                             */
                                       /* get the number                    */
        tempLong = (LONG) (short) va_arg(*arguments, int);
                                       /* create an integer object          */
        argument_list->addLast(new_integer(tempLong));
        break;

      case 'd':                        /* double                            */
      case 'f':                        /* floating point                    */
                                       /* get a double value                */
        tempDouble = va_arg(*arguments, double);
                                       /* convert to string form            */
        argument_list->addLast(new_string(tempDouble));
        break;

      case 'g':                        /* ULONG                             */
                                       /* get the number                    */
        tempULong = va_arg(*arguments, ULONG);
                                       /* create an integer object          */
        argument_list->addLast(new_numberstring((stringsize_t)tempULong));
        break;

      case 'h':                        /* unsigned short                    */
                                       /* get the number                    */
        tempLong = (LONG) (unsigned short) va_arg(*arguments, int);
                                       /* create an integer object          */
        argument_list->addLast(new_integer(tempLong));
        break;

      case 'l':                        /* LONG                              */
                                       /* get the number                    */
        tempLong = va_arg(*arguments, LONG);
                                       /* create an integer object          */
        argument_list->addLast(new_integer(tempLong));
        break;

      case 'o':                        /* REXX object reference             */
                                       /* get the OREF                      */
        tempOREF = va_arg(*arguments, RexxObject *);
                                       /* insert directly into the array    */
        argument_list->addLast(tempOREF);
        break;

      case 'A':                        /* REXX array of objects             */
                                       /* get the OREF                      */
        {
          RexxArray *tempArray;
          tempArray = va_arg(*arguments, RexxArray *);
                                       /* get the array size                */
          tempLong = tempArray->size();
                                       /* for each argument,                */
          for (i = 1; i <= tempLong; i++) {
                                       /* copy into the argument list       */
            argument_list->addLast(tempArray->get(i));
          }
        }
        break;

      case 'r':                        /* RXSTRING                          */
                                       /* get the RXSTRING                  */
        tempRXSTRING = va_arg(*arguments, RXSTRING);
                                       /* create a string object            */
        argument_list->addLast(new_string(tempRXSTRING.strptr, tempRXSTRING.strlength));
        break;

      case 'n':                        /* pointer to somId                  */
      case 'p':                        /* POINTER                           */
      case 't':                        /* Token                             */
      case 'B':                        /* Byte pointer                      */
      case 'C':                        /* Character pointer                 */
      case 'L':                        /* Pointer to LONG                   */
      case 'V':                        /* VOID *?                           */
      case 'R':                        /* RXSTRING *                        */
                                       /* get the pointer                   */
        tempPointer = va_arg(*arguments, void *);
                                       /* create a pointer object           */
        argument_list->addLast(new_pointer(tempPointer));
        break;

      case 'z':                        /* ASCII-Z string                    */
                                       /* get the pointer                   */
        tempPointer = va_arg(*arguments, void *);
                                       /* create a string object            */
        argument_list->addLast(new_string((char *)tempPointer));
        break;
    }
  }
}

void process_message_result(
  RexxObject *value,                   /* returned value                    */
  PVOID    return_pointer,             /* pointer to return value location  */
  char     interfacedefn )             /* interface definition              */
/******************************************************************************/
/* Function:  Convert an OREF return value into the requested message return  */
/*            type.                                                           */
/******************************************************************************/
{
  RexxObject *object_id = (RexxObject*) IntegerZero; /* object SOM id       */

  switch (interfacedefn) {             /* process the return type           */

      case 'b':                        /* BOOLEAN                           */
                                       /* get the number                    */
        (*((bool *)return_pointer)) = value->longValue(NO_LONG) == 0 ? false : true;
        break;
      case 'c':                        /* CHARACTER                         */
                                       /* get the first character           */
        (*((char *)return_pointer)) = ((RexxString *)value)->getChar(0);
        break;

      case 'i':                        /* int                               */
                                       /* get the number                    */
        (*((int *)return_pointer)) = (int)value->longValue(NO_LONG);
        break;

      case 's':                        /* short                             */
                                       /* get the number                    */
        (*((short *)return_pointer)) = (short)value->longValue(NO_LONG);
        break;

      case 'd':                        /* double                            */
      case 'f':                        /* floating point                    */
                                       /* get the double                    */
        (*((double *)return_pointer)) = value->doubleValue();
        break;

      case 'g':                        /* ULONG                             */
                                       /* get the number                    */
        (*((ULONG *)return_pointer)) = (ULONG)value->longValue(NO_LONG);
        break;

      case 'h':                        /* unsigned short                   */
                                       /* get the number                    */
        (*((unsigned short *)return_pointer)) = (unsigned short)value->longValue(NO_LONG);
        break;

      case 'l':                        /* LONG                              */
                                       /* get the number                    */
        (*((ULONG *)return_pointer)) = (ULONG)value->longValue(NO_LONG);
        break;

      case 'o':                        /* REXX object reference             */
                                       /* copy the value directly           */
        (*((RexxObject **)return_pointer)) = value;
        break;

      case 'n':                        /* pointer to somId                  */
      case 'p':                        /* POINTER                           */
      case 't':                        /* Token                             */
      case 'B':                        /* Byte pointer                      */
      case 'C':                        /* Character pointer                 */
      case 'L':                        /* Pointer to LONG                   */
      case 'V':                        /* VOID *?                           */
      case 'R':                        /* RXSTRING *                        */
                                       /* get the pointer value             */
          (*((PVOID *)return_pointer)) = (void *)((RexxInteger *)object_id)->getValue();
        break;

      case 'v':                        /* nothing returned at all           */
        break;
      case 'z':                        /* ASCII-Z string                    */
                                       /* Force to a string.                */
        value = value->stringValue();
        (*((const char **)return_pointer)) = ((RexxString *)value)->getStringData();
        break;
  }
}

int RexxActivity::messageSend(
    RexxObject      *receiver,         /* target object                     */
    RexxString      *msgname,          /* name of the message to process    */
    size_t           count,            /* count of arguments                */
    RexxObject     **arguments,        /* array of arguments                */
    RexxObject     **result )          /* message result                    */
/******************************************************************************/
/* Function:    send a message (with message lookup) to an object.  This      */
/*              method will do any needed activity setup before hand.         */
/******************************************************************************/
{
  int     rc;                          /* message return code               */
  SYSEXCEPTIONBLOCK exreg;             /* system specific exception info    */
  size_t  startDepth;                  /* starting depth of activation stack*/
  NestedActivityState saveInfo;        /* saved activity info               */

  rc = 0;                              /* default to a clean return         */
  *result = OREF_NULL;                 /* default to no return value        */
  this->saveNestedInfo(saveInfo);      /* save critical nesting info        */
                                       /* make sure we have the stack base  */
  this->nestedInfo.stackptr = SysGetThreadStackBase(TOTAL_STACK_SIZE);
  this->clearExits();                  /* make sure the exits are cleared   */
  this->generateRandomNumberSeed();    /* get a fresh random seed           */
                                       /* Push marker onto stack so we know */
  this->pushNil();                     /* what level we entered.            */
  startDepth = this->depth;            /* Remember activation stack depth   */

  SysRegisterSignals(&exreg);          /* register our signal handlers      */

  try
  {
                                       /* issue a straight message send     */
      *result = receiver->messageSend(msgname, count, arguments);

  }
  catch (ActivityException)
  {
      rc = this->error(startDepth);      /* do error cleanup                  */
  }
  // give uninit objects a chance to run
  TheMemoryObject->runUninits();
  this->restoreNestedInfo(saveInfo);   /* now restore to previous nesting   */
  SysDeregisterSignals(&exreg);        /* deregister the signal handlers    */
  this->popNil();                      /* remove the nil marker             */
  return rc;                           /* return the error code             */
}


#include "RexxNativeAPI.h"             /* bring in the external definitions */

int VLAREXXENTRY RexxSendMessage (
  REXXOBJECT  receiver,                /* receiving object                  */
  const char *msgname,                 /* message to send                   */
  REXXOBJECT  start_class,             /* lookup starting class             */
  const char *interfacedefn,           /* argument, return value definition */
  void *result_pointer,                /* pointer to returned result        */
  ... )                                /* variable number of arguments      */
/******************************************************************************/
/* Function:  Send a message to an object on behalf of an outside agent.      */
/*            Message arguments and return type are described by the          */
/*            interface string.                                               */
/******************************************************************************/
{
  RexxActivity *activity;              /* target activity                   */
  RexxObject *result;                  /* returned result object            */
  RexxArray  *argument_array;          /* array of arguments                */
  RexxList   *argument_list;           /* temp list of arguments            */
  char returnType;                     /* type of return value              */
  int  rc;                             /* message return code               */
  va_list arguments;                   /* message argument list             */
  SYSEXCEPTIONBLOCK exreg;             /* system specific exception info    */
  NestedActivityState saveInfo;        /* saved activity info               */
  size_t startDepth;

  rc = 0;                              /* default to a clean return         */
                                       /* Find an activity for this thread  */
                                       /* (will create one if necessary)    */
  activity = ActivityManager::getActivity();
  activity->saveNestedInfo(saveInfo);  /* save the nested stuff             */
  activity->clearExits();              /* make sure the exits are cleared   */
  activity->generateRandomNumberSeed();/* get a fresh random seed           */
                                       /* Push marker onto stack so we know */
  activity->pushNil();                 /* what level we entered.            */
  startDepth = activity->getActivationDepth(); /* Remember activation stack depth   */
  SysRegisterSignals(&exreg);          /* register our signal handlers      */
  try
  {
      returnType = *interfacedefn++;     /* Get the return type.              */
                                         /* get the argument list start       */
      va_start(arguments, result_pointer);
                                         /* create an argument list           */
      argument_list  = new_list();
      ProtectedObject p(argument_list);
                                         /* go convert the arguments          */
      process_message_arguments(&arguments, interfacedefn, argument_list);
                                         /* now convert to an array           */
      argument_array = argument_list->makeArray();
      ProtectedObject p1(argument_array);
      va_end(arguments);                 /* end of argument processing        */
      if (start_class == OREF_NULL)      /* no start scope given?             */
                                         /* issue a straight message send     */
        result = receiver->messageSend(new_string(msgname)->upper(),
            argument_array->size(), argument_array->data());
      else
                                         /* go issue the message with override*/
        result = receiver->messageSend(new_string(msgname)->upper(),
            argument_array->size(), argument_array->data(), start_class);
      // TODO fix this usage up
      if (result != OREF_NULL) {         /* if we got a result, protect it.   */
        saveObject(result);              /* because might not have references */
                                         /* convert the return result         */
        process_message_result(result, result_pointer, returnType);
      }
  }
  catch (ActivityException)
  {
                                       /* do error cleanup                  */
      rc = activity->error(startDepth);
      result = OREF_NULL;                /* no result in this case            */
  }

  memoryObject.runUninits();           /* be sure to finish UNINIT methods  */
                                       /* restore the nested information    */
  activity->restoreNestedInfo(saveInfo);
  SysDeregisterSignals(&exreg);        /* deregister the signal handlers    */
  activity->popNil();                  /* remove the nil marker             */
                                       /* release our activity usage        */
  ActivityManager::returnActivity(activity);
  return rc;                           /* return the error code             */
}


RexxObject *RexxActivity::nativeRelease(
    RexxObject *result )               /* potential return value            */
/***************************************************************/
/* Function:  Release kernel access, providing locking on      */
/*            the return value, if any                         */
/***************************************************************/
{
  RexxNativeActivation *activation;    /* current activation                */

  if (result != OREF_NULL)             /* only save real references!        */
  {
      activation = (RexxNativeActivation *)ActivityManager::currentActivity->current();
      result = activation->saveObject(result);
  }
  ActivityManager::returnActivity();   /* release the kernel lock           */
  return result;                       /* return the result object          */
}


REXXOBJECT REXXENTRY RexxDispatch (
  REXXOBJECT argList)                  /* ArgLIst array.                    */
/******************************************************************************/
/* Function:  Dispatch message to rexx object.  All args are in a Rexx Array  */
/******************************************************************************/
{
  RexxObject *receiver;                /* receiver of message               */
  RexxString *message;                 /* message name to send to receiver  */
  RexxArray  *args;                    /* argument array for message        */
  RexxObject *result;                  /* returned result object            */

  ActivityManager::lockKernel();
  receiver = ((RexxArray *)argList)->get(1);
  message  = (RexxString *)((RexxArray *)argList)->get(2);
  args     = (RexxArray *)((RexxArray *)argList)->get(3);
  ActivityManager::unlockKernel();
                                       /* process the message               */
  RexxSendMessage(receiver, message->getStringData(), OREF_NULL, "oA", &result, (RexxObject *)args);
  return result;                       /* return the message result         */
}
