/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                                         */
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
    memory_mark(activations);
    memory_mark(topStackFrame);
    memory_mark(currentRexxFrame);
    memory_mark(conditionobj);
    memory_mark(requiresTable);
    memory_mark(waitingObject);
    memory_mark(dispatchMessage);

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
    memory_mark_general(activations);
    memory_mark_general(topStackFrame);
    memory_mark_general(currentRexxFrame);
    memory_mark_general(conditionobj);
    memory_mark_general(requiresTable);
    memory_mark_general(waitingObject);
    memory_mark_general(dispatchMessage);

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
    // some things only occur on subsequent requests
    bool firstDispatch = true;
    // establish the stack base pointer for control stack full detection.
    stackBase = currentThread.getStackBase(TOTAL_STACK_SIZE);

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
            runsem.wait();
            // told to exit.  Most likely we were in the thread pool
            // and the interpreer is shutting down
            if (exit)
            {
                break;
            }

            // we need to have the kernel lock before we can really start working
            requestAccess();
            // we're already marked as active when first called to keep us from
            // getting terminated prematurely before we get a chance to run
            if (!firstDispatch)
            {
                activate();                      // make sure this is marked as active
            }
            firstDispatch = false;               // we need to activate every time after this
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
        memoryObject.runUninits();

        // cast off any items related to our initial dispatch.
        dispatchMessage = OREF_NULL;

        // no longer an active activity
        deactivate();

        // reset our semaphores
        runsem.reset();
        guardsem.reset();

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
    runsem.close();
    guardsem.close();
    currentThread.close();
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
        memoryObject.runUninits();
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
    requestAccess();
    activate();        // let the activity know it's in use, potentially nested

    // belt-and-braces.  Make sure the current activity is explicitly set to
    // this activity before leaving.
    ActivityManager::currentActivity = this;
}


/**
 * Initialize an Activity object.
 *
 * @param createThread
 *               Indicates whether we're going to be running on the
 *               current thread, or creating a new thread.
 */
Activity::Activity(bool createThread)
{
    // we need to protect this from garbage collection while constructing.
    // unfortunately, we can't use ProtectedObject because that requires an
    // active activity, which we can't guarantee at this point.
    GlobalProtectedObject p(this);

    // globally clear the object because we could be reusing the
    // object storage
    clearObject();
    // we need a stack that activaitons can use
    activations = new_internalstack(ACT_STACK_SIZE);
    // The framestack creates space for expression stacks and local variable tables
    frameStack.init();
    // an activity has a couple of semaphores it uses to synchronize execution.
    runsem.create();
    guardsem.create();
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
        runsem.reset();
        // we need to enter this thread already marked as active, otherwise
        // the main thread might shut us down before we get a chance to perform
        // whatever function we're getting asked to run.
        activate();
        // TODO:  check out whether this is an appropriate STACK size.
        currentThread.create(this, C_STACK_SIZE);
    }
    // we are creating an activity that represents the thread
    // we're currently executing on.
    else
    {
        // run on the current thread
        currentThread.useCurrentThread();
        // reset the stack base for this thread.
        // TODO:  What is the TOTAL_STACK_SIZE value?
        stackBase = currentThread.getStackBase(TOTAL_STACK_SIZE);
    }
}


/**
 * Initialize an Activity object that's being recycled for
 * another use.
 */
void Activity::reset()
{
    // most important thing to reset here is the requires table
    resetRunningRequires();
}


/**
 * Create a new activity for processing a method reply
 * instruction.
 *
 * @return The newly created activity.
 */
Activity *Activity::spawnReply()
{
    // recreate a new activiy in the same instance
    return instance->spawnActivity(this);
}


/**
 * Generate a fresh random number seed.
 */
void Activity::generateRandomNumberSeed()
{
    // TODO:  Re-examine how the initial seed is used in RexxActivation.

    // we use our own random number generator, but it's perfectly
    // to use the C library one to generate a random initial seed.
    randomSeed = 0;
    // the random number implementations vary on how large the
    // values are, but the are guaranteed to be at least 16-bit
    // quanties.  We'll compose the inital seed by shifting and xoring
    // 4 values generated by rand().
    for (size_t i = 0; i < 4; i++)
    {
        randomSeed = randomSeed << 16 ^ rand();
    }

    // scramble the seed a few times using our randomization code.
    for (int i = 0; i < 13; i++)
    {
        randomSeed = RexxActivation::RANDOMIZE(randomSeed);
    }
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
        // if we're not to the very base of the stack, terminate the frame
        topStackFrame->termination();
        popStackFrame(false);
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
        topStackFrame->termination();
        popStackFrame(false);
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

    RexxString *condition = (RexxString *)errorInfo->get(OREF_CONDITION);
    // we only display syntax conditions
    if (condition == OREF_NULL || !condition->isEqual(OREF_SYNTAX))
    {
        return 0;   // no error condition to return
    }
    // display the information
    display(errorInfo);

    // set the default return code value in case we don't have a
    // good one in the condition object.
    wholenumber_t rc = Error_Interpretation/1000;
    // try to convert.  Leaves unchanged if not value
    errorInfo->get(OREF_RC)->numberValue(rc);
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
    wholenumber_t rc = Error_Interpretation/1000;
    if (conditionObject != OREF_NULL)
    {
        // try to convert.  Leaves unchanged if not value
        conditionObject->get(OREF_RC)->numberValue(rc);
    }
    return rc;
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
bool Activity::raiseCondition(RexxString *condition, RexxObject *rc, RexxString *description, RexxObject *additional, RexxObject *result)
{
    // just create a condition object and process the traps.
    DirectoryClass *conditionObj = createConditionObject(condition, rc, description, additional, result);
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
bool Activity::raiseCondition(DirectoryClass *conditionObj)
{
    // the condition has not been handled yet.
    bool handled = false;
    RexxString *condition = (RexxString *)conditionObj->get(OREF_CONDITION);

    // unwind the stack frame calling trap until we reach the first real Rexx activation
    for (ActivationBase *activation = getTopStackFrame() ; !activation->isStackBase(); activation = activation->getPreviousStackFrame())
    {
        // see if there is an activation interested in trapping this.
        handled = activation->trap(condition, conditionObj);
        // for a normal condition, we stop checking at the first Rexx activation.
        if (isOfClass(Activation, activation))
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
DirectoryClass *Activity::createConditionObject(RexxString *condition, RexxObject *rc, RexxString *description, RexxObject *additional, RexxObject *result)
{
    // condition objects are directories
    DirectoryClass *conditionObj = new_directory();
    ProtectedObject p(conditionObj);
    // fill in the provided pieces
    conditionObj->put(condition, OREF_CONDITION);
    conditionObj->put(description == OREF_NULL ? OREF_NULLSTRING : description, OREF_DESCRIPTION);
    conditionObj->put(TheFalseObject, OREF_PROPAGATED);

    // the remainders don't have defaults, so only add the items if provided.
    if (rc != OREF_NULL)
    {
        conditionObj->put(rc, OREF_RC);
    }
    if (additional != OREF_NULL)
    {
        conditionObj->put(additional, OREF_ADDITIONAL);
    }
    if (result != OREF_NULL)
    {
        conditionObj->put(result, OREF_RESULT);
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
void Activity::reportAnException(wholenumber_t errcode)
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
void Activity::reportAnException(wholenumber_t errcode, RexxObject *substitution1 )
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
void Activity::reportAnException(wholenumber_t errcode, RexxObject *substitution1, RexxObject *substitution2 )
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
void Activity::reportAnException(wholenumber_t errcode, RexxObject *substitution1, RexxObject *substitution2, RexxObject *substitution3)
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
void Activity::reportAnException(wholenumber_t errcode, RexxObject *substitution1, RexxObject *substitution2,
    RexxObject *substitution3, RexxObject *substitution4 )
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
void Activity::reportAnException(wholenumber_t errcode, const char *substitution1, RexxObject *substitution2,
    const char *substitution3, RexxObject *substitution4)
{
    raiseException(errcode, OREF_NULL, new_array(new_string(substitution1), substitution2, new_string(substitution3), substitution4), OREF_NULL);
}
void Activity::reportAnException(wholenumber_t errcode, const char *string)
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
void Activity::reportAnException(wholenumber_t errcode, const char *string1, const char *string2)
{
    reportAnException(errcode, new_string(string1), new_string(string2));
}


void Activity::reportAnException(wholenumber_t errcode, const char *string, wholenumber_t  integer )
{

    reportAnException(errcode, new_string(string), new_integer(integer));
}


void Activity::reportAnException(wholenumber_t errcode, const char *string, wholenumber_t integer, RexxObject   *obj)
{
    reportAnException(errcode, new_string(string), new_integer(integer), obj);
}


void Activity::reportAnException(wholenumber_t errcode, const char *string, RexxObject *obj, wholenumber_t integer)
{
    reportAnException(errcode, new_string(string), obj, new_integer(integer));
}


void Activity::reportAnException(wholenumber_t errcode, RexxObject *obj, wholenumber_t integer)
{
    reportAnException(errcode, obj, new_integer(integer));
}


void Activity::reportAnException(wholenumber_t errcode, RexxObject *obj, const char *string)
{
    reportAnException(errcode, obj, new_string(string));
}


void Activity::reportAnException(wholenumber_t errcode, const char *string, RexxObject *obj)
{
    reportAnException(errcode, new_string(string), obj);
}


void Activity::reportAnException(wholenumber_t errcode, wholenumber_t  integer)
{
    reportAnException(errcode, new_integer(integer));
}

void Activity::reportAnException(wholenumber_t errcode, wholenumber_t  integer, wholenumber_t  integer2)
{
    reportAnException(errcode, new_integer(integer), new_integer(integer2));
}


void Activity::reportAnException(wholenumber_t errcode, wholenumber_t  a1, RexxObject *a2)
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
void Activity::raiseException(wholenumber_t  errcode, RexxString *description, ArrayClass *additional, RexxObject *result)
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
        conditionobj->put(TheTrueObject, OREF_PROPAGATED);
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
DirectoryClass *Activity::createExceptionObject(wholenumber_t  errcode,
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
    sprintf(work,"%ld.%1ld", errcode/1000, errcode - primary);
    RexxString *code = new_string(work);
    exobj->put(code, OREF_CODE);

    // now the primary code goes in as RC
    wholenumber_t newVal = primary/1000;
    RexxInteger *rc = new_integer(newVal);
    exobj->put(rc, OREF_RC);

    // get the text for the primary error message
    RexxString *errortext = SystemInterpreter::getMessageText(primary);
    // we can have an error for the error!
    if (errortext == OREF_NULL)
    {
        reportException(Error_Execution_error_condition, code);
    }
    exobj->put(errortext, OREF_ERRORTEXT);

    // handle the message substitution values (raw form)
    // only the secondary message has substitutions, but we
    // fill in substitutions parameters into the condition object
    // anyway.
    if (additional == OREF_NULL)
    {
        additional = new_array((size_t)0);
    }

    // add this in
    exobj->put(additional, OREF_ADDITIONAL);

    // do we have a secondary message?
    if (primary != errcode)
    {
        // build the message and add to the condition object
        RexxString *message = buildMessage(errcode, additional);
        exobj->put(message, OREF_NAME_MESSAGE);
    }
    else
    {
        // make this explicitly .nil
        exobj->put(TheNilObject, OREF_NAME_MESSAGE);
    }

    // the description string (rare for exceptions)
    if (description == OREF_NULL)
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

    // add in all location-specific information
    generateProgramInformation(exobj);

    // the condition name is always SYNTAX
    exobj->put(OREF_SYNTAX, OREF_CONDITION);
    // fill in the propagation status.  This is always false for the first
    // potential trap level, gets turned to true if this goes down levels.
    exobj->put(TheFalseObject, OREF_PROPAGATED);

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
    exobj->put(stackFrames, OREF_STACKFRAMES);
    ListClass *traceback = new_list();
    exobj->put(traceback, OREF_TRACEBACK);

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
            exobj->put(lineNumber, OREF_POSITION);
        }
    }

    // TODO:  This really should be done in the package class.
    // if we have source, and this is not part of the interpreter image,
    // add program information
    if (package != OREF_NULL && !package->isOldSpace())
    {
        exobj->put(package->getProgramName(), OREF_PROGRAM);
        exobj->put(package-, OREF_PACKAGE);
    }
    else
    {
        // if not available, then this is explicitly a NULLSTRING.
        exobj->put(OREF_NULLSTRING, OREF_PROGRAM);
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
ArrayClass *Activity::generateStackFrames(bool skipFirst)
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
        else {
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
RexxString *Activity::buildMessage(wholenumber_t messageCode, ArrayClass *substitutions)
{
    // retrieve the secondary message
    RexxString *message = SystemInterpreter::getMessageText(messageCode);
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
RexxString *Activity::messageSubstitution(RexxString *message, ArrayClass  *additional)
{
    size_t substitutions = additional->size();
    // build this up into a mutable buffer.
    Protected<MutableBuffer> newmessage = new_mutable_buffer();

    const char *messageData = message->getStringData();
    size_t searchOffset = 0;

    for (size_t i = 1; i <= substitutions; i++)
    {
        // search for the substibution value
        size_t subposition = message->pos(OREF_AND, searchOffset);
        // no more '&' markers found in the message, we're done building.
        if (subposition == 0)
        {
            break;
        }

        // append the next message section to the buffer
        newMessage->append(messageData + searchOffset, subposition - searchOffset);
        // this will be where we start searching for the next one.
        searchOffset = subposition + 2;

        // get the character following the '&'.  This should be a numeric
        // substitution number.  We only support digits 1-9.
        size_t selector = message->getChar(subposition + 1);
        RexxString *stringVal = OREF_NULLSTRING;
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
                RexxObject *value = additional->get(selector);
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
        newMessage->append(stringVal)
    }
    // append the remainder of the message to the buffer and convert to a string.
    newmessage->append(messageData + searchOffset, message->getLength() - searchOffset));
    return newmessage->makeString();
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
        exobj->put(new_integer(activation->currentLine()), OREF_POSITION);
        exobj->put(package->getProgramName(), OREF_PROGRAM);
        exobj->put(package, OREF_PACKAGE);
    }
    else
    {
        // remove the old package information.
        exobj->remove(OREF_POSITION);
        exobj->remove(OREF_PROGRAM);
        exobj->remove(OREF_PACKAGE);
    }

    // get the error code and redo the message information
    RexxInternalObject *errorcode = exobj->get(OREF_CODE);
    wholenumber_t errornumber = Interpreter::messageNumber((RexxString *)errorcode);

    wholenumber_t primary = (errornumber / 1000) * 1000;
    if (errornumber != primary)
    {
        char work[10];
        sprintf(work,"%1ld%3.3ld", errornumber/1000, errornumber - primary);
        errornumber = atol(work);

        RexxString *message = SystemInterpreter::getMessageText(errornumber);
        ArrayClass *additional = (ArrayClass *)exobj->get(OREF_ADDITIONAL);
        message = messageSubstitution(message, additional);
        // replace the original message text
        exobj->put(message, OREF_NAME_MESSAGE);
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
    RexxString *condition = (RexxString *)conditionObj->get(OREF_CONDITION);
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
    ListClass *trace_backList = (ListClass *)exobj->get(OREF_TRACEBACK);
    if (trace_backList != OREF_NULL)
    {
        ArrayClass *trace_back = trace_backList->makeArray();
        ProtectedObject p(trace_back);

        // display each of the traceback lines
        size_t tracebackSize = trace_back->size();

        for (size_t i = 1; i <= tracebackSize; i++)
        {
            RexxString *text = (RexxString *)trace_back->get(i);
            // if we have a real like, write it out
            if (text != OREF_NULL && text != TheNilObject)
            {
                traceOutput(currentRexxFrame, text);
            }
        }
    }

    // get the error code, and format a message header
    RexxObject *rc = exobj->get(OREF_RC);
    wholenumber_t errorCode = Interpreter::messageNumber((RexxString *)rc);

    RexxString *text = SystemInterpreter::getMessageHeader(errorCode);

    // compose the longer message
    if (text == OREF_NULL)
    {
        text = SystemInterpreter::getMessageText(Message_Translations_error);
    }
    else
    {
        text = text->concat(SystemInterpreter::getMessageText(Message_Translations_error));
    }

    // get the program name
    RexxString *programname = (RexxString *)exobj->get(OREF_PROGRAM);
    // add on the error number
    text = text->concatWith(rc->requestString(), ' ');

    // add on the program name if we have one.
    if (programname != OREF_NULL && programname != OREF_NULLSTRING)
    {
        text = text->concatWith(SystemInterpreter::getMessageText(Message_Translations_running), ' ');
        text = text->concatWith(programname, ' ');

        // if we have a line position, add that on also
        RexxObject *position = exobj->get(OREF_POSITION);
        if (position != OREF_NULL)
        {
            text = text->concatWith(SystemInterpreter::getMessageText(Message_Translations_line), ' ');
            text = text->concatWith(position->requestString(), ' ');
        }
    }
    text = text->concatWithCstring(":  ");

    // and finally the primary error message
    text = text->concat((RexxString *)exobj->get(OREF_ERRORTEXT));
    traceOutput(currentRexxFrame, text);

    // now add the secondary message if we have one.
    RexxString *secondary = (RexxString *)exobj->get(OREF_NAME_MESSAGE);
    if (secondary != OREF_NULL && secondary != (RexxString *)TheNilObject)
    {
        rc = exobj->get(OREF_CODE);
        errorCode = Interpreter::messageNumber((RexxString *)rc);
        text = SystemInterpreter::getMessageHeader(errorCode);
        if (text == OREF_NULL)
        {
            text = SystemInterpreter::getMessageText(Message_Translations_error);
        }
        else
        {
            text = text->concat(SystemInterpreter::getMessageText(Message_Translations_error));
        }

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
Activity::displayDebug(DirectoryClass *exobj)
{
    // get the leading part to indicate this is a debug error, then compose the full
    // message
    RexxString *text = SystemInterpreter::getMessageText(Message_Translations_debug_error);
    text = text->concatWith((exobj->get(OREF_RC))->requestString(), ' ');
    text = text->concatWithCstring(":  ");
    text = text->concatWith((RexxString *)exobj->get(OREF_ERRORTEXT), ' ');
    traceOutput(currentRexxFrame, text);


    // now any secondary message
    secondary = (RexxString *)exobj->get(OREF_NAME_MESSAGE);
    if (secondary != OREF_NULL && secondary != (RexxString *)TheNilObject) {
        text = SystemInterpreter::getMessageText(Message_Translations_debug_error);
        text = text->concatWith((RexxString *)exobj->get(OREF_CODE), ' ');
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
    guardsem.post();
    runsem.post();
    SysActivity::yield();
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
             newstack->push(activations->peek(i-1));
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
    currentRexxFrame = topStackFrame->findRexxContext(); ;

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
void Activity::popStackFrame(bool  reply)
{
    // pop off the top elements and reduce the depth
    ActivationBase *poppedStackFrame = (ActivationBase *)activations->fastPop();
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
    ActivationBase *poppedStackFrame = (ActivationBase *)activations->fastPop();
    stackFrameDepth--;
    // pop off the top elements and reduce the depth
    while (poppedStackFrame != target)
    {
        // clean this up and potentially cache
        cleanupStackFrame(poppedStackFrame);
        poppedStackFrame = (ActivationBase *)activations->fastPop();
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
        ActivationBase *poppedActivation = (ActivationBase *)activations->fastPop();
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
void Activity::unwindToFrame(RexxActivation *frame)
{
    ActivationBase *activation;

    // keep popping frames until we find the tarte frame
    while ((activation = getTopStackFrame()) != frame)
    {
        activation->termination();
        popStackFrame(false);
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
void Activity::initializeThreadContext()
{
    threadContextFunctions.RexxNil = (RexxObjectPtr)TheNilObject;
    threadContextFunctions.RexxTrue = (RexxObjectPtr)TheTrueObject;
    threadContextFunctions.RexxFalse = (RexxObjectPtr)TheFalseObject;
    threadContextFunctions.RexxNullString = (RexxStringObject)OREF_NULLSTRING;
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
    attached = false;
    // if there's a nesting situation, restore the activity to active state.
    if (nestedActivity != OREF_NULL)
    {
        nestedActivity->setSuspended(false);
    }
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
    // clear the run semaphore and save the object we're waiting on
    runsem.reset();
    waitingObject = resource;
    // release the interpreter lock and wait for access.  Don't continue
    // until we get the lock back
    releaseAccess();
    runsem.wait();
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
    guardsem.wait();
    requestAccess();
}


/**
 * Post a guard expression wake up notice
 */
void Activity::guardPost()
{
    guardsem.post();
}


/**
 * Clear a guard expression semaphore in preparation to perform a
 * guard wait
 */
void Activity::guardSet()
{
    guardsem.reset();
}


/**
 * Post an activities run semaphore
 */
void Activity::postDispatch()
{
    waitingObject = OREF_NULL;
    runsem.post();
}

void Activity::kill(
    DirectoryClass *conditionObj)       /* associated "kill" object          */
/******************************************************************************/
/* Function:  Kill a running activity,                                        */
/******************************************************************************/
{
  conditionobj = conditionObj;   /* save the condition object         */
  throw UnhandledCondition;            // we have a fatal error condition
}

void Activity::relinquish()
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
void Activity::yield()
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
bool Activity::halt(RexxString *d)
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
bool Activity::setTrace(bool on)
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

void Activity::releaseAccess()
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

void Activity::requestAccess()
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

void Activity::checkStackSpace()
/******************************************************************************/
/* Function:  Make sure there is enough stack space to run a method           */
/******************************************************************************/
{
#ifdef STACKCHECK
  size_t temp;                          // if checking and there isn't room
  if (((char *)&temp - (char *)stackBase) < MIN_C_STACK && stackcheck == true)
  {
                                        // go raise an exception
      reportException(Error_Control_stack_full);
  }
#endif
}


DirectoryClass *Activity::getLocal()
/******************************************************************************/
/* Function:  Retrive the activities local environment                        */
/******************************************************************************/
{
  return instance->getLocal();              // just return the .local directory
}

thread_id_t  Activity::threadIdMethod()
/******************************************************************************/
/* Function:  Retrieve the activities threadid                                */
/******************************************************************************/
{
    return currentThread.getThreadID();  /* just return the thread id info    */
}

void Activity::queryTrcHlt()
/******************************************************************************/
/* Function:  Determine if Halt or Trace System exits are set                 */
/*            and set a flag to indicate this.                                */
/******************************************************************************/
{                                      /* is HALT sys exit set              */
    if (isExitEnabled(RXHLT))
    {
        clauseExitUsed = true;       /* set flag to indicate one is found */
        return;                            /* and return                        */
    }
    /* is TRACE sys exit set             */
    if (isExitEnabled(RXTRC))
    {
        clauseExitUsed = true;   /* set flag to indicate one is found */
        return;                            /* and return                        */
    }

    clauseExitUsed = false;    /* remember that none are set        */
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


void Activity::callInitializationExit(
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

void Activity::callTerminationExit(
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

bool  Activity::callSayExit(
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

bool Activity::callTraceExit(
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

bool Activity::callTerminalInputExit(
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

bool Activity::callDebugInputExit(
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

bool Activity::callFunctionExit(
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
                temp = temp->requestString();
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


bool Activity::callObjectFunctionExit(
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


bool Activity::callScriptingExit(
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
            condition = createConditionObject(OREF_FAILURENAME, (RexxObject *)result, command, OREF_NULL, OREF_NULL);
        }

        /* Did we find the function??        */
        else if (exit_parm.rxcmd_flags.rxfcerr)
        {
            // raise the condition when things are done
            condition = createConditionObject(OREF_ERRORNAME, (RexxObject *)result, command, OREF_NULL, OREF_NULL);
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


bool  Activity::callPullExit(
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

bool  Activity::callPushExit(
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


bool  Activity::callQueueSizeExit(
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


bool  Activity::callQueueNameExit(
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


bool Activity::callHaltTestExit(
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


bool Activity::callHaltClearExit(
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


bool  Activity::callTraceTestExit(
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


bool Activity::callNovalueExit(
    RexxActivation *activation,        /* sending activation                */
    RexxString     *variableName,      /* name to look up                   */
    RexxInternalObject *&value)        /* the returned value                */
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


bool Activity::callValueExit(
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



void  Activity::traceOutput(       /* write a line of trace information */
      RexxActivation *activation,      /* sending activation                */
      RexxString *line)                /* line to write out                 */
/******************************************************************************/
/* Function:  Write a line of trace output to the .ERROR stream               */
/******************************************************************************/
{
    line = line->stringTrace();          /* get traceable form of this        */
                                         /* if exit declines the call         */
    if (callTraceExit(activation, line))
    {
        /* get the default output stream     */
        RexxObject *stream = getLocalEnvironment(OREF_TRACEOUTPUT);
        /* have .local set up yet?           */
        if (stream != OREF_NULL && stream != TheNilObject)
        {
            stream->sendMessage(OREF_LINEOUT, line);
        }
        /* do the lineout                    */
        else                               /* use the "real" default stream     */
        {
            lineOut(line);
        }
    }
}

void Activity::sayOutput(          /* write a line of say information   */
     RexxActivation *activation,       /* sending activation                */
     RexxString *line)                 /* line to write out                 */
/******************************************************************************/
/* Function:  Write a line of SAY output to the .OUTPUT stream                */
/******************************************************************************/
{
    if (callSayExit(activation, line))
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
            lineOut(line);
        }
    }
}

RexxString *Activity::traceInput(  /* read a line of trace input        */
     RexxActivation *activation)       /* sending activation                */
/******************************************************************************/
/* Function:  Read a line for interactive debug input                         */
/******************************************************************************/
{
    RexxString *value;

    /* if exit declines the call         */
    if (callDebugInputExit(activation, value))
    {
        /* get the input stream              */
        RexxObject *stream = getLocalEnvironment(OREF_DEBUGINPUT);
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


RexxString *Activity::pullInput(   /* read a line of pull input         */
     RexxActivation *activation)       /* sending activation                */
/******************************************************************************/
/* Function:  Read a line for the PULL instruction                            */
/******************************************************************************/
{
    RexxString *value;

    /* if exit declines call             */
    if (callPullExit(activation, value))
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
                value = lineIn(activation);
            }
        }
    }
    return value;                        /* return the read value             */
}

RexxObject *Activity::lineOut(
    RexxString *line)                  /* line to write out                 */
/******************************************************************************/
/* Function:  Write a line out to the real default I/O stream                 */
/******************************************************************************/
{
  size_t  length;                      /* length to write out               */
  const char *data;                    /* data pointer                      */

  length = line->getLength();          /* get the string length and the     */
  data = line->getStringData();        /* data pointer                      */
  printf("%.*s\n",(int)length, data);       /* print it                          */
  return (RexxObject *)IntegerZero;    /* return on residual count          */
}

RexxString *Activity::lineIn(
    RexxActivation *activation)        /* sending activation                */
/******************************************************************************/
/* Function:  Read a line for the PARSE LINEIN instruction                    */
/******************************************************************************/
{
    RexxString *value;

    /* if exit declines call             */
    if (callTerminalInputExit(activation, value))
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


void Activity::queue(              /* write a line to the queue         */
     RexxActivation *activation,       /* sending activation                */
     RexxString *line,                 /* line to write                     */
     int order)                        /* LIFO or FIFO order                */
/******************************************************************************/
/* Function:  Write a line to the external data queue                         */
/******************************************************************************/
{
    /* if exit declines call             */
    if (callPushExit(activation, line, order))
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

void  Activity::terminatePoolActivity()
/******************************************************************************/
/* Function: Mark this FREE activity for termination.  Set its exit flag to 1 */
/*   and POST its run semaphore.                                              */
/******************************************************************************/
{
    exit = true;                   /* Activity should exit          */
    runsem.post();                /* let him run so he knows to exi*/
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
    size_t  startDepth;                  /* starting depth of activation stack*/

                                         /* make sure we have the stack base  */
    stackBase = currentThread.getStackBase(TOTAL_STACK_SIZE);
    generateRandomNumberSeed();    /* get a fresh random seed           */
    startDepth = stackFrameDepth;        /* Remember activation stack depth   */
                                         /* Push marker onto stack so we know */
    createNewActivationStack();    /* what level we entered.            */

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
        wholenumber_t rc = error();                /* do error cleanup                  */
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
    popStackFrame(new_activation); /* pop the top activation            */
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

void Activity::createExitContext(ExitContext &context, NativeActivation *owner)

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
RexxString *Activity::resolveProgramName(RexxString *name, RexxString *dir, RexxString *ext)
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
RexxString *Activity::getLastMessageName()
{
    return activationFrames->messageName();
}
