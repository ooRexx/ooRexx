/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2022 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                                        */
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
#include "RexxCore.h"
#include "StringClass.hpp"
#include "Activity.hpp"
#include "RexxActivation.hpp"
#include "NativeActivation.hpp"
#include "MessageClass.hpp"
#include "ArrayClass.hpp"
#include "TableClass.hpp"
#include "DirectoryClass.hpp"
#include "VariableDictionary.hpp"
#include "RexxCode.hpp"
#include "RexxInstruction.hpp"
#include "RexxMemory.hpp"
#include "VariableDictionary.hpp"
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
#include "GlobalProtectedObject.hpp"
#include "MethodArguments.hpp"
#include "MutableBufferClass.hpp"
#include "SysProcess.hpp"
#include "MutexSemaphore.hpp"
#include "IdentityTableClass.hpp"

#include <stdio.h>
#include <time.h>


const size_t ACT_STACK_SIZE = 20;


/**
 * Allocate storage for a new activity object.
 *
 * @param size   The size to allocate.
 *
 * @return Storate for an activity object.
 */
void *Activity::operator new(size_t size)
{
   return new_object(size, T_Activity);
}


/**
 * Normal garbage collection live marking
 *
 * @param liveMark The current live mark.
 */
void Activity::live(size_t liveMark)
{
    memory_mark(instance);
    memory_mark(oldActivity);
    memory_mark(currentExit);
    memory_mark(nestedActivity);
    memory_mark(activations);
    memory_mark(topStackFrame);
    memory_mark(currentRexxFrame);
    memory_mark(conditionobj);
    memory_mark(requiresTable);
    memory_mark(waitingObject);
    memory_mark(dispatchMessage);
    memory_mark(heldMutexes);

    // have the frame stack do its own marking.
    frameStack.live(liveMark);
    // mark any protected objects we've been watching over

    ProtectedBase *p = protectedObjects;
    while (p != NULL)
    {
        p->mark(liveMark);
        p = p->next;
    }
}


/**
 * Generalized object marking.
 *
 * @param reason The reason for this live marking operation.
 */
void Activity::liveGeneral(MarkReason reason)
{
    memory_mark_general(instance);
    memory_mark_general(oldActivity);
    memory_mark_general(currentExit);
    memory_mark_general(nestedActivity);
    memory_mark_general(activations);
    memory_mark_general(topStackFrame);
    memory_mark_general(currentRexxFrame);
    memory_mark_general(conditionobj);
    memory_mark_general(requiresTable);
    memory_mark_general(waitingObject);
    memory_mark_general(dispatchMessage);
    memory_mark_general(heldMutexes);

    /* have the frame stack do its own marking. */
    frameStack.liveGeneral(reason);

    ProtectedBase *p = protectedObjects;
    while (p != NULL)
    {
        p->markGeneral(reason);
    }
}


/**
 * The main entry point for spawned activities.
 */
void Activity::runThread()
{
    int32_t base;
    // establish the stack base pointer for control stack full detection.
    stackBase = currentThread.getStackBase(&base, TOTAL_STACK_SIZE);

    for (;;)
    {
        // save the actitivation level in case there's an error unwind for an unhandled
        // exception;
        size_t activityLevel = 0;
        // the thread might have terminated for a control stack issue
        // so make sure checking is turned back on before trying to run
        // anything
        stackcheck = true;

        try
        {
            // wait for permission to run, then figure out what action
            // we've been asked to take.
            waitForRunPermission();
            // told to exit.  Most likely we were in the thread pool
            // and the interpreer is shutting down
            if (exit)
            {
                break;
            }

            // we need to have the kernel lock before we can really start working
            requestAccess();
            activityLevel = getActivationLevel();

            // if we have a dispatch message set, send it send message to kick everything off
            if (dispatchMessage != OREF_NULL)
            {
                MessageDispatcher dispatcher(dispatchMessage);
                run(dispatcher);
            }
            else
            {

                // this is a reply message start, just dispatch the Rexx code
                topStackFrame->dispatch();
            }
        }
        catch (ActivityException)    // we've had a termination event, raise an error
        {
            // it's possible that we might not have the kernel lock when
            // control returns to here.  Make sure we have it.
            if (ActivityManager::currentActivity != this)
            {
                requestAccess();
            }
            // process any error, including notifiying any listeners that this occurred.
            error();
        }

        // make sure we get restored to the same base activation level.
        restoreActivationLevel(activityLevel);

        // this is a good place for checking if there are pending uninit objects.
        memoryObject.checkUninitQueue();

        // cast off any items related to our initial dispatch.
        dispatchMessage = OREF_NULL;

        // no longer an active activity
        deactivate();

        // make sure we clean up any mutexes we hold
        cleanupMutexes();

        // reset our semaphores
        runSem.reset();
        guardSem.reset();

        // try to pool this.  If the ActivityManager doesn't take it,
        // we go into termination mode
        if (!instance->poolActivity(this))
        {
            releaseAccess();
            break;
        }
        // release the kernel lock and go wait for more work
        releaseAccess();
    }
    // tell the activity manager we're going away
    ActivityManager::activityEnded(this);
}


/**
 * Do cleanup of activity resources when an activity is completely
 * shutdown and discarded.
 */
void Activity::cleanupActivityResources()
{
    // close our semaphores and destroy the thread.
    runSem.close();
    guardSem.close();
    currentThread.close();
    // make sure any mutexes we are holding get released
    cleanupMutexes();

    // if this was the root activity for an interpreter instance or an
    // activity created for an attach thread, the root activation could be
    // holding references to any objects that have been returned by API
    // calls to that context. Make sure those are cleared out so that
    // the objects are no longer anchored
    clearLocalReferences();
}


/**
 * Clear any local references held by the root member of the
 * activation stack.
 */
void Activity::clearLocalReferences()
{
    getApiContext()->clearLocalReferences();
}


/**
 * We're leaving the current thread.  So we need to deactivate
 * this.
 */
void Activity::exitCurrentThread()
{
    // deactivate the nesting level
    deactivate();
    // if we're inactive, try to run any pending uninits
    if (isInactive())
    {
        memoryObject.checkUninitQueue();
    }
    // this activity owned the kernel semaphore before entering here...release it
    // now.
    releaseAccess();
}


/**
 * Enter the current thread for an API call.
 */
void Activity::enterCurrentThread()
{
    // the activity already existed for this thread, we're reentering,
    // so get the interpreter lock.
    requestApiAccess();
    activate();        // let the activity know it's in use, potentially nested
}


/**
 * Initialize an Activity object.
 *
 * @param createThread
 *               Indicates whether we're going to be running on the
 *               current thread, or creating a new thread.
 */
Activity::Activity(GlobalProtectedObject &p, bool createThread)
{
    // we need to protect this from garbage collection while constructing.
    // unfortunately, we can't use ProtectedObject because that requires an
    // active activity, which we can't guarantee at this point.
    p = this;

    int32_t base;         // used for determining the stack base

    // globally clear the object because we could be reusing the
    // object storage
    clearObject();
    // we need a stack that activaitons can use
    activations = new_internalstack(ACT_STACK_SIZE);
    // The framestack creates space for expression stacks and local variable tables
    frameStack.init();
    // an activity has a couple of semaphores it uses to synchronize execution.
    runSem.create();
    guardSem.create();
    activationStackSize = ACT_STACK_SIZE;
    // stack checking is enabled by default
    stackcheck = true;
    // the activity keeps a pointer to a set of numeric settings that
    // it can swap to global settings when it becomes the
    // active activity.
    numericSettings = Numerics::getDefaultSettings();
    // generate a fresh random seed number
    generateRandomNumberSeed();

    // when loading requires, we need to keep track of which ones are
    // being activively loaded on this thread to sort out reference cycles.
    requiresTable = new_string_table();
    // create a stack frame for this running context
    createNewActivationStack();

    // if we have been asked to create a new thread, we create the
    // system thread instance here
    if (createThread)
    {
        // we need to make sure this is cleared, since we use this
        // to wait for dispatch at thread start up.
        runSem.reset();
        // we need to enter this thread already marked as active, otherwise
        // the main thread might shut us down before we get a chance to perform
        // whatever function we're getting asked to run.
        activate();
        currentThread.create(this, C_STACK_SIZE);
    }
    // we are creating an activity that represents the thread
    // we're currently executing on.
    else
    {
        // run on the current thread
        currentThread.useCurrentThread();
        // reset the stack base for this thread.
        stackBase = currentThread.getStackBase(&base, TOTAL_STACK_SIZE);
    }
}


/**
 * Initialize an Activity object that's being recycled for
 * another use.
 */
void Activity::reset()
{
    // we are going to redispatch a thread, so we need to mark this as
    // active now to prevent this from getting terminated before it has a
    // chance to run
    activate();
}


/**
 * Create a new activity for processing a method reply
 * instruction.
 *
 * @return The newly created activity.
 */
Activity* Activity::spawnReply()
{
    // recreate a new activiy in the same instance
    return instance->spawnActivity(this);
}


/**
 * Generate a fresh random number seed.
 */
void Activity::generateRandomNumberSeed()
{
    // we use our own random number generator, but it's perfectly
    // to use the C library one to generate a random initial seed.
    randomSeed = 0;
    // rand uses a static seed, so we have to set a
    // reasonably random starting seed.  Using both the time and
    // clock makes things a little more random, and the process
    // id and thread id lessens the chance that we get a repeat.
    srand((int)time(NULL) + (int)clock() + SysProcess::getPid() + (intptr_t)currentThread.getThreadID());

    // the random number implementations vary on how large the
    // values are, but the are guaranteed to be at least 16-bit
    // quanties.  We'll compose the inital seed by shifting and xoring
    // 4 values generated by rand().
    for (size_t i = 0; i < 4; i++)
    {
        randomSeed = randomSeed << 16 ^ rand();
    }
}


/**
 * Get a new random seed value for a new activation.
 *
 * @return A random seed value.  Each call will return a unique value.
 */
uint64_t Activity::getRandomSeed()
{
    // randomize this a little from the last version to knock things
    // off sequence.
    randomSeed = randomSeed << 16 ^ rand();
    return randomSeed;
}


/**
 * Force error termination on an activity, returning the resulting
 * REXX error code.
 *
 * @return The resulting Rexx error code.
 */
wholenumber_t Activity::error()
{
    // unwind to a base activation
    while (!topStackFrame->isStackBase())
    {
        // pop and terminate te frame
        popStackFrame(topStackFrame);
    }
    // go display
    return displayCondition(conditionobj);
}


/**
 * Force error termination on an activity, returning the resulting
 * REXX error code.
 *
 * @param activation The activation raising the error.
 * @param errorInfo  The directory containing the error condition information.
 *
 * @return The Rexx error code.
 */
wholenumber_t Activity::error(ActivationBase *activation, DirectoryClass *errorInfo)
{
    // if not passed an explicit error object, use whatever we have in our
    // local holder.
    if (errorInfo == OREF_NULL)
    {
        errorInfo = conditionobj;
    }

    // unwind to a base activation
    while (topStackFrame != activation)
    {
        // if we're not to the stack very base of the stack, terminate the frame
        popStackFrame(topStackFrame);
    }

    // go display
    return displayCondition(errorInfo);
}


/**
 * Display error information and traceback lines for a
 * Syntax condition.
 *
 * @param errorInfo The condition object with the error information
 *
 * @return The major error code for the syntax error, if this is
 *         indeed a syntax conditon.
 */
wholenumber_t Activity::displayCondition(DirectoryClass *errorInfo)
{
    // no condition object?  This is a nop
    if (errorInfo == OREF_NULL)
    {
        return 0;   // no error condition to return
    }

    RexxString *condition = (RexxString *)errorInfo->get(GlobalNames::CONDITION);
    // we only display syntax conditions
    if (condition == OREF_NULL || !condition->isEqual(GlobalNames::SYNTAX))
    {
        return 0;   // no error condition to return
    }
    // display the information
    display(errorInfo);

    // set the default return code value in case we don't have a
    // good one in the condition object.
    wholenumber_t rc = Error_Interpretation / 1000;
    // try to convert.  Leaves unchanged if not value
    errorInfo->get(GlobalNames::RC)->numberValue(rc);
    return rc;
}


/**
 * Extract an error number from a syntax condition object.
 *
 * @param conditionObject
 *               The condition object for the extract.
 *
 * @return The RC value associated with the condition.
 */
wholenumber_t Activity::errorNumber(DirectoryClass *conditionObject)
{
    wholenumber_t rc = Error_Interpretation / 1000;
    if (conditionObject != OREF_NULL)
    {
        // try to convert.  Leaves unchanged if not value
        conditionObject->get(GlobalNames::RC)->numberValue(rc);
    }
    return rc;
}


/**
 * Raise a condition, with potential trapping.
 * Also checks for potential ::OPTIONS condition overrides.
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
bool Activity::raiseCondition(RexxString *condition, RexxObject *rc, RexxObject *description, RexxObject *additional, RexxObject *result)
{
    // if we have ::OPTIONS condition SYNTAX set on the package, we
    // raise a SYNTAX error instead of raising the condition
    RexxActivation *activation = getCurrentRexxFrame();
    if (activation != OREF_NULL)
    {
        if (activation->isErrorSyntaxEnabled() && condition->strCompare(GlobalNames::ERRORNAME))
        {
            reportException(Error_Execution_error_syntax, description, result);
        }
        else if (activation->isFailureSyntaxEnabled() && condition->strCompare(GlobalNames::FAILURE))
        {
            reportException(Error_Execution_failure_syntax, description, result);
        }
        else if (activation->isLostdigitsSyntaxEnabled() && condition->strCompare(GlobalNames::LOSTDIGITS))
        {
            reportException(Error_Execution_lostdigits_syntax, description);
        }
        else if (activation->isNostringSyntaxEnabled() && condition->strCompare(GlobalNames::NOSTRING))
        {
            reportException(Error_Execution_nostring_syntax, description);
        }
        else if (activation->isNotreadySyntaxEnabled() && condition->strCompare(GlobalNames::NOTREADY))
        {
            reportException(Error_Execution_notready_syntax, description);
        }
    }

    // check the context first to see if this will be trapped.  Creating
    // the condition object is pretty expensive, so we want to avoid
    // creating this if we'll just end up throwing it away.
    if (!checkCondition(condition))
    {
        return false;
    }

    // just create a condition object and process the traps.
    DirectoryClass *conditionObj = createConditionObject(condition, rc, description, additional, result);
    return raiseCondition(conditionObj);
}


/**
 * Test if a condition will be trapped before creating a
 * condition object for it.
 *
 * @param condition
 *               The name of the condition being raised
 *
 * @return true if this will be trapped, false otherwise.
 */
bool Activity::checkCondition(RexxString *condition)
{
    // unwind the stack frame calling trap until we reach the first real Rexx activation
    for (ActivationBase *activation = getTopStackFrame();
         !activation->isStackBase();
         activation = activation->getPreviousStackFrame())
    {
        // see if there is an activation interested in trapping this.
        if (activation->willTrap(condition))
        {
            return true;
        }

        // for a normal condition, we stop checking at the first Rexx activation.
        if (isOfClass(Activation, activation))
        {
            return false;
        }
    }

    // reached the bottom of the stack and nothing handled this
    return false;
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
bool Activity::raiseCondition(DirectoryClass *conditionObj)
{
    // the condition has not been handled yet.
    bool handled = false;
    RexxString *condition = (RexxString *)conditionObj->get(GlobalNames::CONDITION);

    // unwind the stack frame calling trap until we reach the first real Rexx activation
    for (ActivationBase *activation = getTopStackFrame();
         !activation->isStackBase();
         activation = activation->getPreviousStackFrame())
    {
        // see if there is an activation interested in trapping this.
        handled = activation->trap(condition, conditionObj);
        // for a normal condition, we stop checking at the first Rexx activation.
        if (handled || isOfClass(Activation, activation))
        {
            break;
        }
    }

    // Control will not return here if the condition was trapped via
    // SIGNAL ON SYNTAX.  For CALL ON conditions, handled will be
    // true if a trap is pending.

    return handled;
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
DirectoryClass* Activity::createConditionObject(RexxString *condition, RexxObject *rc, RexxObject *description, RexxObject *additional, RexxObject *result)
{
    // condition objects are directories
    DirectoryClass *conditionObj = new_directory();
    ProtectedObject p(conditionObj);
    // fill in the provided pieces
    conditionObj->put(condition, GlobalNames::CONDITION);
    conditionObj->put(description == OREF_NULL ? GlobalNames::NULLSTRING : description, GlobalNames::DESCRIPTION);
    conditionObj->put(TheFalseObject, GlobalNames::PROPAGATED);

    // the remainders don't have defaults, so only add the items if provided.
    if (rc != OREF_NULL)
    {
        conditionObj->put(rc, GlobalNames::RC);
    }
    if (additional != OREF_NULL)
    {
        conditionObj->put(additional, GlobalNames::ADDITIONAL);
    }
    if (result != OREF_NULL)
    {
        conditionObj->put(result, GlobalNames::RESULT);
    }

    // add in all location-specific information
    generateProgramInformation(conditionObj);
    return conditionObj;
}


/**
 * Simple raise a specific error number function.
 *
 * @param errcode
 */
void Activity::reportAnException(RexxErrorCodes errcode)
{
    raiseException(errcode, OREF_NULL, OREF_NULL, OREF_NULL);
}


/**
 * Raise an error with a single object substitution in the
 * message.
 *
 * @param errcode The error code.
 * @param substitution1
 *                The substitution value.
 */
void Activity::reportAnException(RexxErrorCodes errcode, RexxObject *substitution1)
{
    raiseException(errcode, OREF_NULL, new_array(substitution1), OREF_NULL);
}


/**
 * Raise an error with two object substitutions in the message.
 *
 * @param errcode The error code.
 * @param substitution1
 *                The substitution value.
 * @param substitution2
 *                Another substitution value.
 */
void Activity::reportAnException(RexxErrorCodes errcode, RexxObject *substitution1, RexxObject *substitution2)
{
    raiseException(errcode, OREF_NULL, new_array(substitution1, substitution2), OREF_NULL);
}


/**
 * Raise an error with three object substitutions in the
 * message.
 *
 * @param errcode The error code.
 * @param substitution1
 *                The substitution value.
 * @param substitution2
 *                Another substitution value.
 */
void Activity::reportAnException(RexxErrorCodes errcode, RexxObject *substitution1, RexxObject *substitution2, RexxObject *substitution3)
{
    raiseException(errcode, OREF_NULL, new_array(substitution1, substitution2, substitution3), OREF_NULL);
}


/**
 * Raise an error with four object substitutions in the message.
 *
 * @param errcode The error code.
 * @param substitution1
 *                The substitution value.
 * @param substitution2
 *                Another substitution value.
 */
void Activity::reportAnException(RexxErrorCodes errcode, RexxObject *substitution1, RexxObject *substitution2,
                                 RexxObject *substitution3, RexxObject *substitution4)
{
    raiseException(errcode, OREF_NULL, new_array(substitution1, substitution2, substitution3, substitution4), OREF_NULL);
}


/**
 * Raise an error with four object substitutions in the message.
 *
 * @param errcode The error code.
 * @param substitution1
 *                The substitution value.
 * @param substitution2
 *                Another substitution value.
 */
void Activity::reportAnException(RexxErrorCodes errcode, const char *substitution1, RexxObject *substitution2,
                                 const char *substitution3, RexxObject *substitution4)
{
    raiseException(errcode, OREF_NULL, new_array(new_string(substitution1), substitution2, new_string(substitution3), substitution4), OREF_NULL);
}
void Activity::reportAnException(RexxErrorCodes errcode, const char *string)
{
    reportAnException(errcode, new_string(string));
}


/**
 * Raise an error using aASCII-A string as a substitutions
 * parameters
 *
 * @param errcode The error code.
 * @param string  The substitution value.
 */
void Activity::reportAnException(RexxErrorCodes errcode, const char *string1, const char *string2)
{
    reportAnException(errcode, new_string(string1), new_string(string2));
}


void Activity::reportAnException(RexxErrorCodes errcode, const char *string, wholenumber_t  integer)
{

    reportAnException(errcode, new_string(string), new_integer(integer));
}


void Activity::reportAnException(RexxErrorCodes errcode, const char *string, wholenumber_t integer, RexxObject   *obj)
{
    reportAnException(errcode, new_string(string), new_integer(integer), obj);
}


void Activity::reportAnException(RexxErrorCodes errcode, const char *string, RexxObject *obj, wholenumber_t integer)
{
    reportAnException(errcode, new_string(string), obj, new_integer(integer));
}


void Activity::reportAnException(RexxErrorCodes errcode, RexxObject *obj, wholenumber_t integer)
{
    reportAnException(errcode, obj, new_integer(integer));
}


void Activity::reportAnException(RexxErrorCodes errcode, RexxObject *obj, const char *string)
{
    reportAnException(errcode, obj, new_string(string));
}


void Activity::reportAnException(RexxErrorCodes errcode, const char *string, RexxObject *obj)
{
    reportAnException(errcode, new_string(string), obj);
}


void Activity::reportAnException(RexxErrorCodes errcode, wholenumber_t  integer)
{
    reportAnException(errcode, new_integer(integer));
}

void Activity::reportAnException(RexxErrorCodes errcode, wholenumber_t  integer, wholenumber_t  integer2)
{
    reportAnException(errcode, new_integer(integer), new_integer(integer2));
}


void Activity::reportAnException(RexxErrorCodes errcode, wholenumber_t  a1, RexxObject *a2)
{
    reportAnException(errcode, new_integer(a1), a2);
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
void Activity::raiseException(RexxErrorCodes errcode, RexxString *description, ArrayClass *additional, RexxObject *result)
{
    // during error processing, we need to request the string value of message
    // substitution objects.  It is possible that the string process will also
    // cause a syntax error, resulting in a recursion loop.  We snip that off here,
    // by disallowing a nested error condition.
    if (requestingString)
    {
        throw RecursiveStringError;
    }

    // get the top-most stack frame and also the top Rexx frame
    ActivationBase *topFrame = getTopStackFrame();
    RexxActivation *activation = getCurrentRexxFrame();
    // if we're raised within a real Rexx context, we need to deal with forwarded
    // frames
    if (topFrame == activation)
    {
        // unwind the stack until we find a frame that is not forwarded
        while (activation != OREF_NULL && activation->isForwarded())
        {
            // terminate and remove this stack frame
            popStackFrame(activation);
            // grab the new current frame
            activation = getCurrentRexxFrame();
        }
    }
    else
    {
        activation = NULL;      // raised from a native context, don't add to stack trace
    }

    // create an exception object
    conditionobj = createExceptionObject(errcode, description, additional, result);

    // and go raise as a condition
    if (!raiseCondition(conditionobj))
    {
        // we're raising a syntax error and this was not trapped by the
        // top condition.  We start propagating this condition until we either
        // find a frame that traps this or we run out of frames.
        // fill in the propagation status
        conditionobj->put(TheTrueObject, GlobalNames::PROPAGATED);
        // if we have an Rexx context to work with, unwind to that point,
        if (activation != OREF_NULL)
        {
            // unwind the frame to this point
            unwindToFrame(activation);
            popStackFrame(activation);     // remove it from the stack
        }
        // go propagate
        raisePropagate(conditionobj);
    }
}


/**
 * Create a new error exception object for a SYNTAX error
 *
 * @param errcode    The error code to raise.
 * @param description
 *                   The description string.
 * @param additional Message substitution information.
 * @param result     The result object.
 *
 * @return The created exception dictionary.
 */
DirectoryClass* Activity::createExceptionObject(RexxErrorCodes errcode,
                                                RexxString *description, ArrayClass *additional, RexxObject *result)
{
    // build an exception object for the SYNTAX error
    DirectoryClass *exobj = (DirectoryClass *)new_directory();
    // this is the anchor for anything else
    ProtectedObject p(exobj);

    // error codes are handled as a composite number.  The lowest 3 digits
    // are th minor code.  The major code is above that.
    wholenumber_t primary = (errcode / 1000) * 1000;

    char work[32];

    // get a version of the error code formatted in "dot" format.
    sprintf(work, "%d.%1zd", errcode / 1000, errcode - primary);
    RexxString *code = new_string(work);
    exobj->put(code, GlobalNames::CODE);

    // now the primary code goes in as RC
    wholenumber_t newVal = primary / 1000;
    RexxInteger *rc = new_integer(newVal);
    exobj->put(rc, GlobalNames::RC);

    // get the text for the primary error message
    RexxString *errortext = Interpreter::getMessageText(primary);
    // we can have an error for the error!
    if (errortext == OREF_NULL)
    {
        reportException(Error_Execution_error_condition, code);
    }
    exobj->put(errortext, GlobalNames::ERRORTEXT);

    // handle the message substitution values (raw form)
    // only the secondary message has substitutions, but we
    // fill in substitutions parameters into the condition object
    // anyway.
    if (additional == OREF_NULL)
    {
        additional = new_array((size_t)0);
    }

    // add this in
    exobj->put(additional, GlobalNames::ADDITIONAL);

    // do we have a secondary message?
    if (primary != errcode)
    {
        // build the message and add to the condition object
        RexxString *message = buildMessage(errcode, additional);
        exobj->put(message, GlobalNames::MESSAGE);
    }
    else
    {
        // make this explicitly .nil
        exobj->put(TheNilObject, GlobalNames::MESSAGE);
    }

    // the description string (rare for exceptions)
    if (description == OREF_NULL)
    {
        // use an explicit null string
        exobj->put(GlobalNames::NULLSTRING, GlobalNames::DESCRIPTION);
    }
    else
    {
        exobj->put(description, GlobalNames::DESCRIPTION);
    }

    if (result != OREF_NULL)
    {
        exobj->put(result, GlobalNames::RESULT);
    }

    // add in all location-specific information
    generateProgramInformation(exobj);

    // the condition name is always SYNTAX
    exobj->put(GlobalNames::SYNTAX, GlobalNames::CONDITION);
    // fill in the propagation status.  This is always false for the first
    // potential trap level, gets turned to true if this goes down levels.
    exobj->put(TheFalseObject, GlobalNames::PROPAGATED);

    return exobj;
}


/**
 * Add program location-specific information to a condition object.
 *
 * @param exobj  The exception object being constructed.
 */
void Activity::generateProgramInformation(DirectoryClass *exobj)
{
    // create lists for both the stack frames and the traceback lines
    ListClass *stackFrames = new_list();
    exobj->put(stackFrames, GlobalNames::STACKFRAMES);
    ListClass *traceback = new_list();
    exobj->put(traceback, GlobalNames::TRACEBACK);

    ActivationFrame *frame = activationFrames;

    PackageClass *package = OREF_NULL;
    StackFrameClass *firstFrame = OREF_NULL;

    while (frame != NULL)
    {
        StackFrameClass *stackFrame = frame->createStackFrame();
        // save the topmost package object we can find for error reporting
        if (package == OREF_NULL && frame->getPackage() != OREF_NULL)
        {
            firstFrame = stackFrame;
            package = frame->getPackage();
        }
        stackFrames->append(stackFrame);
        traceback->append(stackFrame->getTraceLine());
        frame = frame->next;
    }

    if (firstFrame != OREF_NULL)
    {
        RexxObject *lineNumber = firstFrame->getLine();
        if (lineNumber != TheNilObject)
        {
            // add the line number information
            exobj->put(lineNumber, GlobalNames::POSITION);
        }
    }

    // if we have source, and this is not part of the interpreter image,
    // add program information
    if (package != OREF_NULL)
    {
        exobj->put(package->getProgramName(), GlobalNames::PROGRAM);
        exobj->put(package, GlobalNames::PACKAGE_REF);
    }
    else
    {
        // if not available, then this is explicitly a NULLSTRING.
        exobj->put(GlobalNames::NULLSTRING, GlobalNames::PROGRAM);
    }
}


/**
 * Generate a list of stack frames for an Exception object.
 *
 * @param skipFirst Determines if we should skip the first frame.  Used primarily
 *                  for the RexxContext stackFrames() method to avoid returning
 *                  the stackframes method as the first item.
 *
 * @return An array of the stack frames in the call context.
 */
ArrayClass* Activity::generateStackFrames(bool skipFirst)
{
    // create lists for both the stack frames and the traceback lines
    ArrayClass *stackFrames = new_array((size_t)0);
    ProtectedObject p(stackFrames);

    ActivationFrame *frame = activationFrames;

    while (frame != NULL)
    {
        // if asked to skip the first frame, just turn the flag off
        // and go around again
        if (skipFirst)
        {
            skipFirst = false;
        }
        else
        {
            StackFrameClass *stackFrame = frame->createStackFrame();
            stackFrames->append(stackFrame);
        }
        frame = frame->next;
    }
    return stackFrames;
}


/**
 * Build a message and perform the indicated substitutions.
 *
 * @param messageCode
 *               The target message code
 * @param substitutions
 *               An array of substitution values
 *
 * @return The message with the substitution values inserted.
 */
RexxString* Activity::buildMessage(wholenumber_t messageCode, ArrayClass *substitutions)
{
    // retrieve the secondary message
    RexxString *message = Interpreter::getMessageText(messageCode);
    // bad message if we can't find this
    if (message == OREF_NULL)
    {
        reportException(Error_Execution_error_condition, messageCode);
    }
    // do required substitutions
    return messageSubstitution(message, substitutions);
}


/**
 * Perform any required message substitutions on the secondary
 * error message.
 *
 * @param message    The error text.  Substitution parameters are marked
 *                   inside the messages.
 * @param additional The array of values to substitute.
 *
 * @return The formatted string.
 */
RexxString* Activity::messageSubstitution(RexxString *message, ArrayClass  *additional)
{
    size_t substitutions = additional->size();
    // build this up into a mutable buffer.
    Protected<MutableBuffer> newMessage = new MutableBuffer();

    const char *messageData = message->getStringData();
    size_t searchOffset = 0;

    for (size_t i = 1; i <= substitutions; i++)
    {
        // search for the substibution value
        size_t subposition = message->pos(GlobalNames::AND, searchOffset);
        // no more '&' markers found in the message, we're done building.
        if (subposition == 0)
        {
            break;
        }

        // append the next message section to the buffer
        newMessage->append(messageData + searchOffset, subposition - searchOffset - 1);
        // this will be where we start searching for the next one.
        searchOffset = subposition + 1;

        // get the character following the '&'.  This should be a numeric
        // substitution number.  We only support digits 1-9.
        // NOTE:  pos returns the position origin 1, but getChar()
        // is origin 0.  This really picks up the next character.
        size_t selector = message->getChar(subposition);
        RexxString *stringVal = GlobalNames::NULLSTRING;
        // if this a bad selector, substitute anyway, but give a generic marker
        if (selector < '0' || selector > '9')
        {
            stringVal = new_string("<BAD MESSAGE>");
        }
        else
        {
            // now get the value from the substitutions array
            selector -= '0';
            // still in range?  Convert to a string value and add to the message
            if (selector <= substitutions)
            {
                RexxInternalObject *value = additional->get(selector);
                if (value != OREF_NULL)
                {
                    // do this carefully, we need to try to avoid recursion errors
                    requestingString = true;
                    stackcheck = false;
                    // save the actitivation level in case there's an error unwind for an unhandled
                    // exception;
                    size_t activityLevel = getActivationLevel();
                    // do this under a try block to intercept problems
                    try
                    {
                        stringVal = value->stringValue();
                    }
                    catch (ActivityException)
                    {
                        // we use the default object name if there are any problems
                        stringVal = value->defaultName();
                    }

                    // make sure we get restored to the same base activation level.
                    restoreActivationLevel(activityLevel);
                    // returning to normal operations.
                    requestingString = false;
                    stackcheck = true;
                }
            }
        }
        // add on the message substitutions
        newMessage->append(stringVal);
    }
    // append the remainder of the message to the buffer and convert to a string.
    newMessage->append(messageData + searchOffset, message->getLength() - searchOffset);
    return newMessage->makeString();
}


/**
 * Reraise an exception object in a prior context.
 *
 * @param exobj  The exception object with the syntax information.
 */
void Activity::reraiseException(DirectoryClass *exobj)
{
    RexxActivation *activation = getCurrentRexxFrame();
    // have a target activation?  Replace the
    // package information from the original with the current.
    if (activation != OREF_NULL)
    {
        PackageClass *package = activation->getPackageObject();
        // set the position and program name
        exobj->put(new_integer(activation->currentLine()), GlobalNames::POSITION);
        exobj->put(package->getProgramName(), GlobalNames::PROGRAM);
        exobj->put(package, GlobalNames::PACKAGE_REF);
    }
    else
    {
        // remove the old package information.
        exobj->remove(GlobalNames::POSITION);
        exobj->remove(GlobalNames::PROGRAM);
        exobj->remove(GlobalNames::PACKAGE_REF);
    }

    // get the error code and redo the message information
    RexxInternalObject *errorcode = exobj->get(GlobalNames::CODE);
    wholenumber_t errornumber = Interpreter::messageNumber((RexxString *)errorcode);

    wholenumber_t primary = (errornumber / 1000) * 1000;
    if (errornumber != primary)
    {
        char work[22];
        sprintf(work, "%1zd%3.3zd", errornumber / 1000, errornumber - primary);
        errornumber = atol(work);

        RexxString *message = Interpreter::getMessageText(errornumber);
        ArrayClass *additional = (ArrayClass *)exobj->get(GlobalNames::ADDITIONAL);
        message = messageSubstitution(message, additional);
        // replace the original message text
        exobj->put(message, GlobalNames::MESSAGE);
    }

    raisePropagate(exobj);
}


/**
 * Propagate a condition down the chain of activations
 *
 * @param conditionObj
 *               The condition object.
 */
void Activity::raisePropagate(DirectoryClass *conditionObj)
{
    RexxString *condition = (RexxString *)conditionObj->get(GlobalNames::CONDITION);
    ActivationBase *activation = getTopStackFrame();

    // loop to the top of the stack
    while (activation != OREF_NULL)
    {
        // give this one a chance to trap (will never return for trapped
        // PROPAGATE conditions).  These will be syntax errors and can only be
        // trapped via SIGNAL.
        activation->trap(condition, conditionObj);
        // make sure this is marked as propagated after the first...probably
        // already done, but it doesn't hurt to ensure this.
        conditionObj->put(TheTrueObject, GlobalNames::PROPAGATED);
        // if we've unwound to the stack base and not been trapped, we need
        // to fall through to the kill processing.  The stackbase should have trapped
        // this.  We never cleanup the stackbase activation though.
        if (activation->isStackBase())
        {
            break;
        }
        // clean up this stack frame
        popStackFrame(activation);
        activation = getTopStackFrame();
    }
    // not trapped, so kill the activity for this error
    kill(conditionObj);
}

/**
 * Display information from an exception object.
 *
 * @param exobj  The exception object.
 */
void Activity::display(DirectoryClass *exobj)
{
    // get the traceback info
    ListClass *trace_backList = (ListClass *)exobj->get(GlobalNames::TRACEBACK);
    if (trace_backList != OREF_NULL)
    {
        ArrayClass *trace_back = trace_backList->makeArray();
        ProtectedObject p(trace_back);

        // display each of the traceback lines
        size_t tracebackSize = trace_back->size();

        for (size_t i = 1; i <= tracebackSize; i++)
        {
            RexxString *text = (RexxString *)trace_back->get(i);
            // if we have a real line, write it out
            if (text != OREF_NULL && text != TheNilObject)
            {
                traceOutput(currentRexxFrame, text);
            }
        }
    }

    // get the error code, and format a message header
    RexxString *rc = (RexxString *)exobj->get(GlobalNames::RC);
    wholenumber_t errorCode = Interpreter::messageNumber(rc);
    Protected<RexxString> text = Interpreter::getMessageText(Message_Translations_error);

    // get the program name
    RexxString *programname = (RexxString *)exobj->get(GlobalNames::PROGRAM);
    // add on the error number
    text = text->concatWith(rc->requestString(), ' ');

    // add on the program name if we have one.
    if (programname != OREF_NULL && programname != GlobalNames::NULLSTRING)
    {
        text = text->concatWith(Interpreter::getMessageText(Message_Translations_running), ' ');
        text = text->concatWith(programname, ' ');

        // if we have a line position, add that on also
        RexxInternalObject *position = exobj->get(GlobalNames::POSITION);
        if (position != OREF_NULL)
        {
            text = text->concatWith(Interpreter::getMessageText(Message_Translations_line), ' ');
            text = text->concatWith(position->requestString(), ' ');
        }
    }
    text = text->concatWithCstring(":  ");

    // and finally the primary error message
    text = text->concat((RexxString *)exobj->get(GlobalNames::ERRORTEXT));
    traceOutput(currentRexxFrame, text);

    // now add the secondary message if we have one.
    RexxString *secondary = (RexxString *)exobj->get(GlobalNames::MESSAGE);
    if (secondary != OREF_NULL && secondary != (RexxString *)TheNilObject)
    {
        rc = (RexxString *)exobj->get(GlobalNames::CODE);
        errorCode = Interpreter::messageNumber(rc);
        text = Interpreter::getMessageText(Message_Translations_error);

        text = text->concatWith((RexxString *)rc, ' ');
        text = text->concatWithCstring(":  ");
        text = text->concat(secondary);

        // and write that out also
        traceOutput(currentRexxFrame, text);
    }
}


/**
 * Display information about an error in interactive debug.
 *
 * @param exobj  The exception object.
 *
 * @return
 */
void Activity::displayDebug(DirectoryClass *exobj)
{
    // get the leading part to indicate this is a debug error, then compose the full
    // message
    RexxString *text = Interpreter::getMessageText(Message_Translations_debug_error);
    text = text->concatWith((exobj->get(GlobalNames::RC))->requestString(), ' ');
    text = text->concatWithCstring(":  ");
    text = text->concatWith((RexxString *)exobj->get(GlobalNames::ERRORTEXT), ' ');
    traceOutput(currentRexxFrame, text);


    // now any secondary message
    RexxString *secondary = (RexxString *)exobj->get(GlobalNames::MESSAGE);
    if (secondary != OREF_NULL && secondary != (RexxString *)TheNilObject)
    {
        text = Interpreter::getMessageText(Message_Translations_debug_error);
        text = text->concatWith((RexxString *)exobj->get(GlobalNames::CODE), ' ');
        text = text->concatWithCstring(":  ");
        text = text->concat(secondary);
        traceOutput(getCurrentRexxFrame(), text);
    }
}


/**
 * Release an activity to run
 */
void Activity::run()
{
    // post both of the semaphores.  The activity will be waiting
    // on one of these on another thread, so we yield control to
    // give the other thread a chance to run.
    guardSem.post();
    runSem.post();
    yield();
}



/**
 * Run a message object on a spawned thread.
 *
 * @param target The target message object.
 */
void Activity::run(MessageClass *target)
{
    // set the dispatch message, then go release the thread to
    // start running.
    dispatchMessage = target;
    run();
}


/**
 * Check the activation stack to see if we need to expand the size.
 */
void Activity::checkActivationStack()
{
    // no room for a new stack frame?  need to expand the stack
    if (stackFrameDepth == activationStackSize)
    {
        // allocate a larger stack
        InternalStack *newstack = new_internalstack(activationStackSize + ACT_STACK_SIZE);
        // now copy all of the entries over to the new frame stack
        for (size_t i = activationStackSize; i != 0; i--)
        {
            newstack->push(activations->peek(i - 1));
        }
        // update the frame information
        activations = newstack;
        activationStackSize += ACT_STACK_SIZE;
    }
}


/**
 * Update the top of the stack markers after a push or a pop
 * operation on the stack frame.
 */
void Activity::updateFrameMarkers()
{
    // we have a new top entry...get this from the stack and adjust
    // the markers appropriately
    topStackFrame = (ActivationBase *)activations->getTop();
    // the new activation is the new top and there may or may not be
    // a rexx context to deal with
    currentRexxFrame = topStackFrame->findRexxContext();
    ;

    // update the numeric settings
    numericSettings = topStackFrame->getNumericSettings();
    // this should be true, but make sure we don't clobber the global settings accidentally
    if (ActivityManager::currentActivity == this)
    {
        Numerics::setCurrentSettings(numericSettings);
    }
}


/**
 * Push a Rexx code activation on to the stack frame.
 *
 * @param new_activation
 *               The new activation to add.
 */
void Activity::pushStackFrame(ActivationBase *new_activation)
{
    checkActivationStack();         // make sure the stack is not filled
    // push on to the stack and bump the depth
    activations->push(new_activation);
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
 * The new frame will have a NativeActivation that's marked
 * as a stack base frame.  Additional call frames are pushed on
 * top of that activation.  Any operations that unwind the
 * stack frames will stop when they hit the activation stack
 * base.
 */
void Activity::createNewActivationStack()
{
    // make sure we have a new stack
    checkActivationStack();
    // This is a root activation that will allow API functions to be called
    // on this thread without having an active bit of ooRexx code first.
    NativeActivation *new_activation = ActivityManager::newNativeActivation(this);
    // this indicates this is a new stack basepoint. Error/rollbacks will terminate
    // at this point.
    new_activation->setStackBase();
    // create a new root element on the stack and bump the depth indicator
    activations->push(new_activation);
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
void Activity::popStackFrame(bool  reply)
{
    // pop off the top elements and reduce the depth
    ActivationBase *poppedStackFrame = (ActivationBase *)activations->pop();
    stackFrameDepth--;

    // did we just pop off the last element of a stack frame?  This should not happen, so
    // push it back on to the stack
    if (poppedStackFrame->isStackBase())
    {
        activations->push(poppedStackFrame);
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
            // the popped stack frame might still be in the save stack, but can
            // also contain pointers back to locations on the C stack.  Make sure
            // that this never tries to mark anything in case of a garbage collection
            poppedStackFrame->setHasNoReferences();
        }
    }
}


/**
 * Clean up a popped stack frame.
 *
 * @param poppedStackFrame
 */
void Activity::cleanupStackFrame(ActivationBase *poppedStackFrame)
{
    // make sure this frame is terminated first
    poppedStackFrame->termination();
    // ensure this never marks anything
    poppedStackFrame->setHasNoReferences();
}


/**
 * Pop entries off the stack frame upto and including the
 * target activation.
 *
 * @param target The target for the pop operation.
 */
void Activity::popStackFrame(ActivationBase *target)
{
    ActivationBase *poppedStackFrame = (ActivationBase *)activations->pop();
    stackFrameDepth--;
    // pop off the top elements and reduce the depth
    while (poppedStackFrame != target)
    {
        // clean this up and potentially cache
        cleanupStackFrame(poppedStackFrame);
        poppedStackFrame = (ActivationBase *)activations->pop();
        stackFrameDepth--;
    }

    // clean this up and potentially cache
    cleanupStackFrame(poppedStackFrame);
    // update the frame information.
    updateFrameMarkers();
}


/**
 * Unwind the stack frames back to the base
 * frame.
 */
void Activity::unwindStackFrame()
{
    // pop activations off until we find the one at the base of the stack.
    while (stackFrameDepth > 0)
    {
        // check the top activation.  If it's a stack base item, then
        // we've reached the unwind point.
        ActivationBase *poppedActivation = (ActivationBase *)activations->pop();
        stackFrameDepth--;
        if (poppedActivation->isStackBase())
        {
            // at the very base of the activity, we keep a base item.  If this
            // is the bottom stack frame here, then push it back on.
            if (stackFrameDepth == 0)
            {
                activations->push(poppedActivation);
                stackFrameDepth++;
            }
            break;
        }
    }

    // update the frame information.
    updateFrameMarkers();
}


/**
 * Unwind the stack frame down to a given depth.
 *
 * @param depth  The target frame depth.
 */
void Activity::unwindToDepth(size_t depth)
{
    // pop elements until we unwind to the target
    while (stackFrameDepth > depth)
    {
        activations->pop();
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
void Activity::unwindToFrame(RexxActivation *frame)
{
    ActivationBase *activation;

    // keep popping frames until we find the tarte frame
    while ((activation = getTopStackFrame()) != frame)
    {
        popStackFrame(activation);
    }
}


/**
 * Set up an activity as a root activity used either for a main
 * interpreter thread or an attached thread.
 *
 * @param interpreter
 *               The interpreter instance this thread belongs to.
 */
void Activity::setupAttachedActivity(InterpreterInstance *interpreter)
{
    // associate this with the instance
    addToInstance(interpreter);

    // mark this as an attached activity
    attachCount++;
    // also mark this as being on a thread not originally controlled by the
    // interpreter. This indicates that the base of the activity is the attach point
    newThreadAttached = true;
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
void Activity::addToInstance(InterpreterInstance *interpreter)
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
void Activity::setupExits()
{
    // set the appropriate exit interlocks
    queryTrcHlt();
}


/**
 * Complete initialization of the thread context function
 * vector by filling in the constant objects.
 */
void Activity::initializeThreadContext()
{
    threadContextFunctions.RexxNil = (RexxObjectPtr)TheNilObject;
    threadContextFunctions.RexxTrue = (RexxObjectPtr)TheTrueObject;
    threadContextFunctions.RexxFalse = (RexxObjectPtr)TheFalseObject;
    threadContextFunctions.RexxNullString = (RexxStringObject)GlobalNames::NULLSTRING;
}


/**
 * Detach a thread from the interpreter instance,
 */
void Activity::detachThread()
{
    instance->detachThread(this);
}


/**
 * Cleanup the resources for a detached activity, including
 * removing the suspended state from a pushed activity nest.
 */
void Activity::detachInstance()
{
    // Undo this attached status
    instance = OREF_NULL;

    // clear the attach trackers
    attachCount = 0;
    newThreadAttached = false;
    // if there's a nesting situation, restore the activity to active state.
    if (nestedActivity != OREF_NULL)
    {
        nestedActivity->setSuspended(false);
    }
    nestedActivity = OREF_NULL;

    // also clean up references to other objects anchored in this activity
    oldActivity = OREF_NULL;
    conditionobj = OREF_NULL;
    dispatchMessage = OREF_NULL;
    waitingObject = OREF_NULL;
}


/**
 * Handle a nested attach on an Activity. In this situation,
 * a program has exited the Rexx code to native code, which
 * then has used AttachThread() to access API services. We
 * Reuse the activity, but push another NativeActivation
 * instance on to the stack to anchor allocated objects.
 */
void Activity::nestAttach()
{
    attachCount++;
    // Creating the activation stack needs to allocate objects, so we need go grab
    // the kernel lock before doing this
    requestAccess();
    // create the base marker for anchoring any objects returned by
    // this instance.
    createNewActivationStack();
    // we're in a state here where we need the access for a short window to
    // allocate the objects for the stack, but we will be requesting it again
    // once the nesting is complete, so we need to release it again now.
    releaseAccess();
}


/**
 * Return from a nested attach. We need to pop our dummy
 * native activation from the stack so that objects in the
 * local reference table can be GC'd.
 */
void Activity::returnAttach()
{
    attachCount--;
    // remove the stack frame we created as a GC anchor so that objects
    // obtained using this nested thread instance can be GC'd.

    // This is just a precaution. It might be possible that there are other activations
    // present, so make sure we get back to a base activation.
    while (!topStackFrame->isStackBase())
    {
        // if we're not to the very base of the stack, terminate the frame
        popStackFrame(topStackFrame);
    }

    // NB: popStackframe has protections against popping a stack base activation,
    // so we need to handle this activity here otherwise this ends up being a NOP.
    ActivationBase *poppedStackFrame = (ActivationBase *)activations->pop();
    stackFrameDepth--;
    // the popped stack frame might still be in the save stack, but can
    // also contain pointers back to locations on the C stack.  Make sure
    // that this never tries to mark anything in case of a garbage collection
    poppedStackFrame->setHasNoReferences();

    // and make sure all of the frame markers are reset.
    updateFrameMarkers();
}


/**
 * Release the kernel access because code is going to "leave"
 * the kernel.  This prepares for this by adding a native
 * activation on to the stack to act as a server for the
 * external call.  This way new native methods do not need to
 * be created just to get an activation created
 */
void Activity::exitKernel()
{
    // create new activation frame using the current Rexx frame (which can be null, but
    // is not likely to be).
    NativeActivation *new_activation = ActivityManager::newNativeActivation(this, currentRexxFrame);
    // this becomes the new top activation.  We also turn on the variable pool for
    // this situation.
    pushStackFrame(new_activation);
    new_activation->enableVariablepool();
    // release the kernel in preparation for exiting
    releaseAccess();
}


/**
 * Recover the kernel access and pop the native activation
 * created by activity_exit_kernel from the activity stack.
 */
void Activity::enterKernel()
{
    requestAccess();
    popStackFrame(false);
}


/**
 * Check to see if the target activity is part of the same
 * activity stack on a thread.
 *
 * @param target The activity to check.
 *
 * @return true if the target is the same as the argument activity
 *         or if the argument activity is a parent activity to
 *         the argument activity due to AttachThread nesting.
 */
bool Activity::isSameActivityStack(Activity *target)
{
    // just compare the thread ids activity is associated with
    return target->isThread(threadIdMethod());
}


/**
 * Check for a circular wait dead lock error
 *
 * @param targetActivity
 *               The target activity.
 */
void Activity::checkDeadLock(Activity *targetActivity)
{
    Activity *owningActivity;
    // are we currently waiting on something?
    if (waitingObject != OREF_NULL)
    {
        // there are only a few object types we can wait on.  Each
        // holds the activity that currently holds the lock
        if (isOfClass(Message, waitingObject))
        {
            owningActivity = ((MessageClass *)waitingObject)->getActivity();
        }
        else
        {
            owningActivity = ((VariableDictionary *)waitingObject)->getReservingActivity();
        }
        // do we have a curcular wait?
        if (owningActivity == targetActivity)
        {
            reportException(Error_Execution_deadlock);
        }
        // if we have an owning activity, have it perform a check also
        if (owningActivity != OREF_NULL)
        {
            owningActivity->checkDeadLock(targetActivity);
        }
    }
}


/**
 * Wait for a new run event to occur
 *
 * @param resource The object we're waiting on (used for deadlock detection)
 */
void Activity::waitReserve(RexxInternalObject *resource)
{
    // We use the guard semaphore both for waiting to wakeup for guard expression
    // evaluation and also for obtaining the guard lock
    guardSem.reset();
    waitingObject = resource;
    // release the interpreter lock and wait for access.  Don't continue
    // until we get the lock back
    releaseAccess();
    waitForGuardPermission();
    requestAccess();
}


/**
 * Wait for a guard post event
 */
void Activity::guardWait()
{
    // we release the access while we are waiting so something
    // can actually run to post the change event.
    releaseAccess();
    waitForGuardPermission();
    requestAccess();
}


/**
 * Post a guard expression wake up notice
 */
void Activity::guardPost()
{
    waitingObject = OREF_NULL;
    guardSem.post();
}


/**
 * Clear a guard expression semaphore in preparation to perform a
 * guard wait
 */
void Activity::guardSet()
{
    guardSem.reset();
}


/**
 * Post an activities run semaphore
 */
void Activity::postDispatch()
{
    runSem.post();
    dispatchPosted = true;
}


/**
 * Kill a running activity,
 *
 * @param conditionObj
 *               The condition object associated with the kill reason.
 */
void Activity::kill(DirectoryClass *conditionObj)
{
    // save the condition object and unwind to the beginning.
    conditionobj = conditionObj;
    throw UnhandledCondition;
}


/**
 * Relinquish to other system processes
 */
void Activity::relinquish()
{
    ActivityManager::relinquish(this);
}

/**
 * Tap the current running activation on this activity to
 * give up control at the next reasonsable boundary.
 */
void Activity::yield()
{
    // get the current rexx frame and request that it yield control
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
bool Activity::halt(RexxString *d)
{
    // raise the halt condition in the top Rexx frame
    RexxActivation *activation = currentRexxFrame;

    if (activation != NULL)
    {
        // please make it stop :-)
        return activation->halt(d);
    }
    return true;
}


/**
 * Tap the current running activation to turn on tracing as soon
 * as possible.
 *
 * @param on     Indicates whether we are turning trace on or off.
 *
 * @return true if we have an activation to tell to stop, false if the
 *         activity's not really working.
 */
bool Activity::setTrace(bool on)
{

    RexxActivation *activation = currentRexxFrame;

    if (activation != NULL)
    {
        if (on)
        {

            activation->externalTraceOn();
        }
        else
        {

            activation->externalTraceOff();
        }
        return true;
    }
    return false;
}


/**
 * Release exclusive access to the kernel
 */
void Activity::releaseAccess()
{
    // make sure we're really the holder on this
    if (ActivityManager::currentActivity == this)
    {
        // reset the numeric settings
        Numerics::setDefaultSettings();
        ActivityManager::releaseAccess();
    }
}


/**
 * Acquire exclusive access to the kernel
 */
void Activity::requestAccess()
{
    // try the fast version first
    if (ActivityManager::lockKernelImmediate())
    {
        setupCurrentActivity();
        return;
    }
    // can't get it, go stand in line
    ActivityManager::addWaitingActivity(this, false);
}


/**
 * Acquire priority API exclusive access to the kernel
 */
void Activity::requestApiAccess()
{
    // try the fast version first
    if (ActivityManager::lockKernelImmediate())
    {
        setupCurrentActivity();
        return;
    }
    // indicate we're waiting with priority
    setWaitingForApiAccess();
    // can't get it, go stand in line
    ActivityManager::addWaitingApiActivity(this);
    // and clear that waiting state
    clearWaitingForApiAccess();
}


/**
 * Perform the actual wait for kernel access for this activity;
 */
void Activity::waitForKernel()
{
    // we need to set a semaphore to flag that we are waiting
    // on a semaphore in case there is a Windows message queue recursive
    // reentry.
    waitingOnSemaphore = true;
    ActivityManager::lockKernel();
    waitingOnSemaphore = false;
}


/**
 * Perform final setup for an activity as the current activity.
 * This assumes the kernel lock has already been obtained.
 */
void Activity::setupCurrentActivity()
{
    DispatchSection lock;

    // update the current activity pointer and the global numeric settings.
    ActivityManager::currentActivity = this;
    Numerics::setCurrentSettings(numericSettings);
}

void Activity::checkStackSpace()
/******************************************************************************/
/* Function:  Make sure there is enough stack space to run a method           */
/******************************************************************************/
{
#ifdef STACKCHECK
    size_t temp;                          // if checking and there isn't room
    if (((char *)&temp - (char *)stackBase) < MIN_C_STACK && stackcheck == true)
    {
        reportException(Error_Control_stack_full);
    }
#endif
}


/**
 * Retrieve the .local directory for the current activity.
 *
 * @return The .local directory.
 */
DirectoryClass *Activity::getLocal()
{
    // the interpreter instance owns this
    return instance->getLocal();
}


/**
 * Return the activity's thread id.
 *
 * @return
 */
thread_id_t  Activity::threadIdMethod()
{
    return currentThread.getThreadID();
}


/**
 * Determine if Halt or Trace System exits are set
 * and set a flag to indicate this.
 */
void Activity::queryTrcHlt()
{
    clauseExitUsed = false;
    if (isExitEnabled(RXHLT))
    {
        clauseExitUsed = true;
    }
    if (isExitEnabled(RXTRC))
    {
        clauseExitUsed = true;
    }
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
bool Activity::callExit(RexxActivation * activation,
    const char *exitName, int function,
    int subfunction, void *exitbuffer)
{
    ExitHandler &handler = getExitHandler(function);

    int rc = handler.call(this, activation, function, subfunction, exitbuffer);

    // did we get an error situation back?
    if (rc == RXEXIT_RAISE_ERROR || rc < 0)
    {
        // if this is the I/O function, we disable that exit to prevent
        // recursive error conditions.
        if (function == RXSIO)
        {
            disableExit(RXSIO);
        }
        // rais the system service error
        reportException(Error_System_service_service, exitName);
    }
    return rc == RXEXIT_HANDLED;
}


/**
 * Call the initialization exit
 *
 * @param activation The activation context for the top level.
 */
void Activity::callInitializationExit(RexxActivation *activation)
{
    if (isExitEnabled(RXINI))  // is the exit enabled?
    {
        callExit(activation, "RXINI", RXINI, RXINIEXT, NULL);
    }
}


/**
 * Call the termination exit for a top-level program.
 *
 * @param activation The activaiton context.
 */
void Activity::callTerminationExit(RexxActivation *activation)
{
    if (isExitEnabled(RXTER))
    {
        callExit(activation, "RXTER", RXTER, RXTEREXT, NULL);
    }
}


/**
 * Call the sytem I/O exit for say output.
 *
 * @param activation The source activation context.
 * @param sayoutput  The output string.
 *
 * @return The handled/not handled flag.
 */
bool  Activity::callSayExit(RexxActivation *activation, RexxString *sayoutput)
{
    if (isExitEnabled(RXSIO))
    {
        RXSIOSAY_PARM exit_parm;

        sayoutput->toRxstring(exit_parm.rxsio_string);
        return !callExit(activation, "RXSIO", RXSIO, RXSIOSAY, &exit_parm);
    }
    return true;
}


/**
 * Call the trace output I/O exit.
 *
 * @param activation The activation context.
 * @param traceoutput
 *                   The trace line.
 *
 * @return The handled flag.
 */
bool Activity::callTraceExit(RexxActivation *activation, RexxString *traceoutput)
{
    if (isExitEnabled(RXSIO))  // is the exit enabled?
    {
        RXSIOSAY_PARM exit_parm;
        traceoutput->toRxstring(exit_parm.rxsio_string);
        return !callExit(activation, "RXSIO", RXSIO, RXSIOTRC, &exit_parm);
    }
    return true;
}


/**
 * Request input from the terminal input exit.
 *
 * @param activation The activation context.
 * @param inputstring
 *                   The returned intput string.
 *
 * @return The handled indicator.
 */
bool Activity::callTerminalInputExit(RexxActivation *activation, RexxString *&inputstring)
{
    if (isExitEnabled(RXSIO))
    {
        RXSIOTRD_PARM exit_parm;
        char retbuffer[DEFRXSTRING];

        *retbuffer = '\0';
        // we send along an rxstring value for a return result.
        MAKERXSTRING(exit_parm.rxsiotrd_retc, retbuffer, DEFRXSTRING);
        if (!callExit(activation, "RXSIO", RXSIO, RXSIOTRD, &exit_parm))
        {
            return true;
        }
        // process the return value
        inputstring = new_string(exit_parm.rxsiotrd_retc);
        if (exit_parm.rxsiotrd_retc.strptr != retbuffer)
        {
            SystemInterpreter::releaseResultMemory(exit_parm.rxsiotrd_retc.strptr);

        }
        return false;
    }
    return true;
}


/**
 * Retrieve an input string from the debug input exit.
 *
 * @param activation The activation context.
 * @param inputstring
 *                   The retrieved inputstring.
 *
 * @return The handled flag.
 */
bool Activity::callDebugInputExit(RexxActivation *activation, RexxString *&inputstring)
{
    if (isExitEnabled(RXSIO))
    {
        RXSIOTRD_PARM exit_parm;
        char retbuffer[DEFRXSTRING];

        *retbuffer = '\0';
        MAKERXSTRING(exit_parm.rxsiotrd_retc, retbuffer, DEFRXSTRING);
        if (!callExit(activation, "RXSIO", RXSIO, RXSIODTR, &exit_parm))
        {
            return true;
        }
        inputstring = new_string(exit_parm.rxsiotrd_retc);
        if (exit_parm.rxsiotrd_retc.strptr != retbuffer)
        {
            SystemInterpreter::releaseResultMemory(exit_parm.rxsiotrd_retc.strptr);

        }
        return false;
    }
    return true;
}


/**
 * Call the function exit to handle a function call.
 *
 * @param activation The activation context.
 * @param rname      The call target name.
 * @param isFunction true if this is a function call.
 * @param funcresult The returned function result.
 * @param arguments  The array of function arguments.
 * @param argcount   The count of arguments.
 *
 * @return The handled flag.
 */
bool Activity::callFunctionExit(RexxActivation *activation, RexxString *rname, bool isFunction,
    ProtectedObject &funcresult, RexxObject **arguments, size_t argcount)
{

    if (isExitEnabled(RXFNC))
    {
        RXFNCCAL_PARM exit_parm;
        char retbuffer[DEFRXSTRING];

        // clear the error codes
        exit_parm.rxfnc_flags.rxfferr = 0;
        exit_parm.rxfnc_flags.rxffnfnd = 0;

        // set the call type flag
        exit_parm.rxfnc_flags.rxffsub = isFunction ? 0 : 1;

        // requires a number of rxstring values on output
        exit_parm.rxfnc_namel = (unsigned short)rname->getLength();
        exit_parm.rxfnc_name = rname->getStringData();

        // Get current active queue name
        RexxString *stdqueue = Interpreter::getCurrentQueue();
        exit_parm.rxfnc_que = stdqueue->getStringData();
        exit_parm.rxfnc_quel = (unsigned short)stdqueue->getLength();

        // build an array of RXSTRINGs to hold the arguments
        exit_parm.rxfnc_argc = (unsigned short)argcount;

        // allocate enough memory for all arguments.
        // At least one item needs to be allocated in order to avoid an error
        // in the sysexithandler!
        PCONSTRXSTRING argrxarray = (PCONSTRXSTRING) SystemInterpreter::allocateResultMemory(
             sizeof(CONSTRXSTRING) * std::max((size_t)exit_parm.rxfnc_argc, (size_t)1));
        if (argrxarray == OREF_NULL)
        {
            reportException(Error_System_resources);
        }

        exit_parm.rxfnc_argv = argrxarray;

        for (size_t argindex=0; argindex < exit_parm.rxfnc_argc; argindex++)
        {
            // classic REXX style interface
            RexxString *temp = (RexxString *)arguments[argindex];
            // all arguments need to be passed as string equivalents
            if (temp != OREF_NULL)
            {
                temp = temp->requestString();
                temp->toRxstring(argrxarray[argindex]);
            }
            // explicitly clear out omitted arguments
            else
            {
                argrxarray[argindex].strlength = 0;
                argrxarray[argindex].strptr = (const char *)NULL;
            }
        }

        MAKERXSTRING(exit_parm.rxfnc_retc, retbuffer, DEFRXSTRING);

        bool wasHandled = callExit(activation, "RXFNC", RXFNC, RXFNCCAL, (void *)&exit_parm);

        // release the argument array
        SystemInterpreter::releaseResultMemory(argrxarray);

        if (!wasHandled)
        {
            return true;
        }

        // test for reported errors first
        if (exit_parm.rxfnc_flags.rxfferr)
        {
            reportException(Error_Incorrect_call_external, rname);
        }
        else if (exit_parm.rxfnc_flags.rxffnfnd)
        {
            reportException(Error_Routine_not_found_name,rname);
        }

        // if not a function call and no return value was given, raise
        // an error
        if (exit_parm.rxfnc_retc.strptr == OREF_NULL && isFunction )
        {
            reportException(Error_Function_no_data_function,rname);
        }

        if (exit_parm.rxfnc_retc.strptr)
        {
            funcresult = new_string(exit_parm.rxfnc_retc);
            if (exit_parm.rxfnc_retc.strptr != retbuffer)
            {
                SystemInterpreter::releaseResultMemory(exit_parm.rxfnc_retc.strptr);
            }
        }
        return false;
    }
    return true;
}


/**
 * Invoke the object-oriented function exit.
 *
 * @param activation The activation context.
 * @param rname      The function name.
 * @param isFunction true if this is a function call.
 * @param funcresult The returned result.
 * @param arguments  The array of arguments.
 * @param argcount   The count of arguments.
 *
 * @return The handled flag.
 */
bool Activity::callObjectFunctionExit(RexxActivation *activation, RexxString *rname,
    bool isFunction, ProtectedObject &funcresult, RexxObject **arguments, size_t argcount)
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
        RXOFNCCAL_PARM exit_parm;
        // error flags are always off
        exit_parm.rxfnc_flags.rxfferr = 0;
        exit_parm.rxfnc_flags.rxffnfnd = 0;

        exit_parm.rxfnc_flags.rxffsub = isFunction ? 0 : 1;

        rname->toRxstring(exit_parm.rxfnc_name);

        // arguments are passed directly
        exit_parm.rxfnc_argc = argcount;
        exit_parm.rxfnc_argv = (RexxObjectPtr *)arguments;
        // no result value
        exit_parm.rxfnc_retc = NULLOBJECT;

        if (!callExit(activation, "RXOFNC", RXOFNC, RXOFNCCAL, (void *)&exit_parm))
        {
            return true;
        }

        // handle the errors
        if (exit_parm.rxfnc_flags.rxfferr)
        {
            reportException(Error_Incorrect_call_external, rname);
        }
        else if (exit_parm.rxfnc_flags.rxffnfnd)
        {
            reportException(Error_Routine_not_found_name,rname);
        }
        if (exit_parm.rxfnc_retc == NULLOBJECT && isFunction)
        {
            reportException(Error_Function_no_data_function,rname);
        }

        // set the function result back
        funcresult = (RexxObject *)exit_parm.rxfnc_retc;
        return false;
    }
    return true;
}


/**
 * Call the scripting function exit.  this is the very
 * last step in external call lookups.
 *
 * @param activation The activation context.
 * @param rname      The call name.
 * @param isFunction true if this is a function call, false for
 *                   a routine.
 * @param funcresult The returned result.
 * @param arguments  The function arguments.
 * @param argcount   The argument count.
 *
 * @return The handled flag.
 */
bool Activity::callScriptingExit(RexxActivation *activation, RexxString *rname,
    bool isFunction, ProtectedObject &funcresult, RexxObject **arguments, size_t argcount)
{
    if (isExitEnabled(RXEXF))
    {
        RXEXFCAL_PARM exit_parm;

        exit_parm.rxfnc_flags.rxfferr = 0;
        exit_parm.rxfnc_flags.rxffnfnd = 0;

        exit_parm.rxfnc_flags.rxffsub = isFunction ? 0 : 1;

        rname->toRxstring(exit_parm.rxfnc_name);

        // this also passes arguments as is
        exit_parm.rxfnc_argc = argcount;
        exit_parm.rxfnc_argv = (RexxObjectPtr *)arguments;
        exit_parm.rxfnc_retc = NULLOBJECT;

        if (!callExit(activation, "RXEXF", RXEXF, RXEXFCAL, (void *)&exit_parm))
        {
            return true;
        }

        if (exit_parm.rxfnc_flags.rxfferr)
        {
            reportException(Error_Incorrect_call_external, rname);
        }
        else if (exit_parm.rxfnc_flags.rxffnfnd)
        {
            reportException(Error_Routine_not_found_name,rname);
        }
        if (exit_parm.rxfnc_retc == NULLOBJECT && isFunction)
        {
            reportException(Error_Function_no_data_function,rname);
        }

        // set the function result back
        funcresult = (RexxObject *)exit_parm.rxfnc_retc;
        return false;
    }
    return true;
}


/**
 * Call the command exit.
 *
 * @param activation The activaton context.
 * @param address    The current address environment
 * @param command    The string command.
 * @param result     The command result.
 * @param condition  any raised condition.
 *
 * @return The handled flag.
 */
bool Activity::callCommandExit(RexxActivation *activation, RexxString *address, RexxString *command, ProtectedObject &result, ProtectedObject &condition)
{
    // give the security manager the first pass
    SecurityManager *manager = activation->getEffectiveSecurityManager();
    if (manager != OREF_NULL)
    {
        if (manager->checkCommand(this, address, command, result, condition))
        {
            return false;
        }
    }

    if (isExitEnabled(RXCMD))
    {
        RXCMDHST_PARM exit_parm;
        char          retbuffer[DEFRXSTRING];

        // clear error flags
        exit_parm.rxcmd_flags.rxfcfail = 0;
        exit_parm.rxcmd_flags.rxfcerr = 0;

        exit_parm.rxcmd_addressl = (unsigned short)address->getLength();
        exit_parm.rxcmd_address = address->getStringData();

        command->toRxstring(exit_parm.rxcmd_command);

        // we have no DLL support
        exit_parm.rxcmd_dll = NULL;
        exit_parm.rxcmd_dll_len = 0;

        MAKERXSTRING(exit_parm.rxcmd_retc, retbuffer, DEFRXSTRING);

        if (!callExit(activation, "RXCMD", RXCMD, RXCMDHST, (void *)&exit_parm))
        {
            return true;
        }
        // handle the failure conditions
        if (exit_parm.rxcmd_flags.rxfcfail)
        {
            // raise the condition when things are done
            condition = createConditionObject(GlobalNames::FAILURE, (RexxObject *)result, command, OREF_NULL, OREF_NULL);
        }

        else if (exit_parm.rxcmd_flags.rxfcerr)
        {
            // raise the condition when things are done
            condition = createConditionObject(GlobalNames::ERRORNAME, (RexxObject *)result, command, OREF_NULL, OREF_NULL);
        }

        // get the return code string
        result = new_string(exit_parm.rxcmd_retc);
        if (exit_parm.rxcmd_retc.strptr != retbuffer)
        {
            SystemInterpreter::releaseResultMemory(exit_parm.rxcmd_retc.strptr);
        }
        return false;
    }
    return true;
}


/**
 * Pull a string from the queue input exit.
 *
 * @param activation The activation context.
 * @param inputstring
 *                   The returned input string.
 *
 * @return The handled flag.
 */
bool  Activity::callPullExit(RexxActivation *activation, RexxString *&inputstring)
{
    if (isExitEnabled(RXMSQ))
    {
        RXMSQPLL_PARM exit_parm;
        char          retbuffer[DEFRXSTRING];


        MAKERXSTRING(exit_parm.rxmsq_retc, retbuffer, DEFRXSTRING);

        if (!callExit(activation, "RXMSQ", RXMSQ, RXMSQPLL, (void *)&exit_parm))
        {
            return true;
        }
        // if nothing returned, return .nil to indicate this is an empty stack
        if (exit_parm.rxmsq_retc.strptr == NULL)
        {
            inputstring = (RexxString *)TheNilObject;
        }
        else
        {
            inputstring = new_string(exit_parm.rxmsq_retc);
            if (exit_parm.rxmsq_retc.strptr != retbuffer)
            {
                SystemInterpreter::releaseResultMemory(exit_parm.rxmsq_retc.strptr);
            }
        }
        return false;
    }
    return true;
}

/**
 * Handle a push or queue operation via a system exit.
 *
 * @param activation The activation context.
 * @param outputString The string to push
 * @param lifo_flag  The queuing order flag.
 *
 * @return The handled flag.
 */
bool  Activity::callPushExit(RexxActivation *activation, RexxString *outputString, QueueOrder lifo_flag)
{
    if (isExitEnabled(RXMSQ))
    {
        RXMSQPSH_PARM exit_parm;

        exit_parm.rxmsq_flags.rxfmlifo = lifo_flag == QUEUE_LIFO;

        outputString->toRxstring(exit_parm.rxmsq_value);

        return !callExit(activation, "RXMSQ", RXMSQ, RXMSQPSH, (void *)&exit_parm);
    }
    return true;
}


/**
 * Retrieve the queue size via an exit.
 *
 * @param activation The activation context
 * @param returnsize The returned size.
 *
 * @return The handled flag.
 */
bool  Activity::callQueueSizeExit(RexxActivation *activation, RexxInteger *&returnsize)
{
    if (isExitEnabled(RXMSQ))
    {
        RXMSQSIZ_PARM exit_parm;

        if (!callExit(activation, "RXMSQ", RXMSQ, RXMSQSIZ, (void *)&exit_parm))
        {
            return true;
        }
        // return the size as an integer object
        returnsize = new_integer(exit_parm.rxmsq_size);
        return false;
    }
    return true;
}


/**
 * Call the system exit indicating a queue name change.
 *
 * @param activation The activation context.
 * @param inputstring
 *                   The new queue name (can also be a return value).
 *
 * @return The handled flag.
 */
bool  Activity::callQueueNameExit(RexxActivation *activation, RexxString *&inputstring)
{
    if (isExitEnabled(RXMSQ))
    {
        RXMSQNAM_PARM exit_parm;
        char          retbuffer[DEFRXSTRING];

        // we pass the name in the result buffer
        MAKERXSTRING(exit_parm.rxmsq_name, retbuffer, inputstring->getLength());
        memcpy(exit_parm.rxmsq_name.strptr, inputstring->getStringData(), inputstring->getLength());

        if (!callExit(activation, "RXMSQ", RXMSQ, RXMSQNAM, (void *)&exit_parm))
        {
            return true;
        }

        inputstring = new_string(exit_parm.rxmsq_name);
        if (exit_parm.rxmsq_name.strptr != retbuffer)
        {
            SystemInterpreter::releaseResultMemory(exit_parm.rxmsq_name.strptr);
        }
        return false;
    }
    return true;
}


/**
 * Call the external handler to test for a halt condition.
 *
 * @param activation The activation context.
 *
 * @return The handled flag.
 */
bool Activity::callHaltTestExit(RexxActivation *activation)
{
    if (isExitEnabled(RXHLT))
    {
        RXHLTTST_PARM exit_parm;

        exit_parm.rxhlt_flags.rxfhhalt = 0;

        if (!callExit(activation, "RXHLT", RXHLT, RXHLTTST, (void *)&exit_parm))
        {
            return true;
        }

        // if halt was requested, signal that in the activation.
        if (exit_parm.rxhlt_flags.rxfhhalt == 1)
        {
            activation->halt(OREF_NULL);
        }
        return false;
    }
    return true;
}


/**
 * Call the halt exit to clear the halt condition after
 * it has been acknowledged.
 *
 * @param activation The activation context.
 *
 * @return The handled flag.
 */
bool Activity::callHaltClearExit(RexxActivation *activation)
{
    if (isExitEnabled(RXHLT))
    {
        RXHLTTST_PARM exit_parm;

        // this handler really only has an effect on the exit implementer.
        return !callExit(activation, "RXHLT", RXHLT, RXHLTCLR, (void *)&exit_parm);
    }
    return true;
}


/**
 * Call the trace exit checking for a change in trace setting.
 *
 * @param activation The activation context.
 * @param currentsetting
 *                   The current trace setting.
 *
 * @return The handled flag.
 */
bool  Activity::callTraceTestExit(RexxActivation *activation, bool currentsetting)
{
    if (isExitEnabled(RXTRC))  // is the exit enabled?
    {
        RXTRCTST_PARM exit_parm;

        exit_parm.rxtrc_flags.rxftrace = 0;

        if (!callExit(activation, "RXTRC", RXTRC, RXTRCTST, (void *)&exit_parm))
        {
            return true;
        }

        // has the trace setting changed from off to on?
        if (!currentsetting && (exit_parm.rxtrc_flags.rxftrace == 1))
        {
            activation->externalTraceOn();
            return false;
        }
        // this could also be a change from on to off
        else if (currentsetting &&  (exit_parm.rxtrc_flags.rxftrace != 1))
        {
            activation->externalTraceOff();
            return false;
        }
    }
    return true;
}


/**
 * Call the NOVALUE exit to see if the exit handler can provide
 * a new value.
 *
 * @param activation The activation context.
 * @param variableName
 *                   The name of the variable.
 * @param value      The returned value if handled.
 *
 * @return The handled flag.
 */
bool Activity::callNovalueExit(RexxActivation *activation, RexxString *variableName, RexxObject *&value)
{
    if (isExitEnabled(RXNOVAL))
    {
        RXVARNOVALUE_PARM exit_parm;

        // the name is passed as a RexxStringObject
        exit_parm.variable_name = (RexxStringObject)variableName;
        // the value is returned as an object
        exit_parm.value = NULLOBJECT;

        if (callExit(activation, "RXNOVAL", RXNOVAL, RXNOVALCALL, (void *)&exit_parm))
        {
            value = (RexxObject *)exit_parm.value;
            return false;
        }
    }
    return true;
}


/**
 * Call the VALUE() bif exit.
 *
 * @param activation The current activation context.
 * @param selector   The value variable pool selector.
 * @param variableName
 *                   The name of the variable.
 * @param newValue   A potential new value.
 * @param value      The returned old value.
 *
 * @return The handled flag.
 */
bool Activity::callValueExit(RexxActivation *activation, RexxString *selector, RexxString *variableName,
    RexxObject *newValue, ProtectedObject &value)
{
    if (isExitEnabled(RXVALUE))         // is the exit enabled?
    {
        RXVALCALL_PARM exit_parm;

        // copy the selector and variable parts
        exit_parm.selector = (RexxStringObject)selector;
        exit_parm.variable_name = (RexxStringObject)variableName;

        // the value is returned as an object, and the old value is
        // also passed that way
        exit_parm.value = (RexxObjectPtr)newValue;
        if (callExit(activation, "RXVALUE", RXVALUE, RXVALUECALL, (void *)&exit_parm))
        {
            value = (RexxObject *)exit_parm.value;
            return false;
        }
    }
    return true;
}


/**
 * Retrieve the current security manager instance.
 *
 * @return the security manager instance in effect for the
 *         activity.
 */
SecurityManager *Activity::getEffectiveSecurityManager()
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
SecurityManager *Activity::getInstanceSecurityManager()
{
    // return the manager from the instance
    return instance->getSecurityManager();
}


/**
 * Write out a line of trace output.
 *
 * @param activation The current activation context.
 * @param line       The output line
 */
void  Activity::traceOutput(RexxActivation *activation, RexxString *line)
{
    // make sure this is a real string value (likely, since we constructed it in the first place)
    line = line->stringTrace();

    // if the exit passes on the call, we write this to the .traceouput
    if (callTraceExit(activation, line))
    {
        RexxObject *stream = getLocalEnvironment(GlobalNames::TRACEOUTPUT);

        if (stream != OREF_NULL && stream != TheNilObject)
        {
            // need this in case the .traceoutput or the .error monitor
            // gives us an invalid object with no LINEOUT
            try
            {
                ProtectedObject result;
                stream->sendMessage(GlobalNames::LINEOUT, line, result);
            }
            catch (NativeActivation *)
            {
                lineOut(line); // don't lose the data!
            }
        }
        // could not find the target, but don't lose the data!
        else
        {
            lineOut(line);
        }
    }
}

/**
 * Write out a line of say output
 *
 * @param activation The current activation context.
 * @param line       The output line.
 */
void Activity::sayOutput(RexxActivation *activation, RexxString *line)
{
    if (callSayExit(activation, line))
    {
        // say output goes to .output
        RexxObject *stream = getLocalEnvironment(GlobalNames::OUTPUT);
        if (stream != OREF_NULL && stream != TheNilObject)
        {
            ProtectedObject result;
            stream->sendMessage(GlobalNames::SAY, line, result);
        }
        else
        {
            lineOut(line);
        }
    }
}


/**
 * Read a line of internactive debug.
 *
 * @param activation The activation context.
 *
 * @return The debug input string.
 */
RexxString *Activity::traceInput(RexxActivation *activation)
{
    RexxString *value;

    if (callDebugInputExit(activation, value))
    {
        // .debuginput is used for the debug read
        RexxObject *stream = getLocalEnvironment(GlobalNames::DEBUGINPUT);
        if (stream != OREF_NULL)
        {
            ProtectedObject result;
            value = (RexxString *)stream->sendMessage(GlobalNames::LINEIN, result);
            // use a null string of we get .nil back.
            if (value == (RexxString *)TheNilObject)
            {
                value = GlobalNames::NULLSTRING;
            }
        }
        else
        {
            // just use a null string if nothing is set up.
            value = GlobalNames::NULLSTRING;
        }
    }
    return value;
}


/**
 * Read a line for the pull instruction.
 *
 * @param activation The activation context.
 *
 * @return The read input string.
 */
RexxString *Activity::pullInput(RexxActivation *activation)
{
    RexxString *value;

    if (callPullExit(activation, value))
    {
        // we handle both the queue and I/O parts here
        RexxObject *stream = getLocalEnvironment(GlobalNames::STDQUE);
        // read from the rexx queue first
        if (stream != OREF_NULL)
        {
            ProtectedObject result;
            value = (RexxString *)stream->sendMessage(GlobalNames::PULL, result);
            // if we don't get anything from the queue, try again from linein
            if (value == (RexxString *)TheNilObject)
            {
                value = lineIn(activation);
            }
        }
    }
    return value;
}


/**
 * Write a line to the real default output stream.
 *
 * @param line   The line to write.
 *
 * @return Returns the residual count of 0 as an integer.
 */
RexxObject *Activity::lineOut(RexxString *line)
{
    size_t length = line->getLength();
    const char *data = line->getStringData();
    printf("%.*s" line_end,(int)length, data);
    return IntegerZero;
}


/**
 * Read a line from the input stream.
 *
 * @param activation The activation context.
 *
 * @return The input string.
 */
RexxString *Activity::lineIn(RexxActivation *activation)
{
    RexxString *value;

    if (callTerminalInputExit(activation, value))
    {
        // by default, we read from .INPUT
        RexxObject *stream = getLocalEnvironment(GlobalNames::INPUT);
        if (stream != OREF_NULL)
        {
            ProtectedObject result;
            // read using the LINEIN method
            value = (RexxString *)stream->sendMessage(GlobalNames::LINEIN, result);
            if (value == (RexxString *)TheNilObject)
            {
                value = GlobalNames::NULLSTRING;
            }
        }
        else
        {
            value = GlobalNames::NULLSTRING;
        }
    }
    return value;
}


/**
 * Write a line to the current output queue.
 *
 * @param activation The current activation context.
 * @param line       The line to push/queue
 * @param order      The queuing order.
 */
void Activity::queue(RexxActivation *activation, RexxString *line, QueueOrder order)
{
    if (callPushExit(activation, line, order))
    {
        // we use the current queue object as the target
        RexxObject *targetQueue = getLocalEnvironment(GlobalNames::STDQUE);
        if (targetQueue != OREF_NULL)
        {
            ProtectedObject result;
            if (order == QUEUE_LIFO)
            {
                targetQueue->sendMessage(GlobalNames::PUSH, line, result);
            }
            else
            {
                targetQueue->sendMessage(GlobalNames::QUEUE, line, result);
            }
        }
    }
}


/**
 * Mark this FREE activity for termination.  Set its exit flag to 1
 * and POST its run semaphore.
 */
void  Activity::terminatePoolActivity()
{
    exit = true;
    runSem.post();
}


/**
 * Run a task that needs to enter the interpreter on a thread.
 * The activity will set up the root activation and run the
 * task under that context to ensure proper error handling and
 * kernel access.
 *
 * @param target The dispatcher object that implements the call out.
 */
void Activity::run(ActivityDispatcher &target)
{
    // we unwind to the current activation depth on termination.
    size_t  startDepth;
    int32_t base;         // used for determining the stack base

    // update the stack base
    stackBase = currentThread.getStackBase(&base, TOTAL_STACK_SIZE);
    // get a new random seed
    generateRandomNumberSeed();
    startDepth = stackFrameDepth;
    // create the base marker
    createNewActivationStack();

    // save the actitivation level in case there's an error unwind for an unhandled
    // exception;
    size_t activityLevel = getActivationLevel();
    // create a new native activation
    NativeActivation *newNActa = ActivityManager::newNativeActivation(this);
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
        wholenumber_t rc = error();
        target.handleError(rc, conditionobj);
    }

    // make sure we get restored to the same base activation level.
    restoreActivationLevel(activityLevel);
    // give uninit objects a chance to run
    memoryObject.checkUninitQueue();
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
void Activity::run(CallbackDispatcher &target)
{
    // create new activation frame using the current Rexx frame (which can be null, but
    // is not likely to be).
    NativeActivation *new_activation = ActivityManager::newNativeActivation(this, currentRexxFrame);
    // this becomes the new top activation.  We also turn on the variable pool for
    // this situation.
    pushStackFrame(new_activation);
    new_activation->enableVariablepool();

    // go run this
    new_activation->run(target);
    popStackFrame(new_activation);
}


/**
 * Run a task under the context of an activity.  This will be
 * a task that runs with a nested error trapping without
 * releasing the kernel lock.
 *
 * @param target The dispatcher object that implements the call out.
 */
void Activity::run(TrappingDispatcher &target)
{
    // create new activation frame using the current Rexx frame (which can be null, but
    // is not likely to be).
    NativeActivation *new_activation = ActivityManager::newNativeActivation(this, currentRexxFrame);
    // this becomes the new top activation.
    pushStackFrame(new_activation);
    // go run this
    new_activation->run(target);
    // and pop the activation when we're done.
    popStackFrame(new_activation);
}


/**
 * Inherit all activity-specific settings from a parent activity.
 *
 * @param parent The source of the setting information.
 */
void Activity::inheritSettings(Activity *parent)
{
    clauseExitUsed = parent->clauseExitUsed;
}


/**
 * Set up a method context for use before a call out.
 *
 * @param context The method context to initialize.
 * @param owner   The native activation that owns this context.
 */
void Activity::createMethodContext(MethodContext &context, NativeActivation *owner)
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
void Activity::createCallContext(CallContext &context, NativeActivation *owner)
{
    // hook this up with the activity
    context.threadContext.threadContext = &threadContext.threadContext;
    context.threadContext.functions = &callContextFunctions;
    context.context = owner;
}


/**
 * Set up an exit  context for use before a call out.
 *
 * @param context The method context to initialize.
 * @param owner   The native activation that owns this context.
 */
void Activity::createExitContext(ExitContext &context, NativeActivation *owner)
{
    // hook this up with the activity
    context.threadContext.threadContext = &threadContext.threadContext;
    context.threadContext.functions = &exitContextFunctions;
    context.context = owner;
}


/**
 * Set up an exit context for use with a I/O redirection command
 *
 * @param context The method context to initialize.
 * @param owner   The native activation that owns this context.
 */
void Activity::createRedirectorContext(RedirectorContext &context, NativeActivation *owner)
{
    // This is handed out to the calling code rather than being
    // a straight up context
    context.redirectorContext.functions = &ioRedirectorContextFunctions;
    // The redirectory does not have an exposed thread context, so we need to
    // make a direct hookup.
    context.activity = this;
    context.context = owner;
}


/**
 * Resolve a program using the activity context information.
 *
 * @param name   The name we're interested in.
 * @param dir    A parent directory to use as part of the search.
 * @param ext    Any parent extension name.
 * @param type   The resolve type, RESOLVE_DEFAULT or RESOLVE_REQUIRES.
 *
 * @return The fully resolved file name, if it exists.  Returns OREF_NULL for
 *         non-located files.
 */
RexxString *Activity::resolveProgramName(RexxString *name, RexxString *dir, RexxString *ext, ResolveType type)
{
    return instance->resolveProgramName(name, dir, ext, type);
}


/**
 * Retrieve a value from the instance local environment.
 *
 * @param name   The name of the .local object.
 *
 * @return The object stored at the given name.
 */
RexxObject *Activity::getLocalEnvironment(RexxString *name)
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
CommandHandler *Activity::resolveCommandHandler(RexxString *name)
{
    return instance->resolveCommandHandler(name);
}


/**
 * Validate that an API call is occuring on the correct thread.
 */
void Activity::validateThread()
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
RexxString* Activity::getLastMessageName()
{
    return activationFrames->messageName();
}


/**
 * Register this mutex with the activity as being held.
 *
 * @param sem    the semaphore to add
 */
void Activity::addMutex(MutexSemaphoreClass *sem)
{
    if (heldMutexes == OREF_NULL)
    {
        heldMutexes = new_identity_table();
    }
    heldMutexes->put(sem, sem);
}


/**
 * Remove a mutex from the activity held list
 *
 * @param sem    the semaphore to remove
 */
void Activity::removeMutex(MutexSemaphoreClass *sem)
{
    // if we don't have a mutex table or this semaphore does not appear
    // in our table, then it is probably owned by a a nested activity. Pass
    // this request along to the pushed down activity
    if (heldMutexes != OREF_NULL && heldMutexes->hasIndex(sem))
    {
        heldMutexes->remove(sem);
    }
    else if (nestedActivity != OREF_NULL)
    {
        nestedActivity->removeMutex(sem);
    }
}


/**
 * Cleanup any held mutexes on activity termination.
 */
void Activity::cleanupMutexes()
{
    if (heldMutexes != OREF_NULL)
    {
        Protected<ArrayClass> semaphores = heldMutexes->allIndexes();
        for (size_t i = 1; i <= semaphores->items(); i++)
        {
            MutexSemaphoreClass *mutex = (MutexSemaphoreClass *)semaphores->get(i);
            // release the semaphore until we get a failure so the nesting is unwound.
            mutex->forceLockRelease();
        }

        // clear out the mutex list.
        heldMutexes->empty();
        heldMutexes = OREF_NULL;
    }
}

/**
 * Check for a circular requires reference and raise an error if it occurs.
 *
 * @param name   The name of the requires files
 */
void Activity::checkRequires(RexxString *name)
{
    // if this is in the list of currently loading requires files, reject the second load
    if (requiresTable->hasIndex(name))
    {
        reportException(Error_Execution_circular_requires, name);
    }
}
