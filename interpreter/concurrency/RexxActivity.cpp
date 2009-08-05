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
/* REXX Kernel                                                                */
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
#include "PointerClass.hpp"
#include "InterpreterInstance.hpp"
#include "ActivityDispatcher.hpp"
#include "MessageDispatcher.hpp"
#include "Interpreter.hpp"
#include "PackageClass.hpp"
#include "SystemInterpreter.hpp"
#include "ActivationFrame.hpp"
#include "StackFrameClass.hpp"

const size_t ACT_STACK_SIZE = 20;

/**
 * The main entry point for spawned activities.
 */
void RexxActivity::runThread()
{
    bool firstDispatch = true;           // some things only occur on subsequent requests
                                         /* establish the stack base pointer  */
    this->stackBase = currentThread.getStackBase(TOTAL_STACK_SIZE);

    for (;;)
    {
        // save the actitivation level in case there's an error unwind for an unhandled
        // exception;
        size_t activityLevel = 0;

        try
        {
            this->runsem.wait();             /* wait for run permission           */
            if (this->exit)                  /* told to exit?                     */
            {
                break;                       /* we're out of here                 */
            }
            this->requestAccess();           /* now get the kernel lock           */
            // we're already marked as active when first called to keep us from
            // getting terminated prematurely before we get a chance to run
            if (!firstDispatch)
            {
                this->activate();                // make sure this is marked as active
            }
            firstDispatch = false;               // we need to activate every time after this
            activityLevel = getActivationLevel();

            // if we have a dispatch message set, send it the send message to kick everything off
            if (dispatchMessage != OREF_NULL)
            {
                MessageDispatcher dispatcher(dispatchMessage);
                run(dispatcher);
            }
            else
            {

                // this is a reply message start, just dispatch the Rexx code
                this->topStackFrame->dispatch();
            }
        }
        catch (ActivityException)    // we've had a termination event, raise an error
        {
            // it's possible that we might not have the kernel lock when
            // control returns to here.  Make sure we have it.
            if (ActivityManager::currentActivity != this)
            {
                this->requestAccess();
            }
            this->error();
        }

        // make sure we get restored to the same base activation level.
        restoreActivationLevel(activityLevel);
        memoryObject.runUninits();         /* run any needed UNINIT methods now */

        this->deactivate();                // no longer an active activity

        dispatchMessage = OREF_NULL;       // we're done with the message object

        runsem.reset();                    /* reset the run semaphore and the   */
        guardsem.reset();                  /* guard semaphore                   */

        // try to pool this.  If the ActivityManager doesn't take, we go into termination mode
        if (!instance->poolActivity(this))
        {
            this->releaseAccess();
            break;
        }
        // release the kernel lock and go wait for more work
        this->releaseAccess();
    }
    // tell the activity manager we're going away
    ActivityManager::activityEnded(this);
    return;                              /* finished                          */
}


/**
 * Do cleanup of activity resources when an activity is completely
 * shutdown and discarded.
 */
void RexxActivity::cleanupActivityResources()
{
    runsem.close();
    guardsem.close();
    currentThread.close();
}


/**
 * We're leaving the current thread.  So we need to deactivate
 * this.
 */
void RexxActivity::exitCurrentThread()
{
    // deactivate the nesting level
    deactivate();
    // if we're inactive, try to run any pending uninits
    if (isInactive())
    {
        memoryObject.runUninits();
    }
    // this activity owned the kernel semaphore before entering here...release it
    // now.
    releaseAccess();
}

/**
 * Enter the current thread for an API call.
 */
void RexxActivity::enterCurrentThread()
{
    /* Activity already existed for this */
    /* get kernel semophore in activity  */
    requestAccess();
    activate();        // let the activity know it's in use, potentially nested
    // belt-and-braces.  Make sure the current activity is explicitly set to
    // this activity before leaving.
    ActivityManager::currentActivity = this;
}


void *RexxActivity::operator new(size_t size)
/******************************************************************************/
/* Function:  Create a new activity                                           */
/******************************************************************************/
{
                                       /* get the new activity storage      */
   return new_object(size, T_Activity);
}


/**
 * Initialize an Activity object.
 *
 * @param createThread
 *               Indicates whether we're going to be running on the
 *               current thread, or creating a new thread.
 */
RexxActivity::RexxActivity(bool createThread)
{
    this->clearObject();               /* globally clear the object         */
                                       /* create an activation stack        */
    this->activations = new_internalstack(ACT_STACK_SIZE);
    this->frameStack.init();           /* initialize the frame stack        */
    this->runsem.create();             /* create the run and                */
    this->guardsem.create();           /* guard semaphores                  */
    this->activationStackSize = ACT_STACK_SIZE;  /* set the activation stack size     */
    this->stackcheck = true;           /* start with stack checking enabled */
                                       /* use default settings set          */
                                       /* set up current usage set          */
    this->numericSettings = Numerics::getDefaultSettings();
    this->generateRandomNumberSeed();  /* get a fresh random seed           */
                                       /* Create table for progream being   */
    this->requiresTable = new_table(); /*installed vial ::REQUIRES          */
    // create a stack frame for this running context
    createNewActivationStack();

    if (createThread)                    /* need to create a thread?          */
    {
        runsem.reset();                  /* set the run semaphore             */
        // we need to enter this thread already marked as active, otherwise
        // the main thread might shut us down before we get a chance to perform
        // whatever function we're getting asked to run.
        activate();
                                         /* create a thread                   */
        currentThread.create(this, C_STACK_SIZE);
    }
    else                               /* thread already exists             */
    {
        // run on the current thread
        currentThread.useCurrentThread();
                                         /* establish the stack base pointer  */
        this->stackBase = currentThread.getStackBase(TOTAL_STACK_SIZE);
    }
}


/**
 * Initialize an Activity object that's being recycled for
 * another use.
 */
void RexxActivity::reset()
{
                                       /* Make sure any left over           */
                                       /* ::REQUIRES is cleared out.        */
    this->resetRunningRequires();
}


/**
 * Create a new activity for processing a method reply
 * instruction.
 *
 * @return The newly created activity.
 */
RexxActivity *RexxActivity::spawnReply()
{
    // recreate a new activiy in the same instance
    return instance->spawnActivity(this);
}

void RexxActivity::generateRandomNumberSeed()
/******************************************************************************/
/* Function:  Generate a fresh random number seed.                            */
/******************************************************************************/
{
    // a good random value for a starting point would be the address of the
    // activity.
    uint64_t rnd = (uint64_t)(uintptr_t)this;
    // flip the bits to generate a little more noise.  This value is
    // largely to ensure that the timestamp value doesn't produce similar
    // seeds because of low timer resolution.
    rnd = ~rnd;

    RexxDateTime  timestamp;             /* current timestamp                 */
    SystemInterpreter::getCurrentTime(&timestamp);       /* get a fresh time stamp            */
                                         /* take the seed from the time       */
    randomSeed = rnd + timestamp.getBaseTime();
    for (int i = 0; i < 13; i++)
    {           /* randomize the seed number a bit   */
                /* scramble the seed a bit           */
        randomSeed = RANDOMIZE(randomSeed);
    }
}


wholenumber_t RexxActivity::error()
/******************************************************************************/
/* Function:  Force error termination on an activity, returning the resulting */
/*            REXX error code.                                                */
/******************************************************************************/
{
    // unwind to a base activation
    while (!topStackFrame->isStackBase())
    {
        // if we're not to the stack very base of the stack, terminate the frame
        this->topStackFrame->termination();
        this->popStackFrame(false);
    }

    wholenumber_t rc = Error_Interpretation/1000;      /* set default return code           */
    /* did we get a condtion object?     */
    if (this->conditionobj != OREF_NULL)
    {
        /* force it to display               */
        this->display(this->conditionobj);
        // try to convert.  Leaves unchanged if not value
        this->conditionobj->at(OREF_RC)->numberValue(rc);
    }
    return rc;                           /* return the error code             */
}


wholenumber_t RexxActivity::error(RexxActivationBase *activation, RexxDirectory *errorInfo)
/******************************************************************************/
/* Function:  Force error termination on an activity, returning the resulting */
/*            REXX error code.                                                */
/******************************************************************************/
{
    // if not passed an explicit error object, use whatever we have in our
    // local holder.
    if (errorInfo == OREF_NULL)
    {
        errorInfo = this->conditionobj;
    }

    // unwind to a base activation
    while (topStackFrame != activation)
    {
        // if we're not to the stack very base of the stack, terminate the frame
        this->topStackFrame->termination();
        this->popStackFrame(false);
    }

    wholenumber_t rc = Error_Interpretation/1000;      /* set default return code           */
    /* did we get a condtion object?     */
    if (errorInfo != OREF_NULL)
    {
        /* force it to display               */
        this->display(errorInfo);
        // try to convert.  Leaves unchanged if not value
        errorInfo->at(OREF_RC)->numberValue(rc);
    }
    return rc;                           /* return the error code             */
}


/**
 * Extract an error number from a syntax condition object.
 *
 * @param conditionObject
 *               The condition object for the extract.
 *
 * @return The RC value associated with the condition.
 */
wholenumber_t RexxActivity::errorNumber(RexxDirectory *conditionObject)
{
    wholenumber_t rc = Error_Interpretation/1000;      /* set default return code           */
    /* did we get a condtion object?     */
    if (conditionObject != OREF_NULL)
    {
        // try to convert.  Leaves unchanged if not value
        conditionObject->at(OREF_RC)->numberValue(rc);
    }
    return rc;                           /* return the error code             */
}


/**
 * Raise a condition, with potential trapping.
 *
 * @param condition  The condition name.
 * @param rc         The rc value
 * @param description
 *                   The description value.
 * @param additional the exception additional information.
 * @param result     The condition result info.
 *
 * @return true if this was trapped via CALL ON, false for untrapped
 *         conditions.
 */
bool RexxActivity::raiseCondition(RexxString *condition, RexxObject *rc, RexxString *description, RexxObject *additional, RexxObject *result)
{
    // just create a condition object and process the traps.
    RexxDirectory *conditionObj = createConditionObject(condition, rc, description, additional, result);
    return raiseCondition(conditionObj);
}

/**
 * Process condition trapping for a condition or syntax
 * error.
 *
 * @param conditionObj
 *               The condition object that describes the condition.
 *
 * @return true if this was trapped, false otherwise.  If trapped
 *         via a SIGNAL ON, this will NOT return to here.
 */
bool RexxActivity::raiseCondition(RexxDirectory *conditionObj)
{
    bool handled = false;                     /* condition not handled yet         */
    RexxString *condition = (RexxString *)conditionObj->at(OREF_CONDITION);

    /* invoke the error traps, on all    */
    /*  nativeacts until reach 1st       */
    /*  also give 1st activation a shot. */
    for (RexxActivationBase *activation = this->getTopStackFrame() ; !activation->isStackBase(); activation = activation->getPreviousStackFrame())
    {
        handled = activation->trap(condition, conditionObj);
        if (isOfClass(Activation, activation)) /* reached our 1st activation yet.   */
        {
            break;                           /* yes, break out of loop            */
        }
    }

    /* Control will not return here if the condition was trapped via*/
    /* SIGNAL ON SYNTAX.  For CALL ON conditions, handled will be   */
    /* true if a trap is pending.                                   */

    return handled;                      /* this has been handled             */
}


/**
 * Create a condition object from the provided information.
 *
 * @param condition  The name of the raised condition.
 * @param rc         The rc value (can be null)
 * @param description
 *                   The description string.
 * @param additional Additional information.
 * @param result     result information.
 *
 * @return The constructed condition object (a directory).
 */
RexxDirectory *RexxActivity::createConditionObject(RexxString *condition, RexxObject *rc, RexxString *description, RexxObject *additional, RexxObject *result)
{
    // condition objects are directories
    RexxDirectory *conditionObj = new_directory();
                                       /* put in the condition name         */
    conditionObj->put(condition, OREF_CONDITION);
                                       /* fill in default description       */
    conditionObj->put(description == OREF_NULL ? OREF_NULLSTRING : description, OREF_DESCRIPTION);
                                       /* fill in the propagation status    */
    conditionObj->put(TheFalseObject, OREF_PROPAGATED);
    if (rc != OREF_NULL)                 /* have an RC value?                 */
    {
        conditionObj->put(rc, OREF_RC);    /* add to the condition argument     */
    }
    if (additional != OREF_NULL)         /* or additional information         */
    {
        conditionObj->put(additional, OREF_ADDITIONAL);
    }
    if (result != OREF_NULL)             /* given a return result?            */
    {
        conditionObj->put(result, OREF_RESULT);
    }
    return conditionObj;
}

void RexxActivity::reportAnException(
    wholenumber_t errcode )            /* REXX error code                   */
/******************************************************************************/
/* Function:  Forward on an exception condition                               */
/******************************************************************************/
{
                                       /* send along with nothing           */
  this->raiseException(errcode, OREF_NULL, OREF_NULL, OREF_NULL);
}

void RexxActivity::reportAnException(
    wholenumber_t errcode,             /* REXX error code                   */
    RexxObject *substitution1 )        /* substitution information          */
/******************************************************************************/
/* Function:  Forward on an exception condition                               */
/******************************************************************************/
{
  this->raiseException(errcode, OREF_NULL, new_array(substitution1), OREF_NULL);
}

void RexxActivity::reportAnException(
    wholenumber_t errcode,             /* REXX error code                   */
    RexxObject *substitution1,         /* substitution information          */
    RexxObject *substitution2 )        /* substitution information          */
/******************************************************************************/
/* Function:  Forward on an exception condition                               */
/******************************************************************************/
{
  this->raiseException(errcode, OREF_NULL, new_array(substitution1, substitution2), OREF_NULL);
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
  this->raiseException(errcode, OREF_NULL, new_array(substitution1, substitution2, substitution3), OREF_NULL);
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
  this->raiseException(errcode, OREF_NULL, new_array(substitution1, substitution2, substitution3, substitution4), OREF_NULL);
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
    this->raiseException(errcode, OREF_NULL, new_array(new_string(substitution1), substitution2, new_string(substitution3), substitution4), OREF_NULL);
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
    const char *string1,
    const char *string2)
/******************************************************************************/
/* Function:  Raise an error using a single REXX character string only        */
/*            as a substitution parameter                                     */
/******************************************************************************/
{
                                       /* convert and forward along         */
  this->reportAnException(errcode, new_string(string1), new_string(string2));
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


/**
 * Raise an exception on the current activity.
 *
 * @param errcode    The syntax error code.
 * @param description
 *                   The associated description string.
 * @param additional The message substitution parameters.
 * @param result     The message result.
 */
void RexxActivity::raiseException(wholenumber_t  errcode, RexxString *description, RexxArray *additional, RexxObject *result)
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
    // during error processing, we need to request the string value of message
    // substitution objects.  It is possible that the string process will also
    // cause a syntax error, resulting in a recursion loop.  We snip that off here,
    // by disallowing a nested error condition.
    if (requestingString)
    {
        throw RecursiveStringError;
    }

    RexxActivationBase *topFrame = this->getTopStackFrame();

    RexxActivation *activation = this->getCurrentRexxFrame(); /* get the current activation        */
    // if we're raised within a real Rexx context, we need to deal with forwarded
    // frames
    if (topFrame == activation)
    {
        // unwind the stack until we find
        while (activation != OREF_NULL && activation->isForwarded())
        {
            // terminate and remove this stack frame
            popStackFrame(activation);
            // grab the new current frame
            activation = this->getCurrentRexxFrame();
        }
    }
    else {
        activation = NULL;      // raised from a native context, don't add to stack trace
    }

    this->conditionobj = createExceptionObject(errcode, description, additional, result);

    /* process as common condition       */
    if (!this->raiseCondition(conditionobj))
    {
        /* fill in the propagation status    */
        conditionobj->put(TheTrueObject, OREF_PROPAGATED);
        // if we have an Rexx context to work with, unwind to that point,
        if (activation != OREF_NULL)
        {
            // unwind the frame to this point
            unwindToFrame(activation);
            popStackFrame(activation);     // remove it from the stack
        }
        this->raisePropagate(conditionobj);  /* pass on down the chain            */
    }
}


/**
 * Create a new error exception object.
 *
 * @param errcode    The error code to raise.
 * @param description
 *                   The description string.
 * @param additional Message substitution information.
 * @param result     The result object.
 *
 * @return The created exception dictionary.
 */
RexxDirectory *RexxActivity::createExceptionObject(
    wholenumber_t  errcode,            /* REXX error code                   */
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

    /* All error detection done. we can  */
    /*  build and save exobj now.        */
    /* get an exception directory        */
    RexxDirectory *exobj = (RexxDirectory *)new_directory();
    // this is the anchor for anything else
    ProtectedObject p(exobj);

    wholenumber_t primary = (errcode / 1000) * 1000;   /* get the primary message number    */

    char work[32];
                                         /* format the number (string) into   */
                                         /*  work buffer.                     */
    sprintf(work,"%d.%1d", errcode/1000, errcode - primary);
    RexxString *code = new_string(work); /* get the formatted code            */
    exobj->put(code, OREF_CODE);

    wholenumber_t newVal = primary/1000;
    RexxInteger *rc = new_integer(newVal); /* get the primary message number    */
    exobj->put(rc, OREF_RC);             /* add the return code               */
                                         /* get the primary message text      */
    RexxString *errortext = SystemInterpreter::getMessageText(primary);
    if (errortext == OREF_NULL)          /* no corresponding message          */
    {
        /* this is an error                  */
        reportException(Error_Execution_error_condition, code);
    }
    exobj->put(errortext, OREF_ERRORTEXT);

    // handle the message substitution values (raw form)
    if (additional == OREF_NULL)
    {
        /* use a zero size array             */
        additional = new_array((size_t)0);
    }
    // add this in
    exobj->put(additional, OREF_ADDITIONAL);

    if (primary != errcode)              /* have a secondary message to issue?*/
    {
        /* retrieve the secondary message    */
        RexxString *message = SystemInterpreter::getMessageText(errcode);
        if (message == OREF_NULL)          /* no corresponding message          */
        {
            /* this is an error                  */
            reportException(Error_Execution_error_condition, code);
        }
        /* do required substitutions         */
        message = messageSubstitution(message, additional);
        /* replace the original message text */
        exobj->put(message, OREF_NAME_MESSAGE);
    }
    else
    {
        /* don't give a secondary message    */
                                         /* fill in the decimal error code    */
        exobj->put(TheNilObject, OREF_NAME_MESSAGE);
    }

    // the description string (rare for exceptions)
    if (description == OREF_NULL)        /* no description?                   */
    {
        // use an explicit null string
        exobj->put(OREF_NULLSTRING, OREF_DESCRIPTION);
    }
    else
    {
        exobj->put(description, OREF_DESCRIPTION);
    }

    if (result != OREF_NULL)
    {
        exobj->put(result, OREF_RESULT);
    }

    // create lists for both the stack frames and the traceback lines
    RexxList *stackFrames = new_list();
                                         /* add to the exception object       */
    exobj->put(stackFrames, OREF_STACKFRAMES);

    RexxList *traceback = new_list();    /* create a traceback list           */
                                         /* add to the exception object       */
    exobj->put(traceback, OREF_TRACEBACK);

    ActivationFrame *frame = activationFrames;
    while (frame != OREF_NULL && frame->getSource() == OREF_NULL)
    {
        frame = frame->next;
    }

    RexxSource *source = OREF_NULL;

    // if we have a frame, then process the list
    if (frame != NULL)
    {
        StackFrameClass *firstFrame = frame->createStackFrame();
        // save the source object associated with that frame
        source = frame->getSource();
        stackFrames->append(firstFrame);
        traceback->append(firstFrame->getTraceLine());

        // step to the next frame
        frame = frame->next;
        while (frame != NULL)
        {
            StackFrameClass *stackFrame = frame->createStackFrame();
            stackFrames->append(stackFrame);
            traceback->append(stackFrame->getTraceLine());
            frame = frame->next;
        }

        RexxObject *lineNumber = firstFrame->getLine();
        if (lineNumber != TheNilObject)
        {
            // add the line number information
            exobj->put(lineNumber, OREF_POSITION);
        }
    }

    if (source != OREF_NULL)             /* have source for this?             */
    {
        exobj->put(source->getProgramName(), OREF_PROGRAM);
        exobj->put(source->getPackage(), OREF_PACKAGE);
    }
    else
    {
        // if not available, then this is explicitly a NULLSTRINg.
        exobj->put(OREF_NULLSTRING, OREF_PROGRAM);
    }

    // the condition name is always SYNTAX
    exobj->put(OREF_SYNTAX, OREF_CONDITION);
    /* fill in the propagation status    */
    exobj->put(TheFalseObject, OREF_PROPAGATED);

    return exobj;
}


RexxString *RexxActivity::messageSubstitution(
    RexxString *message,               /* REXX error message                */
    RexxArray  *additional )           /* substitution information          */
/******************************************************************************/
/* Function:  Perform any required message substitutions on the secondary     */
/*            error message.                                                  */
/******************************************************************************/
{
    size_t substitutions = additional->size();  /* get the substitution count        */
    RexxString *newmessage = OREF_NULLSTRING;        /* start with a null string          */
                                         /* loop through and substitute values*/
    for (size_t i = 1; i <= substitutions; i++)
    {
        /* search for a substitution         */
        size_t subposition = message->pos(OREF_AND, 0);
        if (subposition == 0)              /* not found?                        */
        {
            break;                           /* get outta here...                 */
        }
                                             /* get the leading part              */
        RexxString *front = message->extract(0, subposition - 1);
        /* pull off the remainder            */
        RexxString *back = message->extract(subposition + 1, message->getLength() - (subposition + 1));
        /* get the descriptor position       */
        size_t selector = message->getChar(subposition);
        /* not a good number?                */
        RexxString *stringVal = OREF_NULLSTRING;
        if (selector < '0' || selector > '9')
        {
            /* use a default message             */
            stringVal = new_string("<BAD MESSAGE>"); /* must be stringValue, not value, otherwise trap */
        }
        else
        {
            selector -= '0';                 /* convert to a number               */
            // still in range?
            if (selector <= substitutions)    /* out of our range?                 */
            {
                RexxObject *value = additional->get(selector);
                if (value != OREF_NULL)      /* have a value?                     */
                {
                    /* set the reentry flag              */
                    this->requestingString = true;
                    this->stackcheck = false;    /* disable the checking              */
                    // save the actitivation level in case there's an error unwind for an unhandled
                    // exception;
                    size_t activityLevel = getActivationLevel();
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

                    // make sure we get restored to the same base activation level.
                    restoreActivationLevel(activityLevel);
                    /* we're safe again                  */
                    this->requestingString = false;
                    this->stackcheck = true;     /* disable the checking              */
                }
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

/**
 * Reraise an exception object in a prior context.
 *
 * @param exobj  The exception object with the syntax information.
 */
void RexxActivity::reraiseException(RexxDirectory *exobj)
{
    RexxActivation *activation = this->getCurrentRexxFrame();/* get the current activation        */
    /* have a target activation?         */
    if (activation != OREF_NULL)
    {
        RexxSource *source = activation->getSourceObject();
        // set the position and program name
        exobj->put(new_integer(activation->currentLine()), OREF_POSITION);
        exobj->put(source->getProgramName(), OREF_PROGRAM);
        exobj->put(source->getPackage(), OREF_PACKAGE);
    }
    else
    {
        exobj->remove(OREF_POSITION);      /* remove the position               */
        exobj->remove(OREF_PROGRAM);       /* remove the program name           */
        exobj->remove(OREF_PACKAGE);       /* remove the program name           */
    }

    RexxObject *errorcode = exobj->at(OREF_CODE);    /* get the error code                */
                                         /* convert to a decimal              */
    wholenumber_t errornumber = Interpreter::messageNumber((RexxString *)errorcode);
    /* get the primary message number    */
    wholenumber_t primary = (errornumber / 1000) * 1000;
    if (errornumber != primary)
    {        /* have an actual secondary message? */
        char            work[10];            /* temp buffer for formatting        */
             /* format the number (string) into   */
             /*  work buffer.                     */
        sprintf(work,"%1d%3.3d", errornumber/1000, errornumber - primary);
        errornumber = atol(work);          /* convert to a long value           */
                                           /* retrieve the secondary message    */
        RexxString *message = SystemInterpreter::getMessageText(errornumber);
        /* Retrieve any additional parameters*/
        RexxArray *additional = (RexxArray *)exobj->at(OREF_ADDITIONAL);
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
                                         /* get the condition                 */
    RexxString *condition = (RexxString *)conditionObj->at(OREF_CONDITION);
    RexxActivationBase *activation = getTopStackFrame(); /* get the current activation        */

    /* loop to the top of the stack      */
    while (activation != OREF_NULL)
    {
        /* give this one a chance to trap    */
        /* (will never return for trapped    */
        /* PROPAGATE conditions)             */
        activation->trap(condition, conditionObj);
        /* this is a propagated condition    */
        conditionObj->put(TheTrueObject, OREF_PROPAGATED);
        // if we've unwound to the stack base and not been trapped, we need
        // to fall through to the kill processing.  The stackbase should have trapped
        // this.  We never cleanup the stackbase activation though.
        if (activation->isStackBase())
        {
            break;
        }
        // clean up this stack frame
        popStackFrame(activation);
        activation = this->getTopStackFrame(); /* get the sender's sender           */
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
    /* get the traceback info            */
    RexxList *trace_backList = (RexxList *)exobj->at(OREF_TRACEBACK);
    if (trace_backList != OREF_NULL)     /* have a traceback?                 */
    {
        /* convert to an array               */
        RexxArray *trace_back = trace_backList->makeArray();
        ProtectedObject p(trace_back);
        /* get the traceback size            */
        size_t tracebackSize = trace_back->size();

        for (size_t i = 1; i <= tracebackSize; i++)     /* loop through the traceback starttrc */
        {
            RexxString *text = (RexxString *)trace_back->get(i);
            /* have a real line?                 */
            if (text != OREF_NULL && text != TheNilObject)
            {
                /* write out the line                */
                this->traceOutput(currentRexxFrame, text);
            }
        }
    }
    RexxObject *rc = exobj->at(OREF_RC);             /* get the error code                */
    /* get the message number            */
    wholenumber_t errorCode = Interpreter::messageNumber((RexxString *)rc);
    /* get the header                    */
    RexxString *text = SystemInterpreter::getMessageHeader(errorCode);
    if (text == OREF_NULL)               /* no header available?              */
    {
        /* get the leading part              */
        text = SystemInterpreter::getMessageText(Message_Translations_error);
    }
    else                                 /* add to the message text           */
    {
        text = text->concat(SystemInterpreter::getMessageText(Message_Translations_error));
    }
    /* get the name of the program       */
    RexxString *programname = (RexxString *)exobj->at(OREF_PROGRAM);
    /* add on the error number           */
    text = text->concatWith(REQUEST_STRING(rc), ' ');
    /* if program exists, add the name   */
    /* of the program to the message     */
    if (programname != OREF_NULL && programname != OREF_NULLSTRING)
    {
        /* add on the "running" part         */
        text = text->concatWith(SystemInterpreter::getMessageText(Message_Translations_running), ' ');
        /* add on the program name           */
        text = text->concatWith(programname, ' ');
        /* Get the position/Line number info */
        RexxObject *position = exobj->at(OREF_POSITION);
        if (position != OREF_NULL)         /* Do we have position/Line no info? */
        {
            /* Yes, add on the "line" part       */
            text = text->concatWith(SystemInterpreter::getMessageText(Message_Translations_line), ' ');
            /* add on the line number            */
            text = text->concatWith(REQUEST_STRING(position), ' ');
            /* add on the ":  "                  */
        }
    }
    text = text->concatWithCstring(":  ");
    /* and finally the error message     */
    text = text->concat((RexxString *)exobj->at(OREF_ERRORTEXT));
    /* write out the line                */
    this->traceOutput(currentRexxFrame, text);
    /* get the secondary message         */
    RexxString *secondary = (RexxString *)exobj->at(OREF_NAME_MESSAGE);
    /* have a real message?              */
    if (secondary != OREF_NULL && secondary != (RexxString *)TheNilObject)
    {
        rc = exobj->at(OREF_CODE);         /* get the error code                */
                                           /* get the message number            */
        errorCode = Interpreter::messageNumber((RexxString *)rc);
        /* get the header                    */
        text = SystemInterpreter::getMessageHeader(errorCode);
        if (text == OREF_NULL)             /* no header available?              */
        {
            /* get the leading part              */
            text = SystemInterpreter::getMessageText(Message_Translations_error);
        }
        else                               /* add to the message text           */
        {
            text = text->concat(SystemInterpreter::getMessageText(Message_Translations_error));
        }
        /* add on the error number           */
        text = text->concatWith((RexxString *)rc, ' ');
        /* add on the ":  "                  */
        text = text->concatWithCstring(":  ");
        /* and finally the error message     */
        text = text->concat(secondary);
        /* write out the line                */
        this->traceOutput(currentRexxFrame, text);
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
  text = SystemInterpreter::getMessageText(Message_Translations_debug_error);
                                       /* add on the error number           */
  text = text->concatWith(REQUEST_STRING(exobj->at(OREF_RC)), ' ');
                                       /* add on the ":  "                  */
  text = text->concatWithCstring(":  ");
                                       /* and finally the error message     */
  text = text->concatWith((RexxString *)exobj->at(OREF_ERRORTEXT), ' ');
                                       /* write out the line                */
  this->traceOutput(currentRexxFrame, text);
                                       /* get the secondary message         */
  secondary = (RexxString *)exobj->at(OREF_NAME_MESSAGE);
                                       /* have a real message?              */
  if (secondary != OREF_NULL && secondary != (RexxString *)TheNilObject) {
                                       /* get the leading part              */
    text = SystemInterpreter::getMessageText(Message_Translations_debug_error);
                                       /* add on the error number           */
    text = text->concatWith((RexxString *)exobj->at(OREF_CODE), ' ');
                                       /* add on the ":  "                  */
    text = text->concatWithCstring(":  ");
                                       /* and finally the error message     */
    text = text->concat(secondary);
                                       /* write out the line                */
    this->traceOutput(getCurrentRexxFrame(), text);
  }
  return TheNilObject;                 /* just return .nil                  */
}

void RexxActivity::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  memory_mark(this->activations);
  memory_mark(this->topStackFrame);
  memory_mark(this->currentRexxFrame);
  memory_mark(this->conditionobj);
  memory_mark(this->requiresTable);
  memory_mark(this->nextWaitingActivity);
  memory_mark(this->waitingObject);
  memory_mark(this->dispatchMessage);

  /* have the frame stack do its own marking. */
  frameStack.live(liveMark);
  // mark any protected objects we've been watching over

  ProtectedObject *p = protectedObjects;
  while (p != NULL)
  {
      memory_mark(p->protectedObject);
      p = p->next;
  }
}
void RexxActivity::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  memory_mark_general(this->activations);
  memory_mark_general(this->topStackFrame);
  memory_mark_general(this->currentRexxFrame);
  memory_mark_general(this->conditionobj);
  memory_mark_general(this->requiresTable);
  memory_mark_general(this->nextWaitingActivity);
  memory_mark_general(this->waitingObject);
  memory_mark_general(this->dispatchMessage);

  /* have the frame stack do its own marking. */
  frameStack.liveGeneral(reason);

  ProtectedObject *p = protectedObjects;
  while (p != NULL)
  {
      memory_mark_general(p->protectedObject);
      p = p->next;
  }
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
    guardsem.post();                     /* and the guard semaphore           */
    runsem.post();                       /* post the run semaphore            */
    SysActivity::yield();                /* yield the thread                  */
}



/**
 * Run a message object on a spawned thread.
 *
 * @param target The target message object.
 */
void RexxActivity::run(RexxMessage *target)
{
    dispatchMessage = target;

    guardsem.post();                     /* and the guard semaphore           */
    runsem.post();                       /* post the run semaphore            */
    SysActivity::yield();                /* yield the thread                  */
}


/**
 * Check the activation stack to see if we need to expand the size.
 */
void RexxActivity::checkActivationStack()
{
    // no room for a new stack frame?  need to expand the stack
    if (this->stackFrameDepth == this->activationStackSize)
    {
        // allocate a larger stack
        RexxInternalStack *newstack = new_internalstack(this->activationStackSize + ACT_STACK_SIZE);
        // now copy all of the entries over to the new frame stack
        for (size_t i = this->activationStackSize; i != 0; i--)
        {
             newstack->push(this->activations->peek(i-1));
        }
        // update the frame information
        this->activations = newstack;
        this->activationStackSize += ACT_STACK_SIZE;
    }
}


/**
 * Update the top of the stack markers after a push or a pop
 * operation on the stack frame.
 */
void RexxActivity::updateFrameMarkers()
{
    // we have a new top entry...get this from the stack and adjust
    // the markers appropriately
    topStackFrame = (RexxActivationBase *)activations->getTop();
    // the new activation is the new top and there may or may not be
    // a rexx context to deal with
    currentRexxFrame = topStackFrame->findRexxContext(); ;

    // update the numeric settings
    numericSettings = topStackFrame->getNumericSettings();
    // this should be tree, but make sure we don't clobber the global settings accidentally
    if (ActivityManager::currentActivity == this)
    {
        Numerics::setCurrentSettings(this->numericSettings);
    }
}


/**
 * Push a Rexx code activation on to the stack frame.
 *
 * @param new_activation
 *               The new activation to add.
 */
void RexxActivity::pushStackFrame(RexxActivationBase *new_activation)
{
    checkActivationStack();         // make sure the stack is not filled
    // push on to the stack and bump the depth
    activations->push((RexxObject *)new_activation);
    stackFrameDepth++;
    // update the frame information.
    // if we're not creating a new frame set, link up the new frame with its predecessor
    if (!new_activation->isStackBase())
    {
        new_activation->setPreviousStackFrame(topStackFrame);
    }
    updateFrameMarkers();
}


/**
 * Create a new set of activation stack frames on this activity.
 * The new frame will have a RexxNativeActivation that's marked
 * as a stack base frame.  Additional call frames are pushed on
 * top of that activation.  Any operations that unwind the
 * stack frames will stop when they hit the activation stack
 * base.
 */
void RexxActivity::createNewActivationStack()
{
    // make sure we have a new stack
    checkActivationStack();
    // This is a root activation that will allow API functions to be called
    // on this thread without having an active bit of ooRexx code first.
    RexxNativeActivation *new_activation = ActivityManager::newNativeActivation(this);
    new_activation->setStackBase();
    // create a new root element on the stack and bump the depth indicator
    activations->push((RexxObject *)new_activation);
    stackFrameDepth++;
    // update the frame information.
    updateFrameMarkers();
}


/**
 * Pop the top activation from the frame stack.
 *
 * @param reply  Indicates we're popping the frame for a reply operation.  In that
 *               case, we can't return the frame to the activation cache.
 */
void RexxActivity::popStackFrame(bool  reply)
{
    // pop off the top elements and reduce the depth
    RexxActivationBase *poppedStackFrame = (RexxActivationBase *)activations->fastPop();
    stackFrameDepth--;

    // did we just pop off the last element of a stack frame?  This should not happen, so
    // push it back on to the stack
    if (poppedStackFrame->isStackBase())
    {
        activations->push((RexxObject *)poppedStackFrame);
        stackFrameDepth++;
    }
    else
    {
        // update the frame information.
        updateFrameMarkers();

        // if this is not a reply operation and the frame we just removed is
        // a Rexx activation, we can just cache this.
        if (!reply)
        {
            /* add this to the cache             */
            ActivityManager::cacheActivation(poppedStackFrame);
        }
    }
}


void RexxActivity::cleanupStackFrame(RexxActivationBase *poppedStackFrame)
{
    // make sure this frame is terminated first
    poppedStackFrame->termination();

    /* add this to the cache             */
    ActivityManager::cacheActivation(poppedStackFrame);
}


/**
 * Pop entries off the stack frame upto and including the
 * target activation.
 *
 * @param target The target for the pop operation.
 */
void RexxActivity::popStackFrame(RexxActivationBase *target)
{
    RexxActivationBase *poppedStackFrame = (RexxActivationBase *)activations->fastPop();
    stackFrameDepth--;
    // pop off the top elements and reduce the depth
    while (poppedStackFrame != target)
    {
        // clean this up and potentially cache
        cleanupStackFrame(poppedStackFrame);
        poppedStackFrame = (RexxActivationBase *)activations->fastPop();
        stackFrameDepth--;
    }

    // clean this up and potentially cache
    cleanupStackFrame(poppedStackFrame);
    // update the frame information.
    updateFrameMarkers();
}


void RexxActivity::unwindStackFrame()
/******************************************************************************/
/* Function:  Remove an activation marker from the activity stack             */
/******************************************************************************/
{
    // pop activations off until we find the one at the base of the stack.
    while (stackFrameDepth > 0)
    {
        // check the top activation.  If it's a stack base item, then
        // we've reached the unwind point.
        RexxActivationBase *poppedActivation = (RexxActivationBase *)activations->fastPop();
        stackFrameDepth--;
        if (poppedActivation->isStackBase())
        {
            // at the very base of the activity, we keep a base item.  If this
            // is the bottom stack frame here, then push it back on.
            if (stackFrameDepth == 0)
            {
                activations->push((RexxObject *)poppedActivation);
                stackFrameDepth++;
            }
            break;
        }
    }

    // update the frame information.
    updateFrameMarkers();
}


void RexxActivity::unwindToDepth(size_t depth)
/******************************************************************************/
/* Function:  Remove an activation marker from the activity stack             */
/******************************************************************************/
{
    // pop elements until we unwind to the target
    while (stackFrameDepth > depth)
    {
        activations->fastPop();
        stackFrameDepth--;
    }

    // update the frame information.
    updateFrameMarkers();
}


/**
 * Unwind to a particular stack frame, terminating each frame
 * in turn;
 *
 * @param frame  The target frame
 */
void RexxActivity::unwindToFrame(RexxActivation *frame)
{
    RexxActivationBase *activation;

    /* unwind the activation stack       */
    while ((activation = getTopStackFrame()) != frame)
    {
        activation->termination();       /* prepare the activation for termin */
        popStackFrame(false);            /* pop the activation off the stack  */
    }
}


/**
 * Set up an activity as a root activity used either for a main
 * interpreter thread or an attached thread.
 *
 * @param interpreter
 *               The interpreter instance this thread belongs to.
 */
void RexxActivity::setupAttachedActivity(InterpreterInstance *interpreter)
{
    // associate this with the instance
    addToInstance(interpreter);

    // mark this as an attached activity
    attached = true;
    // This is a root activation that will allow API functions to be called
    // on this thread without having an active bit of ooRexx code first.
    createNewActivationStack();
}


/**
 * Set up an activity as a root activity used either for a main
 * interpreter thread or an attached thread.
 *
 * @param interpreter
 *               The interpreter instance this thread belongs to.
 */
void RexxActivity::addToInstance(InterpreterInstance *interpreter)
{
    // we're associated with this instance
    instance = interpreter;
    // create a thread context that we can hand out when needed.
    threadContext.threadContext.instance = instance->getInstanceContext();
    threadContext.threadContext.functions = &threadContextFunctions;
    threadContext.owningActivity = this;

    // go copy the exit definitions
    setupExits();
}


/**
 * Process for copying the exit definitions from the
 * hosting instance.
 */
void RexxActivity::setupExits()
{

    // copy all of the system exits
    for (int i = 0; i < LAST_EXIT; i++)
    {
        sysexits[i] = instance->getExitHandler(i + 1);
    }
    // set the appropriate exit interlocks
    queryTrcHlt();
}


/**
 * Complete initialization of the thread context function
 * vector by filling in the constant objects.
 */
void RexxActivity::initializeThreadContext()
{
    threadContextFunctions.RexxNil = (RexxObjectPtr)TheNilObject;
    threadContextFunctions.RexxTrue = (RexxObjectPtr)TheTrueObject;
    threadContextFunctions.RexxFalse = (RexxObjectPtr)TheFalseObject;
    threadContextFunctions.RexxNullString = (RexxStringObject)OREF_NULLSTRING;
}


/**
 * Detach a thread from the interpreter instance,
 */
void RexxActivity::detachThread()
{
    instance->detachThread(this);
}

/**
 * Cleanup the resources for a detached activity, including
 * removing the suspended state from a pushed activity nest.
 */
void RexxActivity::detachInstance()
{
    // Undo this attached status
    instance = OREF_NULL;
    attached = false;
    // if there's a nesting situation, restore the activity to active state.
    if (nestedActivity != OREF_NULL)
    {
        nestedActivity->setSuspended(false);
    }
}


void RexxActivity::exitKernel()
/******************************************************************************/
/*  Function:  Release the kernel access because code is going to "leave"     */
/*             the kernel.  This prepares for this by adding a native         */
/*             activation on to the stack to act as a server for the          */
/*             external call.  This way new native methods do not need to     */
/*             be created just to get an activation created                   */
/******************************************************************************/
{
    // create new activation frame using the current Rexx frame (which can be null, but
    // is not likely to be).
    RexxNativeActivation *new_activation = ActivityManager::newNativeActivation(this, currentRexxFrame);
    // this becomes the new top activation.  We also turn on the variable pool for
    // this situation.
    this->pushStackFrame(new_activation);
    new_activation->enableVariablepool();
    // release the kernel in preparation for exiting
    releaseAccess();
}


void RexxActivity::enterKernel()
/******************************************************************************/
/*  Function:  Recover the kernel access and pop the native activation        */
/*             created by activity_exit_kernel from the activity stack.       */
/******************************************************************************/
{
  requestAccess();                     /* get the kernel lock back          */
  this->popStackFrame(false);          /* pop the top activation            */
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
      owningActivity = ((RexxMessage *)this->waitingObject)->getActivity();
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
    runsem.reset();                      /* clear the run semaphore           */
    this->waitingObject = resource;      /* save the waiting resource         */
    releaseAccess();                     /* release the kernel access         */
    runsem.wait();                       /* wait for the run to be posted     */
    requestAccess();                     /* reaquire the kernel access        */
}

void RexxActivity::guardWait()
/******************************************************************************/
/* Function:  Wait for a guard post event                                     */
/******************************************************************************/
{
    releaseAccess();                     /* release kernel access             */
    guardsem.wait();                     /* wait on the guard semaphore       */
    requestAccess();                     /* reaquire the kernel lock          */
}

void RexxActivity::guardPost()
/******************************************************************************/
/* Function:  Post a guard expression wake up notice                          */
/******************************************************************************/
{
    guardsem.post();                     /* OK for it to already be posted    */
}

void RexxActivity::guardSet()
/******************************************************************************/
/* Function:  Clear a guard expression semaphore in preparation to perform a  */
/*            guard wait                                                      */
/******************************************************************************/
{
    guardsem.reset();               /* set up for guard call             */
}

void RexxActivity::postRelease()
/******************************************************************************/
/* Function:  Post an activities run semaphore                                */
/******************************************************************************/
{
    this->waitingObject = OREF_NULL;     /* no longer waiting                 */
    runsem.post();                       /* OK for it to already be posted    */
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

void RexxActivity::relinquish()
/******************************************************************************/
/*  Function: Relinquish to other system processes                            */
/******************************************************************************/
{
    ActivityManager::relinquish(this);
}

/**
 * Tap the current running activation on this activity to
 * give up control at the next reasonsable boundary.
 */
void RexxActivity::yield()
{
                                       /* get the current activation        */
    RexxActivation *activation = currentRexxFrame;
    // if we're in the context of Rexx code, request that it yield control
    if (activation != NULL)
    {
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
    RexxActivation *activation = currentRexxFrame;
                                       /* got an activation?                */
    if (activation != NULL)
    {
        // please make it stop :-)
        return activation->halt(d);
    }
    return true;
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
    RexxActivation *activation = currentRexxFrame;
                                       /* got an activation?                */
    if (activation != NULL)
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
    // make sure we're really the holder on this
    if (ActivityManager::currentActivity == this)
    {
        // reset the numeric settings
        Numerics::setDefaultSettings();
        ActivityManager::unlockKernel();
    }
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
    // belt and braces to ensure this is done on this thread
    ActivityManager::currentActivity = this;          /* set new current activity          */
}

void RexxActivity::checkStackSpace()
/******************************************************************************/
/* Function:  Make sure there is enough stack space to run a method           */
/******************************************************************************/
{
#ifdef STACKCHECK
  size_t temp;                          /* if checking and there isn't room  */
  if (((char *)&temp - (char *)this->stackBase) < MIN_C_STACK && this->stackcheck == true)
  {
                                       /* go raise an exception             */
      reportException(Error_Control_stack_full);
  }
#endif
}


RexxDirectory *RexxActivity::getLocal()
/******************************************************************************/
/* Function:  Retrive the activities local environment                        */
/******************************************************************************/
{
  return instance->getLocal();              // just return the .local directory
}

thread_id_t  RexxActivity::threadIdMethod()
/******************************************************************************/
/* Function:  Retrieve the activities threadid                                */
/******************************************************************************/
{
    return currentThread.getThreadID();  /* just return the thread id info    */
}

void RexxActivity::queryTrcHlt()
/******************************************************************************/
/* Function:  Determine if Halt or Trace System exits are set                 */
/*            and set a flag to indicate this.                                */
/******************************************************************************/
{                                      /* is HALT sys exit set              */
    if (isExitEnabled(RXHLT))
    {
        this->clauseExitUsed = true;       /* set flag to indicate one is found */
        return;                            /* and return                        */
    }
    /* is TRACE sys exit set             */
    if (isExitEnabled(RXTRC))
    {
        this->clauseExitUsed = true;   /* set flag to indicate one is found */
        return;                            /* and return                        */
    }

    this->clauseExitUsed = false;    /* remember that none are set        */
}


/**
 * Call an individual exit handler.
 *
 * @param activation The activation this is in the context of.
 * @param exitName   The logical name of the handler.
 * @param function   The exit function.
 * @param subfunction
 *                   The exit subfunction.
 * @param exitbuffer The parameter structure for the exit in question.
 *
 * @return The exit handling state.
 */
bool RexxActivity::callExit(RexxActivation * activation,
    const char *exitName, int function,
    int subfunction, void *exitbuffer)
{
    ExitHandler &handler = getExitHandler(function);

    int rc = handler.call(this, activation, function, subfunction, exitbuffer);

    /* got an error case?                  */
    if (rc == RXEXIT_RAISE_ERROR || rc < 0)
    {
        if (function == RXSIO)             /* this the I/O function?              */
        {
            /* disable the I/O exit from here to   */
            /* prevent recursive error conditions  */
            disableExit(RXSIO);
        }
        /* go raise an error                   */
        reportException(Error_System_service_service, exitName);
    }
    return rc == RXEXIT_HANDLED;         /* Did exit handle task?               */
}


void RexxActivity::callInitializationExit(
    RexxActivation *activation)        /* sending activation                */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the initialization system exit.                                */
/******************************************************************************/
{
    if (isExitEnabled(RXINI))  // is the exit enabled?
    {
        /* add the variable RXPROGRAMNAME to */
        /* the variable pool, it contains the*/
        /* script name that is currently run */
                                           /* call the handler                  */
        callExit(activation, "RXINI", RXINI, RXINIEXT, NULL);
    }
}

void RexxActivity::callTerminationExit(
    RexxActivation *activation)        /* sending activation                */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the termination system exit.                                   */
/******************************************************************************/
{
    if (isExitEnabled(RXTER))  // is the exit enabled?
    {
        callExit(activation, "RXTER", RXTER, RXTEREXT, NULL);
    }
}

bool  RexxActivity::callSayExit(
    RexxActivation *activation,        /* sending activation                */
    RexxString     *sayoutput)         /* line to write out                 */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the I/O system exit.                                           */
/******************************************************************************/
{
    if (isExitEnabled(RXSIO))  // is the exit enabled?
    {
        RXSIOSAY_PARM exit_parm;             /* exit parameters                   */

        sayoutput->toRxstring(exit_parm.rxsio_string);
        return !callExit(activation, "RXSIO", RXSIO, RXSIOSAY, &exit_parm);
    }
    return true;                         /* exit didn't handle                */
}

bool RexxActivity::callTraceExit(
    RexxActivation *activation,        /* sending activation                */
    RexxString     *traceoutput)       /* line to write out                 */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the I/O system exit.                                           */
/******************************************************************************/
{
    if (isExitEnabled(RXSIO))  // is the exit enabled?
    {
        RXSIOSAY_PARM exit_parm;             /* exit parameters                   */
        traceoutput->toRxstring(exit_parm.rxsio_string);
        return !callExit(activation, "RXSIO", RXSIO, RXSIOTRC, &exit_parm);
    }
    return true;                         /* exit didn't handle                */
}

bool RexxActivity::callTerminalInputExit(
    RexxActivation *activation,        /* sending activation                */
    RexxString *&inputstring)          /* returned input string             */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the I/O system exit.                                           */
/******************************************************************************/
{
    if (isExitEnabled(RXSIO))  // is the exit enabled?
    {
        RXSIOTRD_PARM exit_parm;             /* exit parameters                   */
        char          retbuffer[DEFRXSTRING];/* Default result buffer             */

        *retbuffer = '\0';
        /* Pass along default RXSTRING       */
        MAKERXSTRING(exit_parm.rxsiotrd_retc, retbuffer, DEFRXSTRING);
        if (!callExit(activation, "RXSIO", RXSIO, RXSIOTRD, &exit_parm))
        {
            return true;
        }
        /* Get input string and return it    */
        inputstring = new_string(exit_parm.rxsiotrd_retc);
        /* user give us a new buffer?        */
        if (exit_parm.rxsiotrd_retc.strptr != retbuffer)
        {
            /* free it                           */
            SystemInterpreter::releaseResultMemory(exit_parm.rxsiotrd_retc.strptr);

        }
        return false;                      /* this was handled                  */
    }
    return true;                         /* not handled                       */
}

bool RexxActivity::callDebugInputExit(
    RexxActivation *activation,        /* sending activation                */
    RexxString    *&inputstring)       /* returned input value              */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the I/O system exit.                                           */
/******************************************************************************/
{
    if (isExitEnabled(RXSIO))  // is the exit enabled?
    {
        RXSIOTRD_PARM exit_parm;             /* exit parameters                   */
        char          retbuffer[DEFRXSTRING];/* Default result buffer             */

        *retbuffer = '\0';
        /* Pass along default RXSTRING       */
        MAKERXSTRING(exit_parm.rxsiotrd_retc, retbuffer, DEFRXSTRING);
        if (!callExit(activation, "RXSIO", RXSIO, RXSIODTR, &exit_parm))
        {
            return true;
        }
        /* Get input string and return it    */
        inputstring = new_string(exit_parm.rxsiotrd_retc);
        /* user give us a new buffer?        */
        if (exit_parm.rxsiotrd_retc.strptr != retbuffer)
        {
            /* free it                           */
            SystemInterpreter::releaseResultMemory(exit_parm.rxsiotrd_retc.strptr);

        }
        return false;                      /* this was handled                  */
    }
    return true;                         /* not handled                       */
}

bool RexxActivity::callFunctionExit(
    RexxActivation *activation,        /* calling activation                */
    RexxString     *rname,             /* routine name                      */
    RexxObject     *calltype,          /* type of call                      */
    ProtectedObject &funcresult,       /* function result                   */
    RexxObject    **arguments,         /* argument array                    */
    size_t          argcount)          /* argument count                    */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the function system exit.                                      */
/******************************************************************************/
{

    if (isExitEnabled(RXFNC))  // is the exit enabled?
    {
        RXFNCCAL_PARM exit_parm;             /* exit parameters                   */
        char          retbuffer[DEFRXSTRING];/* Default result buffer             */
        /* Start building the exit block  */
        exit_parm.rxfnc_flags.rxfferr = 0; /* Initialize error codes to zero */
        exit_parm.rxfnc_flags.rxffnfnd = 0;


        exit_parm.rxfnc_flags.rxffsub = calltype == OREF_FUNCTIONNAME ? 0 : 1;
        /* fill in the name parameter        */
        exit_parm.rxfnc_namel = (unsigned short)rname->getLength();
        exit_parm.rxfnc_name = rname->getStringData();

        /* Get current active queue name     */
        RexxString *stdqueue = Interpreter::getCurrentQueue();
        /* fill in the name                  */
        exit_parm.rxfnc_que = stdqueue->getStringData();
        /* and the length                    */
        exit_parm.rxfnc_quel = (unsigned short)stdqueue->getLength();
        /* Build arg array of RXSTRINGs      */
        /* get number of args                */
        exit_parm.rxfnc_argc = (unsigned short)argcount;


        /* allocate enough memory for all arguments.           */
        /* At least one item needs to be allocated in order to avoid an error   */
        /* in the sysexithandler!                                               */
        PCONSTRXSTRING argrxarray = (PCONSTRXSTRING) SystemInterpreter::allocateResultMemory(
             sizeof(CONSTRXSTRING) * Numerics::maxVal((size_t)exit_parm.rxfnc_argc, (size_t)1));
        if (argrxarray == OREF_NULL)       /* memory error?                     */
        {
            reportException(Error_System_resources);
        }
        /* give the arg array pointer        */
        exit_parm.rxfnc_argv = argrxarray;
        /* construct the arg array           */
        for (size_t argindex=0; argindex < exit_parm.rxfnc_argc; argindex++)
        {
            // classic REXX style interface
            RexxString *temp = (RexxString *)arguments[argindex];
            if (temp != OREF_NULL)           /* got a real argument?              */
            {
                /* force the argument to a string    */
                temp = (RexxString *)REQUEST_STRING(temp);
                /* point to the string               */
                temp->toRxstring(argrxarray[argindex]);
            }
            else
            {
                /* empty argument                    */
                argrxarray[argindex].strlength = 0;
                argrxarray[argindex].strptr = (const char *)NULL;
            }
        }
        /* Pass default result RXSTRING      */
        MAKERXSTRING(exit_parm.rxfnc_retc, retbuffer, DEFRXSTRING);
        /* call the handler                  */
        bool wasHandled = callExit(activation, "RXFNC", RXFNC, RXFNCCAL, (void *)&exit_parm);

        SystemInterpreter::releaseResultMemory(argrxarray);

        if (!wasHandled)
        {
            return true;                     /* this wasn't handled               */
        }


        if (exit_parm.rxfnc_flags.rxfferr) /* function error?                   */
        {
            /* raise an error                    */
            reportException(Error_Incorrect_call_external, rname);
        }
        /* Did we find the function??        */
        else if (exit_parm.rxfnc_flags.rxffnfnd)
        {
            /* also an error                     */
            reportException(Error_Routine_not_found_name,rname);
        }
        /* Was it a function call??          */
        if (exit_parm.rxfnc_retc.strptr == OREF_NULL && calltype == OREF_FUNCTIONNAME)
        {
            /* Have to return data               */
            reportException(Error_Function_no_data_function,rname);
        }

        if (exit_parm.rxfnc_retc.strptr)   /* If we have result, return it      */
        {
            /* Get input string and return it    */
            funcresult = new_string(exit_parm.rxfnc_retc);
            /* user give us a new buffer?        */
            if (exit_parm.rxfnc_retc.strptr != retbuffer)
            {
                /* free it                           */
                SystemInterpreter::releaseResultMemory(exit_parm.rxfnc_retc.strptr);
            }
        }
        return false;                      /* this was handled                  */
    }
    return true;                         /* not handled                       */
}


bool RexxActivity::callObjectFunctionExit(
    RexxActivation *activation,        /* calling activation                */
    RexxString     *rname,             /* routine name                      */
    RexxObject     *calltype,          /* type of call                      */
    ProtectedObject &funcresult,       /* function result                   */
    RexxObject    **arguments,         /* argument array                    */
    size_t          argcount)          /* argument count                    */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the function system exit.                                      */
/******************************************************************************/
{
    // give the security manager the first pass
    SecurityManager *manager = activation->getEffectiveSecurityManager();
    if (manager != OREF_NULL)
    {
        if (manager->checkFunctionCall(rname, argcount, arguments, funcresult))
        {
            return false;
        }
    }

    if (isExitEnabled(RXOFNC))  // is the exit enabled?
    {
        RXOFNCCAL_PARM exit_parm;             /* exit parameters                   */
        /* Start building the exit block  */
        exit_parm.rxfnc_flags.rxfferr = 0; /* Initialize error codes to zero */
        exit_parm.rxfnc_flags.rxffnfnd = 0;


        exit_parm.rxfnc_flags.rxffsub = calltype == OREF_FUNCTIONNAME ? 0 : 1;
        /* fill in the name parameter        */
        rname->toRxstring(exit_parm.rxfnc_name);

        /* get number of args                */
        exit_parm.rxfnc_argc = argcount;
        // the argument pointers get passed as is
        exit_parm.rxfnc_argv = (RexxObjectPtr *)arguments;
        // no result value
        exit_parm.rxfnc_retc = NULLOBJECT;
        /* call the handler                  */
        if (!callExit(activation, "RXOFNC", RXOFNC, RXOFNCCAL, (void *)&exit_parm))
        {
            return true;                     /* this wasn't handled               */
        }

        if (exit_parm.rxfnc_flags.rxfferr) /* function error?                   */
        {
            /* raise an error                    */
            reportException(Error_Incorrect_call_external, rname);
        }
        /* Did we find the function??        */
        else if (exit_parm.rxfnc_flags.rxffnfnd)
        {
            /* also an error                     */
            reportException(Error_Routine_not_found_name,rname);
        }
        /* Was it a function call??          */
        if (exit_parm.rxfnc_retc == NULLOBJECT && calltype == OREF_FUNCTIONNAME)
        {
            /* Have to return data               */
            reportException(Error_Function_no_data_function,rname);
        }
        // set the function result back
        funcresult = (RexxObject *)exit_parm.rxfnc_retc;
        return false;                      /* this was handled                  */
    }
    return true;                         /* not handled                       */
}


bool RexxActivity::callScriptingExit(
    RexxActivation *activation,        /* calling activation                */
    RexxString     *rname,             /* routine name                      */
    RexxObject     *calltype,          /* type of call                      */
    ProtectedObject &funcresult,       /* function result                   */
    RexxObject    **arguments,         /* argument array                    */
    size_t          argcount)          /* argument count                    */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the function system exit.                                      */
/******************************************************************************/
{
    if (isExitEnabled(RXEXF))  // is the exit enabled?
    {
        RXEXFCAL_PARM exit_parm;             /* exit parameters                   */
        /* Start building the exit block  */
        exit_parm.rxfnc_flags.rxfferr = 0; /* Initialize error codes to zero */
        exit_parm.rxfnc_flags.rxffnfnd = 0;


        exit_parm.rxfnc_flags.rxffsub = calltype == OREF_FUNCTIONNAME ? 0 : 1;
        /* fill in the name parameter        */
        rname->toRxstring(exit_parm.rxfnc_name);

        /* get number of args                */
        exit_parm.rxfnc_argc = argcount;
        // the argument pointers get passed as is
        exit_parm.rxfnc_argv = (RexxObjectPtr *)arguments;
        // no result value
        exit_parm.rxfnc_retc = NULLOBJECT;
        /* call the handler                  */
        if (!callExit(activation, "RXEXF", RXEXF, RXEXFCAL, (void *)&exit_parm))
        {
            return true;                     /* this wasn't handled               */
        }

        if (exit_parm.rxfnc_flags.rxfferr) /* function error?                   */
        {
            /* raise an error                    */
            reportException(Error_Incorrect_call_external, rname);
        }
        /* Did we find the function??        */
        else if (exit_parm.rxfnc_flags.rxffnfnd)
        {
            /* also an error                     */
            reportException(Error_Routine_not_found_name,rname);
        }
        /* Was it a function call??          */
        if (exit_parm.rxfnc_retc == NULLOBJECT && calltype == OREF_FUNCTIONNAME)
        {
            /* Have to return data               */
            reportException(Error_Function_no_data_function,rname);
        }
        // set the function result back
        funcresult = (RexxObject *)exit_parm.rxfnc_retc;
        return false;                      /* this was handled                  */
    }
    return true;                         /* not handled                       */
}


bool RexxActivity::callCommandExit(RexxActivation *activation, RexxString *address, RexxString *command, ProtectedObject &result, ProtectedObject &condition)
{
    // give the security manager the first pass
    SecurityManager *manager = activation->getEffectiveSecurityManager();
    if (manager != OREF_NULL)
    {
        if (manager->checkCommand(address, command, result, condition))
        {
            return false;
        }
    }

    if (isExitEnabled(RXCMD))  // is the exit enabled?
    {
        RXCMDHST_PARM exit_parm;             /* exit parameters                   */
        char          retbuffer[DEFRXSTRING];/* Default result buffer             */
        /* Start building the exit block     */
        exit_parm.rxcmd_flags.rxfcfail = 0;/* Initialize failure/error to zero  */
        exit_parm.rxcmd_flags.rxfcerr = 0;
        /* fill in the environment parm      */
        exit_parm.rxcmd_addressl = (unsigned short)address->getLength();
        exit_parm.rxcmd_address = address->getStringData();
        /* make cmdaname into RXSTRING form  */
        command->toRxstring(exit_parm.rxcmd_command);

        exit_parm.rxcmd_dll = NULL;        /* Currently no DLL support          */
        exit_parm.rxcmd_dll_len = 0;       /* 0 means .EXE style                */
                                           /* Pass default result RXSTRING      */
        MAKERXSTRING(exit_parm.rxcmd_retc, retbuffer, DEFRXSTRING);
        /* call the handler                  */
        if (!callExit(activation, "RXCMD", RXCMD, RXCMDHST, (void *)&exit_parm))
        {
            return true;                     /* this wasn't handled               */
        }
        if (exit_parm.rxcmd_flags.rxfcfail)/* need to raise failure condition?  */
        {
            // raise the condition when things are done
            condition = RexxActivity::createConditionObject(OREF_FAILURENAME, (RexxObject *)result, command, OREF_NULL, OREF_NULL);
        }

        /* Did we find the function??        */
        else if (exit_parm.rxcmd_flags.rxfcerr)
        {
            // raise the condition when things are done
            condition = RexxActivity::createConditionObject(OREF_ERRORNAME, (RexxObject *)result, command, OREF_NULL, OREF_NULL);
        }
        /* Get input string and return it    */
        result = new_string(exit_parm.rxcmd_retc);
        /* user give us a new buffer?        */
        if (exit_parm.rxcmd_retc.strptr != retbuffer)
        {
            /* free it                           */
            SystemInterpreter::releaseResultMemory(exit_parm.rxcmd_retc.strptr);
        }
        return false;                      /* this was handled                  */
    }
    return true;                         /* not handled                       */
}


bool  RexxActivity::callPullExit(
    RexxActivation *activation,        /* sending activation                */
    RexxString *&inputstring)          /* returned input string             */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the External Data queue system exit.                           */
/******************************************************************************/
{
    if (isExitEnabled(RXMSQ))  // is the exit enabled?
    {
        RXMSQPLL_PARM exit_parm;             /* exit parameters                   */
        char          retbuffer[DEFRXSTRING];/* Default result buffer             */

                                             /* Pass along default RXSTRING       */
        MAKERXSTRING(exit_parm.rxmsq_retc, retbuffer, DEFRXSTRING);
        /* call the handler                  */
        if (!callExit(activation, "RXMSQ", RXMSQ, RXMSQPLL, (void *)&exit_parm))
        {
            return true;                     /* this wasn't handled               */
        }
        if (exit_parm.rxmsq_retc.strptr == NULL)/* if rxstring not null              */
        {
            /* no value returned,                */
            /* return NIL to note empty stack    */
            inputstring = (RexxString *)TheNilObject;
        }
        else                               /* return resulting object           */
        {
            inputstring = new_string(exit_parm.rxmsq_retc);
            /* user give us a new buffer?        */
            if (exit_parm.rxmsq_retc.strptr != retbuffer)
            {
                /* free it                           */
                SystemInterpreter::releaseResultMemory(exit_parm.rxmsq_retc.strptr);
            }
        }
        return false;                      /* this was handled                  */
    }
    return true;                           /* not handled                       */
}

bool  RexxActivity::callPushExit(
    RexxActivation *activation,        /* sending activation                */
    RexxString *inputstring,           /* string to be placed on the queue  */
    int lifo_flag)                     /* flag to indicate LIFO or FIFO     */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the External Data queue system exit.                           */
/******************************************************************************/
{
    if (isExitEnabled(RXMSQ))  // is the exit enabled?
    {
        RXMSQPSH_PARM exit_parm;             /* exit parameters                   */

                                             /* get the exit handler              */
        if (lifo_flag == QUEUE_LIFO)       /* LIFO queuing requested?           */
        {
            /* set the flag appropriately        */
            exit_parm.rxmsq_flags.rxfmlifo = 1;
        }
        else
        {
            /* this is a FIFO request            */
            exit_parm.rxmsq_flags.rxfmlifo = 0;
        }
        /* make into RXSTRING form           */
        inputstring->toRxstring(exit_parm.rxmsq_value);
        /* call the handler                  */
        if (!callExit(activation, "RXMSQ", RXMSQ, RXMSQPSH, (void *)&exit_parm))
        {
            return true;                     /* this wasn't handled               */
        }
        return false;                      /* this was handled                  */
    }
    return true;                         /* not handled                       */
}


bool  RexxActivity::callQueueSizeExit(
    RexxActivation *activation,        /* sending activation                */
    RexxInteger *&returnsize)          /* returned queue size               */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the External Data queue system exit.                           */
/******************************************************************************/
{
    if (isExitEnabled(RXMSQ))  // is the exit enabled?
    {
        RXMSQSIZ_PARM exit_parm;             /* exit parameters                   */
        /* call the handler                  */
        if (!callExit(activation, "RXMSQ", RXMSQ, RXMSQSIZ, (void *)&exit_parm))
        {
            return true;                     /* this wasn't handled               */
        }
        /* Get queue size and return it      */
        returnsize = new_integer(exit_parm.rxmsq_size);

        return false;                      /* this was handled                  */
    }
    return true;                         /* not handled                       */
}


bool  RexxActivity::callQueueNameExit(
    RexxActivation *activation,        /* sending activation                */
    RexxString    *&inputstring )      /* name of external queue            */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the External Data queue system exit.                           */
/******************************************************************************/
{
    if (isExitEnabled(RXMSQ))  // is the exit enabled?
    {
        RXMSQNAM_PARM exit_parm;             /* exit parameters                   */
        char          retbuffer[DEFRXSTRING];/* Default result buffer             */

        MAKERXSTRING(exit_parm.rxmsq_name, retbuffer, inputstring->getLength());
        memcpy(exit_parm.rxmsq_name.strptr, inputstring->getStringData(), inputstring->getLength());
        /* call the handler                  */
        if (!callExit(activation, "RXMSQ", RXMSQ, RXMSQNAM, (void *)&exit_parm))
        {
            return true;                     /* this wasn't handled               */
        }
        inputstring = new_string(exit_parm.rxmsq_name);
        /* user give us a new buffer?        */
        if (exit_parm.rxmsq_name.strptr != retbuffer)
        {
            /* free it                           */
            SystemInterpreter::releaseResultMemory(exit_parm.rxmsq_name.strptr);
        }
        return false;                      /* this was handled                  */
    }
    return true;                         /* not handled                       */
}


bool RexxActivity::callHaltTestExit(
    RexxActivation *activation)        /* sending activation                */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the Test Halt system exit.                                     */
/******************************************************************************/
{
    if (isExitEnabled(RXHLT))  // is the exit enabled?
    {
        RXHLTTST_PARM exit_parm;             /* exit parameters                   */

                                             /* Clear halt so as not to be sloppy */
        exit_parm.rxhlt_flags.rxfhhalt = 0;
        /* call the handler                  */
        if (!callExit(activation, "RXHLT", RXHLT, RXHLTTST, (void *)&exit_parm))
        {
            return true;                     /* this wasn't handled               */
        }
        /* Was halt requested?               */
        if (exit_parm.rxhlt_flags.rxfhhalt == 1)
        {
            /* Then honor the halt request       */
            activation->halt(OREF_NULL);
        }
        return false;                      /* this was handled                  */
    }
    return true;                         /* not handled                       */
}


bool RexxActivity::callHaltClearExit(
    RexxActivation *activation)        /* sending activation                */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the Clear Halt system exit.                                    */
/******************************************************************************/
{
    if (isExitEnabled(RXHLT))  // is the exit enabled?
    {
        RXHLTTST_PARM exit_parm;             /* exit parameters                   */
        /* call the handler                  */
        if (!callExit(activation, "RXHLT", RXHLT, RXHLTCLR, (void *)&exit_parm))
        {
            return true;                     /* this wasn't handled               */
        }
        return false;                      /* this was handled                  */
    }
    return true;                         /* not handled                       */
}


bool  RexxActivity::callTraceTestExit(
     RexxActivation *activation,       /* sending activation                */
     bool currentsetting)              /* sending activation                */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the Test external trace indicator system exit.                 */
/******************************************************************************/
{
    if (isExitEnabled(RXTRC))  // is the exit enabled?
    {
        RXTRCTST_PARM exit_parm;             /* exit parameters                   */
                                             /* Clear Trace bit before  call      */
        exit_parm.rxtrc_flags.rxftrace = 0;
        /* call the handler                  */
        if (!callExit(activation, "RXTRC", RXTRC, RXTRCTST, (void *)&exit_parm))
        {
            return true;                     /* this wasn't handled               */
        }
        /* if not tracing, and now it is     */
        /* requsted                          */
        if (!currentsetting && (exit_parm.rxtrc_flags.rxftrace == 1))
        {
            /* call routine to handle this       */
            activation->externalTraceOn();
            return false;                    /* this was handled                  */
        }
        // this could be a request to stop tracing
        else if (currentsetting &&  (exit_parm.rxtrc_flags.rxftrace != 1))
        {
            /* call routine to handle this       */
            activation->externalTraceOff();
            return false;                  /* this was handled                  */
        }
    }
    return true;                         /* not handled                       */
}


bool RexxActivity::callNovalueExit(
    RexxActivation *activation,        /* sending activation                */
    RexxString     *variableName,      /* name to look up                   */
    RexxObject    *&value)             /* the returned value                */
/******************************************************************************/
/* Function:   Calls the novalue handler for uninitialized variables          */
/******************************************************************************/
{
    if (isExitEnabled(RXNOVAL))         // is the exit enabled?
    {
        RXVARNOVALUE_PARM exit_parm;       /* exit parameters                   */
        // the name is passed as a RexxStringObject
        exit_parm.variable_name = (RexxStringObject)variableName;
        // the value is returned as an object
        exit_parm.value = NULLOBJECT;      /* no value at the start             */
                                           /* call the handler                  */
        if (callExit(activation, "RXNOVAL", RXNOVAL, RXNOVALCALL, (void *)&exit_parm))
        {
            value = (RexxObject *)exit_parm.value;
            return false;
        }
    }
    return true;                         /* exit didn't handle                */
}


bool RexxActivity::callValueExit(
    RexxActivation *activation,        /* sending activation                */
    RexxString     *selector,          /* the variable set selector         */
    RexxString     *variableName,      /* name to look up                   */
    RexxObject     *newValue,          // new value for the variable
    RexxObject    *&value)             /* the returned value                */
/******************************************************************************/
/* Function:   Calls the exit for the VALUE() BIF                             */
/******************************************************************************/
{
    if (isExitEnabled(RXVALUE))         // is the exit enabled?
    {
        RXVALCALL_PARM exit_parm;       /* exit parameters                   */
        // copy the selector and variable parts
        exit_parm.selector = (RexxStringObject)selector;
        exit_parm.variable_name = (RexxStringObject)variableName;
        // the value is returned as an object, and the old value is
        // also passed that way
        exit_parm.value = (RexxObjectPtr)newValue;
                                           /* call the handler                  */
        if (callExit(activation, "RXVALUE", RXVALUE, RXVALUECALL, (void *)&exit_parm))
        {
            value = (RexxObject *)exit_parm.value;
            return false;
        }
    }
    return true;                         /* exit didn't handle                */
}


/**
 * Retrieve the current security manager instance.
 *
 * @return the security manager instance in effect for the
 *         activity.
 */
SecurityManager *RexxActivity::getEffectiveSecurityManager()
{
    // get the security manager for the top stack frame. If there is none defined, default to
    // ghe global security manager.
    SecurityManager *manager = topStackFrame->getSecurityManager();

    if (manager != OREF_NULL)
    {
        return manager;
    }

    // return the manager from the instance
    return instance->getSecurityManager();
}


/**
 * Return the security manager in effect for this instance.
 *
 * @return The globally defined security manager.
 */
SecurityManager *RexxActivity::getInstanceSecurityManager()
{
    // return the manager from the instance
    return instance->getSecurityManager();
}



void  RexxActivity::traceOutput(       /* write a line of trace information */
      RexxActivation *activation,      /* sending activation                */
      RexxString *line)                /* line to write out                 */
/******************************************************************************/
/* Function:  Write a line of trace output to the .ERROR stream               */
/******************************************************************************/
{
    line = line->stringTrace();          /* get traceable form of this        */
                                         /* if exit declines the call         */
    if (this->callTraceExit(activation, line))
    {
        /* get the default output stream     */
        RexxObject *stream = getLocalEnvironment(OREF_ERRORNAME);
        /* have .local set up yet?           */
        if (stream != OREF_NULL && stream != TheNilObject)
        {
            stream->sendMessage(OREF_LINEOUT, line);
        }
        /* do the lineout                    */
        else                               /* use the "real" default stream     */
        {
            this->lineOut(line);
        }
    }
}

void RexxActivity::sayOutput(          /* write a line of say information   */
     RexxActivation *activation,       /* sending activation                */
     RexxString *line)                 /* line to write out                 */
/******************************************************************************/
/* Function:  Write a line of SAY output to the .OUTPUT stream                */
/******************************************************************************/
{
    if (this->callSayExit(activation, line))
    {
        /* get the default output stream     */
        RexxObject *stream = getLocalEnvironment(OREF_OUTPUT);
        /* have .local set up yet?           */
        if (stream != OREF_NULL && stream != TheNilObject)
        {
            /* do the lineout                    */
            stream->sendMessage(OREF_SAY, line);
        }
        else                               /* use the "real" default stream     */
        {
            this->lineOut(line);
        }
    }
}

RexxString *RexxActivity::traceInput(  /* read a line of trace input        */
     RexxActivation *activation)       /* sending activation                */
/******************************************************************************/
/* Function:  Read a line for interactive debug input                         */
/******************************************************************************/
{
    RexxString *value;

    /* if exit declines the call         */
    if (this->callDebugInputExit(activation, value))
    {
        /* get the input stream              */
        RexxObject *stream = getLocalEnvironment(OREF_INPUT);
        if (stream != OREF_NULL)           /* have a stream?                    */
        {
            /* read from it                      */
            value = (RexxString *)stream->sendMessage(OREF_LINEIN);
            /* get something real?               */
            if (value == (RexxString *)TheNilObject)
            {
                value = OREF_NULLSTRING;       /* just us a null string if not      */
            }
        }
        else
        {
            value = OREF_NULLSTRING;         /* default to a null string          */
        }
    }
    return value;                        /* return value from exit            */
}


RexxString *RexxActivity::pullInput(   /* read a line of pull input         */
     RexxActivation *activation)       /* sending activation                */
/******************************************************************************/
/* Function:  Read a line for the PULL instruction                            */
/******************************************************************************/
{
    RexxString *value;

    /* if exit declines call             */
    if (this->callPullExit(activation, value))
    {
        /* get the external data queue       */
        RexxObject *stream = getLocalEnvironment(OREF_REXXQUEUE);
        if (stream != OREF_NULL)           /* have a data queue?                */
        {
            /* pull from the queue               */
            value = (RexxString *)stream->sendMessage(OREF_PULL);
            /* get something real?               */
            if (value == (RexxString *)TheNilObject)
            {
                /* read from the linein stream       */
                value = this->lineIn(activation);
            }
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
    RexxString *value;

    /* if exit declines call             */
    if (this->callTerminalInputExit(activation, value))
    {
        /* get the input stream              */
        RexxObject *stream = getLocalEnvironment(OREF_INPUT);
        if (stream != OREF_NULL)           /* have a stream?                    */
        {
            /* read from it                      */
            value = (RexxString *)stream->sendMessage(OREF_LINEIN);
            /* get something real?               */
            if (value == (RexxString *)TheNilObject)
            {
                value = OREF_NULLSTRING;       /* just use a null string if not     */
            }
        }
        else
        {
            value = OREF_NULLSTRING;         /* default to a null string          */
        }
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
    /* if exit declines call             */
    if (this->callPushExit(activation, line, order))
    {
        /* get the default queue             */
        RexxObject *targetQueue = getLocalEnvironment(OREF_REXXQUEUE);
        if (targetQueue != OREF_NULL)      /* have a data queue?                */
        {
            /* pull from the queue               */
            if (order == QUEUE_LIFO)         /* push instruction?                 */
            {
                /* push a line                       */
                targetQueue->sendMessage(OREF_PUSH, (RexxObject *)line);

            }
            else
            {
                /* queue a line                      */
                targetQueue->sendMessage(OREF_QUEUENAME, (RexxObject *)line);
            }
        }
    }
}

void  RexxActivity::terminatePoolActivity()
/******************************************************************************/
/* Function: Mark this FREE activity for termination.  Set its exit flag to 1 */
/*   and POST its run semaphore.                                              */
/******************************************************************************/
{
    this->exit = true;                   /* Activity should exit          */
    this->runsem.post();                /* let him run so he knows to exi*/
}


/**
 * Run a task that needs to enter the interpreter on a thread.
 * The activity will set up the root activation and run the
 * task under that context to ensure proper error handling and
 * kernel access.
 *
 * @param target The dispatcher object that implements the call out.
 */
void RexxActivity::run(ActivityDispatcher &target)
{
    size_t  startDepth;                  /* starting depth of activation stack*/

                                         /* make sure we have the stack base  */
    this->stackBase = currentThread.getStackBase(TOTAL_STACK_SIZE);
    this->generateRandomNumberSeed();    /* get a fresh random seed           */
    startDepth = stackFrameDepth;        /* Remember activation stack depth   */
                                         /* Push marker onto stack so we know */
    this->createNewActivationStack();    /* what level we entered.            */

    // save the actitivation level in case there's an error unwind for an unhandled
    // exception;
    size_t activityLevel = getActivationLevel();
    // create a new native activation
    RexxNativeActivation *newNActa = ActivityManager::newNativeActivation(this);
    pushStackFrame(newNActa);            /* push it on the activity stack     */

    try
    {
        // go run the target under the new activation
        newNActa->run(target);
    }
    catch (ActivityException)
    {
        // if we're not the current kernel holder when things return, make sure we
        // get the lock before we continue
        if (ActivityManager::currentActivity != this)
        {
            requestAccess();
        }

        // now do error processing
        wholenumber_t rc = this->error();                /* do error cleanup                  */
        target.handleError(rc, conditionobj);
    }

    // make sure we get restored to the same base activation level.
    restoreActivationLevel(activityLevel);
    // give uninit objects a chance to run
    memoryObject.runUninits();
    // unwind to the same stack depth as the start, removing all new entries
    unwindToDepth(startDepth);
    // if a condition occurred, make sure we inject this into the API-level
    // native activation so the caller can check to see if an error occurred.
    if (target.conditionData != OREF_NULL)
    {
        getApiContext()->setConditionInfo(target.conditionData);
    }
    // make sure we clear this from the activity
    clearCurrentCondition();
}


/**
 * Run a task under the context of an activity.  This will be
 * a task that calls out from the interpreter, which the
 * kernel lock released during the call.
 *
 * @param target The dispatcher object that implements the call out.
 */
void RexxActivity::run(CallbackDispatcher &target)
{
    // create new activation frame using the current Rexx frame (which can be null, but
    // is not likely to be).
    RexxNativeActivation *new_activation = ActivityManager::newNativeActivation(this, currentRexxFrame);
    // this becomes the new top activation.  We also turn on the variable pool for
    // this situation.
    this->pushStackFrame(new_activation);
    new_activation->enableVariablepool();

    // go run this
    new_activation->run(target);
    this->popStackFrame(new_activation); /* pop the top activation            */
}


/**
 * Inherit all activity-specific settings from a parent activity.
 *
 * @param parent The source of the setting information.
 */
void RexxActivity::inheritSettings(RexxActivity *parent)
{
    // copy all of the system exits
    for (int i = 0; i < LAST_EXIT; i++)
    {
        sysexits[i] = parent->sysexits[i];
    }
    clauseExitUsed = parent->clauseExitUsed;
}


/**
 * Set up a method context for use before a call out.
 *
 * @param context The method context to initialize.
 * @param owner   The native activation that owns this context.
 */
void RexxActivity::createMethodContext(MethodContext &context, RexxNativeActivation *owner)
{
    // hook this up with the activity
    context.threadContext.threadContext = &threadContext.threadContext;
    context.threadContext.functions = &methodContextFunctions;
    context.context = owner;
}


/**
 * Set up a call context for use before a call out.
 *
 * @param context The method context to initialize.
 * @param owner   The native activation that owns this context.
 */
void RexxActivity::createCallContext(CallContext &context, RexxNativeActivation *owner)
{
    // hook this up with the activity
    context.threadContext.threadContext = &threadContext.threadContext;
    context.threadContext.functions = &callContextFunctions;
    context.context = owner;
}

void RexxActivity::createExitContext(ExitContext &context, RexxNativeActivation *owner)

/**
 * Set up an exit  context for use before a call out.
 *
 * @param context The method context to initialize.
 * @param owner   The native activation that owns this context.
 */
{
    // hook this up with the activity
    context.threadContext.threadContext = &threadContext.threadContext;
    context.threadContext.functions = &exitContextFunctions;
    context.context = owner;
}


/**
 * Resolve a program using the activity context information.
 *
 * @param name   The name we're interested in.
 * @param dir    A parent directory to use as part of the search.
 * @param ext    Any parent extension name.
 *
 * @return The fully resolved file name, if it exists.  Returns OREF_NULL for
 *         non-located files.
 */
RexxString *RexxActivity::resolveProgramName(RexxString *name, RexxString *dir, RexxString *ext)
{
    return instance->resolveProgramName(name, dir, ext);
}


/**
 * Retrieve a value from the instance local environment.
 *
 * @param name   The name of the .local object.
 *
 * @return The object stored at the given name.
 */
RexxObject *RexxActivity::getLocalEnvironment(RexxString *name)
{
    return instance->getLocalEnvironment(name);
}


/**
 * Resolve a command handler from the interpreter
 * instance.
 *
 * @param name   The name of the command environment.
 *
 * @return A configured command environment, or OREF_NULL if the
 *         target environment is not found.
 */
CommandHandler *RexxActivity::resolveCommandHandler(RexxString *name)
{
    return instance->resolveCommandHandler(name);
}


/**
 * Validate that an API call is occuring on the correct thread.
 */
void RexxActivity::validateThread()
{
    if (!currentThread.validateThread())
    {
        reportException(Error_Execution_invalid_thread);
    }

}


/**
 * Get the name of the last message invocation.
 *
 * @return The last message name.
 */
RexxString *RexxActivity::getLastMessageName()
{
    return activationFrames->messageName();
}


/**
 * Get the method for the last method invocation.
 *
 * @return The last message name.
 */
RexxMethod *RexxActivity::getLastMethod()
{
    return activationFrames->method();
}
