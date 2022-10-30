/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2021 Rexx Language Association. All rights reserved.    */
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
/* Primitive Activation Class                                                 */
/*                                                                            */
/* NOTE:  activations are an execution time only object.  They are never      */
/*        flattened or saved in the image, and hence will never be in old     */
/*        space.  Because of this, activations "cheat" and do not use         */
/*        OrefSet to assign values to get better performance.  Care must be   */
/*        used to maintain this situation.                                    */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "BufferClass.hpp"
#include "DirectoryClass.hpp"
#include "VariableDictionary.hpp"
#include "RexxActivation.hpp"
#include "Activity.hpp"
#include "MethodClass.hpp"
#include "MessageClass.hpp"
#include "RexxCode.hpp"
#include "RexxInstruction.hpp"
#include "CallInstruction.hpp"
#include "DoBlock.hpp"
#include "DoInstruction.hpp"
#include "ProtectedObject.hpp"
#include "ActivityManager.hpp"
#include "Interpreter.hpp"
#include "SystemInterpreter.hpp"
#include "Interpreter.hpp"
#include "RexxInternalApis.h"
#include "PackageManager.hpp"
#include "CompoundVariableTail.hpp"
#include "CommandHandler.hpp"
#include "ActivationFrame.hpp"
#include "StackFrameClass.hpp"
#include "InterpreterInstance.hpp"
#include "PackageClass.hpp"
#include "RoutineClass.hpp"
#include "LanguageParser.hpp"
#include "TrapHandler.hpp"
#include "MethodArguments.hpp"
#include "RequiresDirective.hpp"
#include "CommandIOConfiguration.hpp"
#include "CommandIOContext.hpp"
#include "LibraryPackage.hpp"

/**
 * Create a new activation object
 *
 * @param size   Base size of the object.
 *
 * @return Storage for building an activation object.
 */
void * RexxActivation::operator new(size_t size)
{
    return new_object(size, T_Activation);
}


/**
 * Initialize an activation for direct caching in the activation
 * cache.  At this time, this is not an executable activation
 */
RexxActivation::RexxActivation()
{
    setHasNoReferences();          // nothing referenced from this either
}


/**
 * Initialize an activation for a method invocation.
 *
 * @param _activity The activity we're running under.
 * @param _method   The method being invoked.
 * @param _code     The code to execute.
 */
RexxActivation::RexxActivation(Activity* _activity, MethodClass * _method, RexxCode *_code)
{
    clearObject();                 // start with a fresh object
    activity = _activity;          // save the activity pointer
    scope = _method->getScope();   // save the scope
    code = _code;                  // the code we're executing
    executable = _method;          // save this as the base executable
                                   // save the source object reference also
    packageObject = _method->getPackageObject();
    settings.intermediateTrace = false;
    activationContext = METHODCALL;  // the context is a method call
    parent = OREF_NULL;              // we don't have a parent stack frame when invoked as a method
    executionState = ACTIVE;         // we are now in active execution
    objectScope = SCOPE_RELEASED;    // scope not reserved yet

    // create a new evaluation stack.  This must be done before a
    // local variable frame is created.

    // get our evaluation stack
    allocateStackFrame();

    // initialize from the package-defined settings
    inheritPackageSettings();

    if (_method->isGuarded())            // make sure we set the appropriate guarded state
    {
        setGuarded();
    }

    settings.parentCode = code;

    // allocate a frame for the local variables from activity stack
    allocateLocalVariables();

    // set the initial and initial alternate address settings
    settings.currentAddress = activity->getInstance()->getDefaultEnvironment();
    settings.alternateAddress = settings.currentAddress;
    // get initial random seed value
    randomSeed = activity->getRandomSeed();
    // copy the security manager, and pick up the instance one if nothing is set.
    settings.securityManager = code->getSecurityManager();
    if (settings.securityManager == OREF_NULL)
    {
        settings.securityManager = activity->getInstanceSecurityManager();
    }
    // and the call type is METHOD
    settings.calltype = GlobalNames::METHOD;
}


/**
 * Create a new Rexx activation for an internal level call.
 * An internal level call is an internal call, a call trap,
 * an Interpret statement, or a debug pause execution.
 *
 * @param _activity The current activity.
 * @param _parent   The parent activation.
 * @param _code     The code to be executed.  For interpret and debug pauses, this
 *                  is a new code object.  For call activations, this is the
 *                  parent code object.
 * @param context   The type of call being made.
 */
RexxActivation::RexxActivation(Activity *_activity, RexxActivation *_parent, RexxCode *_code, ActivationContext context)
{
    clearObject();                 // start with a fresh object
    activity = _activity;          // save the activity pointer
    code = _code;                  // get the code we're going to execute

    // if this is a debug pause, then flip on the debug pause flag
    // so we know we're executing the pause, but execute this as an
    // INTERPRET call.
    if (context == DEBUGPAUSE)
    {
        debugPause = true;
        context = INTERPRET;
    }

    activationContext = context;
    settings.intermediateTrace = false;

    // the sender is our parent activity
    parent = _parent;
    executionState = ACTIVE;      // we are now in active execution
    objectScope = SCOPE_RELEASED; // internal calls don't have a scope

    // get our evaluation stack
    allocateStackFrame();

    // inherit parents settings
    _parent->putSettings(settings);
    // step the trace indentation level for this internal nesting
    settings.traceIndent++;

    // if we are doing an internal call, we've inherited our
    // caller's trap state, but if we change anything, then we need
    // to create a new set of tables so that we don't change the caller's
    // settings as well.
    if (context == INTERNALCALL)
    {
        settings.setTrapsCopied(false);
        settings.setReplyIssued(false);
        // invalidate the timestamp...interpret or debug pauses use the old timestamp.
        settings.timeStamp.valid = false;
    }

    // this is a nested call until we issue a procedure *
    settings.localVariables.setNested();
    // get the executable from the parent.
    executable = _parent->getExecutable();
    // for internal calls, this is the same source object as the parent
    if (isInterpret())
    {
        // use the source object for the interpret so error tracebacks are correct.
        packageObject = code->getPackageObject();
    }
    else
    {
        // save the source object reference also
        packageObject = executable->getPackageObject();
    }
}


/**
 * Create a top-level activation of Rexx code.  This will
 * either a toplevel program or an external call.
 *
 * @param _activity The current thread we're running on.
 * @param _routine  The routine to invoke.
 * @param _code     The code object to be executed.
 * @param calltype  Type type of call being made (function or subroutine)
 * @param env       The default address environment
 * @param context   The type of call context.
 */
RexxActivation::RexxActivation(Activity *_activity, RoutineClass *_routine, RexxCode *_code,
    RexxString *calltype, RexxString *env, ActivationContext context)
{
    clearObject();
    activity = _activity;
    code = _code;                  // the code comes from the routine object...
    executable = _routine;         // and the routine is our executable
                                   // save the source object reference also
    packageObject = _routine->getPackageObject();

    activationContext = context;
    settings.intermediateTrace = false;
    parent = OREF_NULL;            // there's no parent for a top level call
    executionState = ACTIVE;       // we are now in active execution
    objectScope = SCOPE_RELEASED;  // scope not reserved yet

    // get our evaluation stack
    allocateStackFrame();

    // initialize with the package-defined settings
    inheritPackageSettings();

    // save the source also
    settings.parentCode = code;

    // allocate a frame for the local variables from activity stack
    allocateLocalVariables();

    // we use default address settings
    settings.currentAddress = activity->getInstance()->getDefaultEnvironment();
    settings.alternateAddress = settings.currentAddress;

    // this is a top level call, so we get a fresh random seed
    randomSeed = activity->getRandomSeed();

    // copy the source security manager
    settings.securityManager = code->getSecurityManager();
    // but use the default if not set
    if (settings.securityManager == OREF_NULL)
    {
        settings.securityManager = activity->getInstanceSecurityManager();
    }

    // if we have a default environment specified, apply the override.
    if (env != OREF_NULL)
    {
        setDefaultAddress(env);
    }
    // set the call type
    if (calltype != OREF_NULL)
    {
        settings.calltype = calltype;
    }
}


/**
 * Generalized object marking.
 *
 * @param reason The reason for this live marking operation.
 */
void RexxActivation::live(size_t liveMark)
{
    memory_mark(previous);
    memory_mark(executable);
    memory_mark(code);
    memory_mark(packageObject);
    memory_mark(scope);
    memory_mark(receiver);
    memory_mark(activity);
    memory_mark(parent);
    memory_mark(doStack);
    // settings and stack handle their own marking.
    settings.live(liveMark);
    stack.live(liveMark);
    memory_mark(current);
    memory_mark(next);
    memory_mark(result);
    memory_mark(trapInfo);
    memory_mark(notifyObject);
    memory_mark(environmentList);
    memory_mark(conditionQueue);
    memory_mark(contextObject);

    // We're hold a pointer back to our arguments directly where they
    // are created.  Since in some places, this argument list comes
    // from the C stack, we need to handle the marking ourselves.
    memory_mark_array(argCount, argList);
}


/**
 * Generalized object marking.
 *
 * @param reason The reason for this live marking operation.
 */
void RexxActivation::liveGeneral(MarkReason reason)
{
    memory_mark_general(previous);
    memory_mark_general(executable);
    memory_mark_general(code);
    memory_mark_general(packageObject);
    memory_mark_general(scope);
    memory_mark_general(receiver);
    memory_mark_general(activity);
    memory_mark_general(parent);
    memory_mark_general(doStack);
    // settings and stack handle their own marking.
    settings.liveGeneral(reason);
    stack.liveGeneral(reason);
    memory_mark_general(current);
    memory_mark_general(next);
    memory_mark_general(result);
    memory_mark_general(trapInfo);
    memory_mark_general(notifyObject);
    memory_mark_general(environmentList);
    memory_mark_general(conditionQueue);
    memory_mark_general(contextObject);

    // We're hold a pointer back to our arguments directly where they
    // are created.  Since in some places, this argument list comes
    // from the C stack, we need to handle the marking ourselves.
    memory_mark_general_array(argCount, argList);
}


/**
 * Allocate a stack frame for this activation to use
 * for the evaluation stack.
 */
void RexxActivation::allocateStackFrame()
{
    // a live marking can happen without a properly set up stack (::live()
    // is called). Setting the NoRefBit when creating the stack avoids it.
    setHasNoReferences();
    activity->allocateStackFrame(&stack, code->getMaxStackSize());
    setHasReferences();
}


/**
 * Allocate a new local variable frame.
 */
void RexxActivation::allocateLocalVariables()
{
    // allocate a frame for the local variables from activity stack
    settings.localVariables.init(this, code->getLocalVariableSize());
    activity->allocateLocalVariableFrame(&settings.localVariables);
}


/**
 * Inherit the settings from our package object.
 */
void RexxActivation::inheritPackageSettings()
{
    // just copy the whole initial settings piece.
    settings.packageSettings = packageObject->getSettings();
    // if tracing intermediates, turn on the special fast check flag
    settings.intermediateTrace = settings.packageSettings.traceSettings.tracingIntermediates();
}


/**
 * Re-dispatch an activation after a REPLY
 *
 * @return The result object.
 */
RexxObject * RexxActivation::dispatch()
{
    ProtectedObject r;
    // we just resume running at the old location, reusing the intial values.
    return run(receiver, settings.messageName, argList, argCount, OREF_NULL, r);
}


/**
 * Run some Rexx code...this is it!  This is the heart of the
 * interpreter that makes the whole thing run!
 *
 * @param _receiver The target receiver object (if a method call)
 * @param name      The message, routine, or program name.
 * @param _arglist  The argument list.
 * @param _argcount The count of arguments.
 * @param start     The starting instruction.
 * @param resultObj A protected object for returning a result.
 *
 * @return The execution result (also returned via the protected object.)
 */
RexxObject* RexxActivation::run(RexxObject *_receiver, RexxString *name, RexxObject **_arglist,
                                size_t _argcount, RexxInstruction *start, ProtectedObject &resultObj)
{
    // add the frame to the execution stack
    RexxActivationFrame frame(activity, this);

    receiver = _receiver;
    // the "msgname" can also be the name of an external routine, the label
    // name of an internal routine.
    settings.messageName = name;

    // not a reply restart situation?  We need to do the full
    // initial setup
    if (executionState != REPLIED)
    {
        // exits possible?  We don't use exits for methods in the image
        // we try not to check stuff between clauses unless we have to...the
        // clause boundary flag tells us we need to perform checks.
        if (!code->isOldSpace() && activity->isClauseExitUsed())
        {
            // check at the end of each clause
            clauseBoundary = true;
            // remember that we have sys exits
            settings.setHaveClauseExits(true);
        }
        // save the argument information
        argList = _arglist;
        argCount = _argcount;
        // is this the top-level program call?  We need to dummy up our
        // parent information.
        if (isTopLevelCall())
        {
            // save entry argument list forvariable pool fetch private access
            settings.parentArgList = argList;
            settings.parentArgCount = argCount;
            // make sure the code has resolved any class definitions, requireds, or libraries
            code->install(this);
            // set our starting code position (and the instruction used for error reporting)
            next = code->getFirstInstruction();
            current = next;
            // if this is a program invocation, then we need to potentially
            // run the initialization exit.
            if (isProgramLevelCall())
            {
                activity->callInitializationExit(this);
                activity->getInstance()->setupProgram(this);
            }
            // this is a method call.  We need to do some method setup.
            else
            {
                // guarded methods need to reserve the object scope
                if (isGuarded())
                {
                    // get the object variables and reserve these
                    settings.objectVariables = receiver->getObjectVariables(scope);
                    settings.objectVariables->reserve(activity);
                    objectScope = SCOPE_RESERVED;
                }
                // initialize the SELF and SUPER variables
                setLocalVariable(GlobalNames::SELF, VARIABLE_SELF, receiver);
                setLocalVariable(GlobalNames::SUPER, VARIABLE_SUPER, receiver->superScope(scope));
            }
        }
        // Internal call, interpret, or a debug pause
        else
        {
            // for an internal call, we're handed the starting instruction.  This
            // is probably an interpret, so we need to get it from the code object.
            if (start != OREF_NULL)
            {
                next = start;
            }
            else
            {
                next = code->getFirstInstruction();
            }
            // current and next are always the same at the start in case
            // we encounter any errors.
            current = next;
        }
    }
    // resuming on a new thread after a reply
    else
    {
        // if we could not keep the guard lock when we were spun off, then
        // we need to reaquire (and potentially wait) for the lock now.
        if (settings.hasTransferFailed())
        {
            settings.objectVariables->reserve(activity);
            // turn off the failure flag in case we spin off again.
            settings.setTransferFailed(false);
        }
    }

    // if this is an internal call, then we need to scan a little
    // bit ahead to see if we have a procedure at the start of the
    // code section.
    if (isInternalCall())
    {
        start = next;
        while (start != OREF_NULL && start->isType(KEYWORD_LABEL))
        {
            start = start->nextInstruction;
        }
        // if we only have labels and then a PROCEDURE, this is valid
        // and we allow it when issued.
        if (start != OREF_NULL && start->isType(KEYWORD_PROCEDURE))
        {
            settings.setProcedureValid(true);
        }
    }

    // we are now live
    executionState = ACTIVE;

    // we might have a package option that turned on tracing.  If this
    // is a routine or method invocation in one of those packages, give the
    // initial entry trace so the user knows where we are.
    // Must be one of ::OPTIONS TRACE ALL/RESULTS/INTERMEDIATES/LABELS
    if (tracingLabels() && isMethodOrRoutine())
    {
        traceEntry();
        if (!tracingAll())
        {
            // we pause on the label only for ::OPTIONS TRACE LABELS
            pauseLabel();
        }
    }

    // this is the main execution loop...continue until we get a terminating
    // condition, such as a RETURN or EXIT, or just reaching the end of the code stream.
    while (true)
    {
        try
        {
            RexxInstruction *nextInst = next;
            // loop until we no longer have a next instruction to process
            while (nextInst != OREF_NULL)
            {
                // concurrency is cooperative, so we release access every so often
                // to allow other threads to run.
                if (++instructionCount > yieldInstructions)
                {
                    instructionCount = 0;   // reset the instruction counter even if we didn't yield.
                    // and have the activity manager decide if we need to give up control
                    ActivityManager::relinquishIfNeeded(activity);
                }
                // set the current instruction and prefetch the next one.  Control
                // instructions may change next on us.
                current = nextInst;
                next = nextInst->nextInstruction;
                // execute the current instruction
                nextInst->execute(this, &stack);

                // make sure the stack is cleared after each instruction
                stack.clear();
                // the time stamp is no longer current
                settings.timeStamp.valid = false;

                // do we need to check clause_boundary stuff?  Go do those
                // checks.
                if (clauseBoundary)
                {
                    processClauseBoundary();
                }
                // get our next instrucion and loop around
                nextInst = next;
            }

            // if we've fallen off the end of the code, our state is still
            // active.  Handle this as an implicit exit
            if (executionState == ACTIVE)
            {
                implicitExit();              /* treat this like an EXIT           */
            }

            // had a real RETURN?
            if (executionState == RETURNED)
            {
                // perform the normal termination
                termination();
                // if this was an interpret, then any state changes need to
                // be pushed back to the parent
                if (isInterpret())
                {
                    // save the nested setting
                    bool nested = parent->settings.localVariables.isNested();
                    // propagate parent's settings back
                    // but keep the parent's message name as is
                    settings.messageName = parent->settings.messageName;
                    parent->getSettings(settings);
                    if (!nested)
                    {
                        // if our calling variable context was not nested, we
                        // need to clear it.
                        parent->settings.localVariables.clearNested();
                    }
                    // merge any pending conditions
                    parent->mergeTraps(conditionQueue);
                }

                resultObj = result;  // save the result
                // pop this off of the activity stack and
                activity->popStackFrame(false);
                // this stack frame is no longer active. If there is an error
                // running an uninit method, we want to ensure that this stack
                // frame doesn't get processed when generating the traceback.
                frame.disableFrame();
                // see if there are any objects waiting to run uninits.
                memoryObject.checkUninitQueue();
            }
            // This is a REPLIED state
            else
            {
                // save the reply result for handing back to the caller.
                resultObj = result;
                // reset the next instruction
                next = current->nextInstruction;

                // now we need to create a new activity and
                // attach this activation to it.
                Activity *oldActivity = activity;

                activity = oldActivity->spawnReply();

                // save the pointer to the start of our stack frame.  We're
                // going to need to release this after we migrate everything
                // over.
                RexxInternalObject **framePtr = stack.getFrame();
                // migrate the local variables and the expression stack to the
                // new activity.  NOTE:  these must be done in this order to
                // get them allocated from the new activity in the correct
                // order.
                stack.migrate(activity);
                settings.localVariables.migrate(activity);
                // if we have arguments, we need to migrate those also, as they are
                // subject to overwriting once we return to the parent activation.
                if (argCount > 0)
                {
                    RexxObject **newArguments = (RexxObject **)activity->allocateFrame(argCount);
                    memcpy(newArguments, argList, sizeof(RexxObject *) * argCount);
                    argList = newArguments;  // must be set on "this"
                    settings.parentArgList = newArguments;
                }

                // return our stack frame space back to the old activity.
                oldActivity->releaseStackFrame(framePtr);

                // now push this activation on to the new activity
                activity->pushStackFrame(this);
                // pop the old one off of the stack frame (but without returning it to
                // the activation cache)
                oldActivity->popStackFrame(true);
                // if we have the scope lock, try to transfer it to the new activity.  If
                // the lock is nested on this activity, then we will need to
                // obtain it when we start running on the new thread.
                if (objectScope == SCOPE_RESERVED)
                {
                    if (!settings.objectVariables->transfer(activity))
                    {
                        // this will tell us that we need to try grabbing this again.
                        settings.setTransferFailed(true);
                    }
                }
                // now start the new activity running and give up control on this
                // thread before returning.
                activity->run();
                oldActivity->relinquish();
            }
            return resultObj;
        }
        // an error has occurred.  The thrown object is an activation pointer,
        // which tells us when to stop unwinding
        catch (RexxActivation *t)
        {
            // if we're not the target of this throw, we've already been unwound
            // keep throwing this until it reaches the target activation.
            if (t != this)
            {
                throw;
            }

            // if we're not the current kernel holder when things return, make sure we
            // get the lock before we continue
            if (ActivityManager::currentActivity != activity)
            {
                activity->requestAccess();
            }

            // unwind the activation stack back to our frame
            activity->unwindToFrame(this);

            // do the normal between clause clean up.
            stack.clear();
            settings.timeStamp.valid = false;

            // if we were in a debug pause, we had a error interpreting the
            // line typed at the pause.  We're just going to terminate this
            // like it was a return
            if (debugPause)
            {
                stopExecution(RETURNED);
            }

            // we might have caught a condition.  See if we have something to do.
            if (conditionQueue != OREF_NULL)
            {
                // if we have pending traps, process them now
                if (!conditionQueue->isEmpty())
                {
                    processTraps();
                    // processing the traps might have deferred handling until clause
                    // termination (CALL ON conditions)...turn on the clause_boundary
                    // flag to check for them after instruction completiong.
                    if (!conditionQueue->isEmpty())
                    {
                        clauseBoundary = true;
                    }
                }
            }
        }
    }
}


/**
 * process pending condition traps before going on to execute a
 * clause
 */
void RexxActivation::processTraps()
{
    size_t i = conditionQueue->items();

    // process each item currently in the queue
    while (i--)
    {
        // get the next handler off the queue
        TrapHandler *trapHandler = (TrapHandler *)conditionQueue->pull();
        // condition in DELAY state?
        if (trapHandler->isDelayed())
        {
            // add to the end of the queue
            conditionQueue->append(trapHandler);
        }
        else
        {
            // get the condition object from the current trap handler
            DirectoryClass *conditionObj = trapHandler->getConditionObject();
            // see if we have something to assign to the RC variable
            RexxInternalObject *rc = conditionObj->get(GlobalNames::RC);
            if (rc != OREF_NULL)
            {
                setLocalVariable(GlobalNames::RC, VARIABLE_RC, (RexxObject *)rc);
            }

            // it's possible that the condition can raise an error because of a
            // missing label, so we need to catch any conditions that might be thrown
            try
            {
                // process the trap
                trapHandler->trap(this);
            }
            catch (RexxActivation *t)
            {
                // if we're not the target of this throw, we've already been unwound
                // keep throwing this until it reaches the target activation.
                if (t != this)
                {
                    throw;
                }

                // if we're not the current kernel holder when things return, make sure we
                // get the lock before we continue
                if (ActivityManager::currentActivity != activity)
                {
                    activity->requestAccess();
                }
            }
        }
    }
}


/**
 * Process a numeric "debug skip" TRACE instruction to suppress
 * pauses or tracing for a given number of instructions.
 *
 * @param skipCount The number of clauses to skip.
 */
void RexxActivation::debugSkip(wholenumber_t skipCount)
{
    // this is only allowed from a debug pause, not normal code execution
    if (!debugPause)
    {
        reportException(Error_Invalid_trace_debug);
    }

    // mark the count to skip
    settings.traceSkip = std::abs(skipCount);
    // turn on the skip flag to suppress the tracing.
    // if the skip count is a negative value, we turn off all
    // tracing, not just the debug pauses.
    settings.setTraceSuppressed(skipCount < 0);
    settings.setDebugBypass(true);
}


/**
 * Generate a string version of the current trace setting.
 *
 * @return The current trace setting formatted into a human-readable
 *         string.
 */
RexxString* RexxActivation::traceSetting()
{
    // get this directly from the package settings.
    return settings.packageSettings.getTrace();
}


/**
 * Set the trace using a dynamically evaluated string.
 *
 * @param setting The new trace setting.
 */
void RexxActivation::setTrace(RexxString *setting)
{
    TraceSetting newSettings;

    char traceOption = 0;              // a potential bad character

    if (!newSettings.parseTraceSetting(setting, traceOption))
    {
        reportException(Error_Invalid_trace_trace, new_string(&traceOption, 1));
    }
    // change the settings to the new value
    setTrace(newSettings);
}


/**
 * Set a new trace setting for the context.
 *
 * @param source The trace setting source.
 */
void RexxActivation::setTrace(const TraceSetting &source)
{
    // turn off any trace suppression
    settings.setTraceSuppressed(false);
    settings.traceSkip = 0;

    // this might just be a debug toggle request.  All other trace
    // settings remain the same, but we flip the debug state to the
    // other mode.
    if (source.isDebugToggle())
    {
        // just flip the debug state
        settings.packageSettings.traceSettings.toggleDebug();
        // if no longer in debug mode, we need to reset the prompt issued flag
        if (!settings.packageSettings.traceSettings.isDebug())
        {
            // flipping out of debug mode.  Reissue the debug prompt when
            // turned back on again
            settings.setDebugPromptIssued(false);
        }
    }
    // are we in debug mode already?  A trace setting with no "?" maintains the
    // debug setting, unless it is Trace Off
    else if (settings.packageSettings.traceSettings.isDebug())
    {
        // merge the flag settings
        settings.packageSettings.traceSettings.merge(source);
        // flipped out of debug mode.  Reissue the debug prompt when
        // turned back on again
        if (!settings.packageSettings.traceSettings.isDebug())
        {
            settings.setDebugPromptIssued(false);
        }
    }
    else
    {
        // set the new flags
        settings.packageSettings.traceSettings.set(source);

    }

    // if tracing intermediates, turn on the special fast check flag
    settings.intermediateTrace = settings.packageSettings.traceSettings.tracingIntermediates();
    // if we issued this from a debug prompt, let the pause handler know this changes.
    if (debugPause)
    {
        settings.setDebugBypass(true);
    }
}



/**
 * Process a REXX REPLY instruction
 *
 * @param resultObj The reply result object.
 */
void RexxActivation::reply(RexxObject *resultObj)
{
    // one reply per activation
    if (settings.isReplyIssued())
    {
        reportException(Error_Execution_reply);
    }
    settings.setReplyIssued(true);

    // set the state to terminate the main execution loop
    stopExecution(REPLIED);
    result = resultObj;
}


/**
 * process a REXX RETURN instruction
 *
 * @param resultObj the return result object.
 */
void RexxActivation::returnFrom(RexxObject *resultObj)
{
    // already had a reply and trying to return a result?  There is nobody
    // to receive this result, so this is an error.
    if (settings.isReplyIssued() && resultObj != OREF_NULL)
    {
        reportException(Error_Execution_reply_return);
    }
    // cause this level to terminate terminate the execution loop and shut down
    stopExecution(RETURNED);
    // if this is an interpret, we really need to terminate the parent activation
    if (isInterpret())
    {
        parent->returnFrom(resultObj);
    }
    // normal termination.
    else
    {
        // save the result object
        result = resultObj;

        // if this is a top level program, we need to call the termination exit.
        if (isProgramLevelCall())
        {
            activity->callTerminationExit(this);
        }
    }
    // switch debug off to avoid debug pause after an exit or return.
    resetDebug();
}


/**
 * Process a REXX ITERATE instruction
 *
 * @param name   The optional block name for the iterate.
 */
void RexxActivation::iterate(RexxString *name)
{
    // get the top item off of the block stack
    DoBlock *doblock = topBlockInstruction();

    // we might need to scan several levels down to find our target block instruction.
    while (doblock != OREF_NULL)
    {
        // get the actual block instruction (might not be a loop, but it needs to be
        // for an iterate
        RexxBlockInstruction *loop = doblock->getParent();
        // Is this a request to iterate the inner=most loop?
        if (name == OREF_NULL)
        {
            // we only recognize LOOP constructs for this.
            if (loop->isLoop())
            {
                // reset the indentation
                setIndent(doblock->getIndent());
                // have the loop handle a re-execution.  This will
                // determine if we continue or terminate.
                ((RexxInstructionBaseLoop *)loop)->reExecute(this, &stack, doblock);
                return;
            }

        }
        // a named LEAVE can be either a labeled block or a loop.
        else if (loop->isLabel(name))
        {
            // if we have a name match, but this is not a loop,
            // that's a problem.
            if (!loop->isLoop())
            {
                reportException(Error_Invalid_leave_iterate_name, name);
            }
            // got our target, reset the indent and do the loop.
            setIndent(doblock->getIndent());
            ((RexxInstructionBaseLoop *)loop)->reExecute(this, &stack, doblock);
            return;
        }
        // terminate this block instruction and step to the
        // the next level.
        popBlockInstruction();
        doblock = topBlockInstruction();
    }

    // if we reached here, we either did not find the named
    // block or there were no active loops at all.
    if (name != OREF_NULL)
    {
        reportException(Error_Invalid_leave_iteratevar, name);
    }
    else
    {
        reportException(Error_Invalid_leave_iterate);
    }
}


/**
 * Process a REXX LEAVE instruction
 *
 * @param name   The potential matching label name (can be null)
 */
void RexxActivation::leaveLoop(RexxString *name)
{
    // scan the block stack looking for a match.
    DoBlock *doblock = topBlockInstruction();

    while (doblock != OREF_NULL)
    {
        // get the instruction backing the block
        RexxBlockInstruction *loop = doblock->getParent();
        // no name means leave the innermost block.  An unnamed
        // LEAVE only works with loops.  If this is not a loop,
        // keep searching for one.
        if (name == OREF_NULL)
        {
            // we only recognize LOOP constructs for this.
            if (loop->isLoop())
            {
                loop->terminate(this, doblock);
                return;
            }
        }
        // a named LEAVE can be either a labeled block or a loop.
        else if (loop->isLabel(name))
        {
            loop->terminate(this, doblock);
            return;
        }

        // top one is not the one we need...remove this block
        // instruction and try the next one.

        popBlockInstruction();
        doblock = topBlockInstruction();
    }

    // if we get here, we did not find anything to leave.  This is either
    // a problem with a mismatched name or there was no active loop available to leave.
    if (name != OREF_NULL)
    {
        reportException(Error_Invalid_leave_leavevar, name);
    }
    else
    {
        reportException(Error_Invalid_leave_leave);
    }
}


/**
 * Return the line number of the current instruction
 *
 * @return The line number of the current instruction.
 */
size_t RexxActivation::currentLine()
{
    // we should have a current instruction.  If we don't, just return a
    // default of 1.
    if (current != OREF_NULL)
    {
        return current->getLineNumber();
    }
    else
    {
        return 1;
    }
}


/**
 * Execute a procedure expose instruction.
 *
 * @param variables The list of variables to expose.
 * @param count     The count of variables to expose.
 */
void RexxActivation::procedureExpose(RexxVariableBase **variables, size_t count)
{
    // make sure procedure is valid here
    if (!settings.isProcedureValid())
    {
        reportException(Error_Unexpected_procedure_call);
    }
    // disable further procedure instructions
    settings.setProcedureValid(false);

    // allocate a new variable frame for an internal call (we inherited the
    // caller's variable frame originally)
    activity->allocateLocalVariableFrame(&settings.localVariables);
    // make sure we clear out the dictionary, otherwise we'll see the
    // dynamic entries from the previous level.
    settings.localVariables.procedure(this);

    // now expose each individual variable
    for (size_t i = 0; i < count; i++)
    {
        variables[i]->procedureExpose(this, parent);
    }
}


/**
 * Expose variables for an EXPOSE instruction
 *
 * @param variables The list of variables to expose.
 * @param count     The variable count.
 */
void RexxActivation::expose(RexxVariableBase **variables, size_t count)
{
    // get the object variables for this object (at the current scope)
    VariableDictionary *objectVariables = getObjectVariables();

    // now expose each individual variable
    for (size_t i = 0; i < count; i++)
    {
        variables[i]->expose(this, objectVariables);
    }
}


/**
 * Turn on autoexpose as the result of a USE LOCAL instruction.
 *
 * @param variables The list of variables to declose as local
 *                  variables.
 * @param count     The variable count.
 */
void RexxActivation::autoExpose(RexxVariableBase **variables, size_t count)
{
    // we just request the value for each of these variables now, which will
    // force them to be created as local variables.
    for (size_t i = 0; i < count; i++)
    {
        variables[i]->getRealValue(this);
    }

    // now explicitly make RC, RESULT, SIGL, SELF, and SUPER local
    getLocalVariable(GlobalNames::SELF, VARIABLE_SELF);
    getLocalVariable(GlobalNames::SUPER, VARIABLE_SUPER);
    getLocalVariable(GlobalNames::RC, VARIABLE_RC);
    getLocalVariable(GlobalNames::SIGL, VARIABLE_SIGL);
    getLocalVariable(GlobalNames::RESULT, VARIABLE_RESULT);

    // now switch modes with the local variables so that every new variable
    // is created as an object variable.
    settings.localVariables.setAutoExpose(getObjectVariables());
}


/**
 * Process a forward instruction.
 *
 * @param target     The target object.
 * @param message    The message name.
 * @param superClass The superclass override (if any)
 * @param arguments  The message arguments.
 * @param argcount   The count of message arguments.
 * @param continuing The continue flag.
 *
 * @return The message result.
 */
RexxObject* RexxActivation::forward(RexxObject  *target, RexxString  *message,
                                    RexxClass *superClass, RexxObject **arguments, size_t argcount, bool continuing)
{
    // all pieces that are a note specified on the FORWARD will use the
    // contgext values.
    if (target == OREF_NULL)
    {
        target = receiver;
    }
    if (message == OREF_NULL)
    {
        message = settings.messageName;
    }
    if (arguments == OREF_NULL)
    {
        arguments = argList;
        argcount = argCount;
    }
    // if we are continuing, this is just a message send
    if (continuing)
    {
        ProtectedObject r;
        if (superClass == OREF_NULL)
        {
            target->messageSend(message, arguments, argcount, r);
        }
        else
        {
            target->messageSend(message, arguments, argcount, superClass, r);
        }
        return r;
    }
    // this activation becomes a phantom, and we issued the
    // message as if we don't exist.
    else
    {
        // cannot return a result if a reply has already been issued.
        if (settings.isReplyIssued() && result != OREF_NULL)
        {
            reportException(Error_Execution_reply_exit);
        }
        // poof, we just became invisible
        settings.setForwarded(true);
        // we terminate the execution loop for this activation
        stopExecution(RETURNED);
        // switch off debug for this activation so we don't pause after
        // returning from the forward
        resetDebug();
        ProtectedObject r;
        // now issue the message
        if (superClass == OREF_NULL)
        {
            target->messageSend(message, arguments, argcount, r);
        }
        else
        {
            target->messageSend(message, arguments, argcount, superClass, r);
        }
        // set the return value for end-of-activation processing to handle.
        result = r;
        // terminate this activation
        termination();
        return OREF_NULL;
    }
}


/**
 * Process a REXX exit instruction
 *
 * @param resultObj The result object from the exit (optional)
 */
void RexxActivation::exitFrom(RexxObject *resultObj)
{
    // stop the loop execution
    stopExecution(RETURNED);
    result = resultObj;
    // switch off debug pausing
    resetDebug();

    // if we already had a reply issued, we can't return a result on EXIT
    if (isTopLevelCall())
    {
        if (settings.isReplyIssued() && result != OREF_NULL)
        {
            reportException(Error_Execution_reply_exit);
        }

        // run the termination exit if we need to
        if (isProgramLevelCall())
        {
            activity->callTerminationExit(this);
        }
        // terminate this level
        termination();
    }
    else
    {
        // start terminating with this level
        RexxActivation *activation = this;
        do
        {
            // terminate this level
            activation->termination();
            // pop from the activity stack
            ActivityManager::currentActivity->popStackFrame(false);
            //. go to the next level
            activation = ActivityManager::currentActivity->getCurrentRexxFrame();
        }
        while (!activation->isTopLevel());

        // we are at the main program level, tell it to exit now
        activation->exitFrom(resultObj);
        throw activation;                  // throw this as an exception to start the unwinding
    }
}


/**
 * Process a "fall off the end" exit condition
 */
void RexxActivation::implicitExit()
{
    // at a main program level or completing an INTERPRET instruction?
    if (isTopLevelCall() || isInterpret())
    {
        // real program call?  we might have a termination exit to call
        if (isProgramLevelCall())
        {
            activity->callTerminationExit(this);
        }
        // we are terminating
        executionState = RETURNED;
        return;
    }
    // we've had a nested exit, we need to process this more fully
    exitFrom(OREF_NULL);
}


/**
 * do any cleanup due to a program terminating.
 */
void RexxActivation::termination()
{
    // remove any guard locks for this activaty.
    guardOff();
    // have any setlocals we need to undo?
    if (environmentList != OREF_NULL && !environmentList->isEmpty())
    {
        // restore the environment to the first version.
        SystemInterpreter::restoreEnvironment(((BufferClass *)environmentList->getLastItem())->getData());
    }

    environmentList = OREF_NULL;
    // close any open streams
    closeStreams();

    // release the stack frame, which also releases the frame for the variable cache
    activity->releaseStackFrame(stack.getFrame());
    // do the variable termination
    cleanupLocalVariables();
    // deactivate the context object if we created one.
    if (contextObject != OREF_NULL)
    {
        contextObject->detach();
    }
    // since we don't always control the order of garbage collection and the
    // argument list source, clear the arglist pointers as a belt-and-braces
    // situation.
    argList = OREF_NULL;
    argCount = 0;
}


/**
 * Create/copy a trap table as needed
 */
void RexxActivation::checkTrapTable()
{
    // no trap table created yet?  just create a new collection
    if (settings.traps == OREF_NULL)
    {
        settings.traps = new_string_table();
    }
    // have to copy the trap table for an internal routine call?
    else if (isInternalCall() && !settings.areTrapsCopied())
    {
        // copy the table and remember that we've done that
        settings.traps = (StringTable *)settings.traps->copy();
        settings.setTrapsCopied(true);
    }
}


/**
 * Activate a condition trap.
 *
 * @param condition The condition name being trapped.
 * @param handler   The instruction handling the trap.
 */
void RexxActivation::trapOn(RexxString *condition, RexxInstructionTrapBase *handler, bool signal)
{
    // make sure we have a trap table (we create on demand)
    checkTrapTable();

    // add a state block to our current trap list
    settings.traps->put(new TrapHandler(condition, handler), condition);
    // if this is NOVALUE or ANY, then we need to flip on the switch in the
    // local variables indicating we're interested in NOVALUE events.
    // (we only need to check this for SIGNAL, not for CALL)
    if (signal && (condition->strCompare(GlobalNames::NOVALUE) || condition->strCompare(GlobalNames::ANY)))
    {
        settings.localVariables.setNovalueOn();
        // we also need to disable the novalue error setting from ::OPTIONS in order for the
        // events to be raised.
        disableNovalueSyntax();
    }
    // we also disable an OPTIONS condition SYNTAX that we may have
    bool conditionIsAny = condition->strCompare(GlobalNames::ANY);
    if (isErrorSyntaxEnabled() &&
       (conditionIsAny || condition->strCompare(GlobalNames::ERRORNAME)))
    {
        disableErrorSyntax();
    }
    if (isFailureSyntaxEnabled() &&
       (conditionIsAny || condition->strCompare(GlobalNames::FAILURE)))
    {
        disableFailureSyntax();
    }
    if (signal && isLostdigitsSyntaxEnabled() &&
       (conditionIsAny || condition->strCompare(GlobalNames::LOSTDIGITS)))
    {
        disableLostdigitsSyntax();
    }
    if (signal && isNostringSyntaxEnabled() &&
       (conditionIsAny || condition->strCompare(GlobalNames::NOSTRING)))
    {
        disableNostringSyntax();
    }
    if (isNotreadySyntaxEnabled() &&
       (conditionIsAny || condition->strCompare(GlobalNames::NOTREADY)))
    {
        disableNotreadySyntax();
    }
}


/**
 * Disable a condition trap
 *
 * @param condition The name of the condition we're turning off.
 */
void RexxActivation::trapOff(RexxString *condition, bool signal)
{
    checkTrapTable();

    // remove our existing trap.
    settings.traps->remove(condition);
    // if we no longer have NOVALUE or ANY enabled, then we can turn
    // off novalue processing in the variable pool
    // (we only need to check this for SIGNAL, not for CALL)
    bool conditionIsAny = condition->strCompare(GlobalNames::ANY);
    if (signal &&
       (conditionIsAny || condition->strCompare(GlobalNames::NOVALUE)) &&
       !settings.traps->hasIndex(GlobalNames::NOVALUE) && !settings.traps->hasIndex(GlobalNames::ANY))
    {
        settings.localVariables.setNovalueOff();
        // we also need to disable the novalue error setting from ::OPTIONS in order to restore
        // the real default behavior.
        disableNovalueSyntax();
    }

    // we also disable an OPTIONS condition SYNTAX that we may have
    if (isErrorSyntaxEnabled() &&
       (conditionIsAny || condition->strCompare(GlobalNames::ERRORNAME)))
    {
        disableErrorSyntax();
    }
    if (isFailureSyntaxEnabled() &&
       (conditionIsAny || condition->strCompare(GlobalNames::FAILURE)))
    {
        disableFailureSyntax();
    }
    if (signal && isLostdigitsSyntaxEnabled() &&
       (conditionIsAny || condition->strCompare(GlobalNames::LOSTDIGITS)))
    {
        disableLostdigitsSyntax();
    }
    if (signal && isNostringSyntaxEnabled() &&
       (conditionIsAny || condition->strCompare(GlobalNames::NOSTRING)))
    {
        disableNostringSyntax();
    }
    if (isNotreadySyntaxEnabled() &&
       (conditionIsAny || condition->strCompare(GlobalNames::NOTREADY)))
    {
        disableNotreadySyntax();
    }
}


/**
 * Create/copy an io config table as needed
 */
void RexxActivation::checkIOConfigTable()
{
    // no table created yet?  just create a new collection
    if (settings.ioConfigs == OREF_NULL)
    {
        settings.ioConfigs = new_string_table();
    }
    // have to copy the trap table for an internal routine call?
    else if (isInternalCall() && !settings.isIOConfigCopied())
    {
        // copy the table and remember that we've done that
        settings.ioConfigs = (StringTable *)settings.ioConfigs->copy();
        settings.setIOConfigCopied(true);
    }
}


/**
 * Retrieve the IO configuration for an address enviroment,
 * if it exists
 *
 * @param environment
 *               The name of the environment
 *
 * @return The configuration (if any) associated with the environment name.
 */
CommandIOConfiguration* RexxActivation::getIOConfig(RexxString *environment)
{
    // no config table means no need to check
    if (settings.ioConfigs == OREF_NULL)
    {
        return OREF_NULL;
    }

    Protected<RexxString> name = environment->upper();

    // see if we have one for this environment name (always upper case the name)
    return (CommandIOConfiguration *)settings.ioConfigs->get(name);
}


/**
 * Add an i/o configuration to the config table.
 *
 * @param environment
 *               The name of the address environment
 * @param config The config we're adding
 */
void RexxActivation::addIOConfig(RexxString *environment, CommandIOConfiguration *config)
{
    // create or copy the config table as required
    checkIOConfigTable();
    Protected<RexxString> name = environment->upper();

    settings.ioConfigs->put(config, name);
}


/**
 * Return the top level external activation
 *
 * @return The main external call (a top level call or an external routine invocation)
 */
RexxActivation* RexxActivation::external()
{
    // if an internal call or an interpret, we need to pass this /* along                             */
    if (isInternalLevelCall())
    {
        return parent->external();
    }
    // got the one we need
    else
    {
        return this;
    }
}


/**
 * Raise a condition using exit semantics for the returned value.
 *
 * @param condition  The condition to raise.
 * @param rc         The RC value for the condition.
 * @param description
 *                   The condition description
 * @param additional The addtional information for the condition.
 * @param resultObj  The result object.
 * @param conditionobj
 *                   Any propagated condition object.
 */
void RexxActivation::raiseExit(RexxString *condition, RexxObject *rc, RexxString *description,
                               RexxObject *additional, RexxObject *resultObj, DirectoryClass *conditionobj)
{
    // if we are a top level activation already, just do the raise part now.
    if (isTopLevelCall())
    {
        raise(condition, rc, description, additional, resultObj, conditionobj);
        return;
    }

    // are we at the top level?  This is basically an
    // exit instruction because there's nobody able to handle this.
    if (parent == OREF_NULL)
    {
        exitFrom(resultObj);
    }
    else
    {
        // if we're the top level of the program, run the termination exit
        if (isProgramLevelCall())
        {
            activity->callTerminationExit(this);
        }
        ProtectedObject p(this);
        // terminate the activaiton and remove from the stack
        termination();
        activity->popStackFrame(false);
        // propagate to the parent
        parent->raiseExit(condition, rc, description, additional, resultObj, conditionobj);
    }
}


/**
 * Raise a condition in the context.
 *
 * @param condition  the condition name,
 * @param rc         The rc value for the condition.
 * @param description
 *                   The condition description.
 * @param additional any condition additional information.
 * @param resultObj  The result object associated with the raise.
 * @param conditionobj
 *                   The condition object used for a propagate.
 */
void RexxActivation::raise(RexxString *condition, RexxObject *rc, RexxString *description,
                           RexxObject *additional, RexxObject *resultObj, DirectoryClass *conditionobj)
{
    bool propagated = false;

    Protected<DirectoryClass> p = conditionobj;

    // are we propagating an an existing condition?
    if (condition->strCompare(GlobalNames::PROPAGATE))
    {
        // get the original condition name
        condition = (RexxString *)conditionobj->get(GlobalNames::CONDITION);
        propagated = true;
        // fill in the propagation status
        conditionobj->put(TheTrueObject, GlobalNames::PROPAGATED);
        // if no result was specified, use the one from the condition object
        if (resultObj == OREF_NULL)
        {
            resultObj = (RexxObject *)conditionobj->get(GlobalNames::RESULT);
        }
        // now fill in other pieces from the raise instruction                                  /
        if (rc != OREF_NULL)
        {
            conditionobj->put(rc, GlobalNames::RC);
        }
        if (description != OREF_NULL)
        {
            conditionobj->put(description, GlobalNames::DESCRIPTION);
        }
        if (additional != OREF_NULL)
        {
            conditionobj->put(additional, GlobalNames::ADDITIONAL);
        }
        if (resultObj != OREF_NULL)
        {
            conditionobj->put(resultObj, GlobalNames::RESULT);
        }
    }
    else
    {
        // build a condition object for the condition
        p = activity->createConditionObject(condition, rc, description, additional, result);

        conditionobj = p;
        conditionobj->put(TheFalseObject, GlobalNames::PROPAGATED);
        // not propagated
        propagated = false;
    }

    // is this a SYNTAX error?  These get special handling
    if (condition->strCompare(GlobalNames::SYNTAX))
    {
        // if propagating, terminate this level and reraise the condition
        // with earlier levels.
        if (propagated)
        {
            // protect this activation after we get popped from the stack
            ProtectedObject p(this);
            termination();
            activity->popStackFrame(false);
            ActivityManager::currentActivity->reraiseException(conditionobj);
        }
        else
        {
            // raise the error now at this level.
            ActivityManager::currentActivity->raiseException((RexxErrorCodes)((RexxInteger *)rc)->getValue(), description, (ArrayClass *)additional, resultObj);
        }
    }
    else
    {
        // find a predecessor Rexx activation
        ActivationBase *_sender = senderActivation(condition);
        // see if the sender level is trapping this condition
        bool trapped = false;
        if (_sender != OREF_NULL)
        {
            trapped = _sender->trap(condition, conditionobj);
        }

        // If this was untrapped and either a HALT or a NOMETHOD condition, then we need to raise an error
        if (!trapped)
        {
            if (condition->strCompare(GlobalNames::HALT))
            {
                reportException(Error_Program_interrupted_condition, GlobalNames::HALT);
            }
            // untrapped NOMETHOD conditions are also defined as giving an error. This is a little
            // trickier because the normal NOMETHOD error message includes both the receive and the message
            // name. If we have enough additional information for the full message, we'll use that, otherwise
            // we'll use a generic message that doesn't require substitutions.
            else if (condition->strCompare(GlobalNames::NOMETHOD))
            {
                // this might be a propagate, in which case additional and description might not be
                // set. Pull the current from the condition object.
                additional = (RexxObject *)conditionobj->get(GlobalNames::ADDITIONAL);
                description = (RexxString *)conditionobj->get(GlobalNames::DESCRIPTION);

                // this is a CONDITION, not a syntax error at this point. The ADDITIONAL information
                // is the receiver object, the message name is the description
                if (additional != OREF_NULL && description != OREF_NULL)
                {
                    reportException(Error_No_method_name, additional, description);
                }

                // if the error was not issued above, we fall through to here
                reportException(Error_No_method_unhandled);
            }
        }

        // process the return part, then unwind the call stack
        returnFrom(resultObj);
        throw this;
    }
}


/**
 * Return the object variables dictionary for the current
 * scope level.
 *
 * @return the target object variables.
 */
VariableDictionary* RexxActivation::getObjectVariables()
{
    // not retrieved yet?  We need the dictionary from the current method scope.
    if (settings.objectVariables == OREF_NULL)
    {
        settings.objectVariables = receiver->getObjectVariables(scope);
        // are we a guarded method?  Get the guard lock now.
        if (isGuarded())
        {
            settings.objectVariables->reserve(activity);
            objectScope = SCOPE_RESERVED;
        }
    }
    return settings.objectVariables;
}


/**
 * Resolve a stream name for a BIF call.
 *
 * @param name     The name of the stream.
 * @param stack    The expression stack.
 * @param input    The input/output flag.
 * @param fullName The returned full name of the stream.
 * @param added    A flag indicating we added this.  Set this
 *                 pointer to NULL for a lookup without adding
 *                 to the stream table.
 *
 * @return The backing stream object for the name.
 */
RexxObject* RexxActivation::resolveStream(RexxString *name, bool input, Protected<RexxString> &fullName, bool *added)
{
    bool newName = true;
    bool addName = false;
    // if the file system is NOT case sensitive, remember the qualified name
    StringTable *fileNames;
    // when the caller requires a stream table entry, then set the initial indicator.
    if (added != NULL)
    {
        *added = false;
    }
    StringTable *streamTable = getStreams();
    // the default full name is the initial one
    fullName = name;
    // if length of name is 0, then it's the same as omitted.  This is
    // a request for either the default input or output stream.  The flag
    // tells us which one.
    if (name == OREF_NULL || name->getLength() == 0)
    {
        if (input)
        {
            return getLocalEnvironment(GlobalNames::INPUT);
        }
        else
        {
            return getLocalEnvironment(GlobalNames::OUTPUT);
        }
    }
    // check for the standard input or out put streams.
    else if (name->strCaselessCompare("STDIN") || name->strCaselessCompare("STDIN:"))
    {
        return getLocalEnvironment(GlobalNames::INPUT);
    }
    else if (name->strCaselessCompare("STDOUT") || name->strCaselessCompare("STDOUT:"))
    {
        return getLocalEnvironment(GlobalNames::OUTPUT);
    }
    else if (name->strCaselessCompare("STDERR") || name->strCaselessCompare("STDERR:"))
    {
        return getLocalEnvironment(GlobalNames::ERRORNAME);
    }
    // not one of the standards...go looking for a file.
    else
    {
        RexxString *qualifiedName;
        if (notCaseSensitive()) // probably Windows
        {
            fileNames = getFileNames();
            qualifiedName = (RexxString *)fileNames->get(name);
            if (qualifiedName != OREF_NULL) // we have seen this name before
            {
                fullName = qualifiedName;
                newName = false;    // don't redo the system calls
            }
            else
            {
                // add to fileNames only if the stream is going to be added
                addName = (added != NULL);
            }

        }
        if (newName)
        {
            // get the fully qualified name
            qualifiedName = Interpreter::qualifyFileSystemName(name);
            fullName = qualifiedName;
        }
        if (addName)
        {
            // add the name to the fileNames table for future requests
            fileNames->put(qualifiedName, name);
        }
        // see if we have this in the table already.  If not opened yet, we need
        // to try to open it.
        RexxObject *stream = (RexxObject *)streamTable->get(qualifiedName);
        if (stream == OREF_NULL)
        {
            // do the security manager check first.
            SecurityManager *manager = getEffectiveSecurityManager();
            stream = manager->checkStreamAccess(qualifiedName);
            if (stream != OREF_NULL)
            {
                streamTable->put(stream, qualifiedName);
                return stream;
            }
            // create an instance of the stream class and create a new
            // instance
            RexxObject *t = OREF_NULL;   // required for the findClass call
            RexxClass *streamClass = TheRexxPackage->findClass(GlobalNames::STREAM, t);


            ProtectedObject result;
            stream = streamClass->sendMessage(GlobalNames::NEW, name, result);

            // if we're requested to add this to the table, add it in and return the indicator.
            if (added != NULL)
            {
                streamTable->put(stream, qualifiedName);
                *added = true;
            }
        }
        return stream;
    }
}


/**
 * Return the associated object variables stream table
 *
 * @return The table of opened streams.
 */
StringTable* RexxActivation::getStreams()
{
    // first request for a stream object?  We need to
    // create the table.
    if (settings.streams == OREF_NULL)
    {
        // if we are a top-level program or a method call,
        // we just set a new directory.  NOTE:  For legacy reasons,
        // external routine calls need to inherit the table
        // from the main program context, so this is not a
        // isTopLevelCall() test.
        if (isProgramOrMethod())
        {
            settings.streams = new_string_table();
        }
        else
        {
            // get the caller frame.  If it is not a Rexx one, then
            // we use a fresh stream table
            ActivationBase *callerFrame = getPreviousStackFrame();
            if (callerFrame == OREF_NULL || !callerFrame->isRexxContext())
            {
                settings.streams = new_string_table();
            }
            else
            {

                // alway's use caller's for internal call, external call or interpret
                settings.streams = ((RexxActivation *)callerFrame)->getStreams();
            }
        }
        // determine if the file system is case insensitive or not and save it
        settings.caseInsensitive = !SysFileSystem::isCaseSensitive();
    }
    return settings.streams;
}


/**
 * Signal to a target instruction
 *
 * @param target The target of the signal instruction.
 */
void RexxActivation::signalTo(RexxInstruction *target)
{
    // if this is an interpret or debug pause, we need to shut this
    // activation down and have the parent execute the signal.
    if (isInterpret())
    {
        stopExecution(RETURNED);
        parent->signalTo(target);
    }
    // need to clean up some things in a SIGNAL.  Start with
    // setting the SIGL variable to the current linenumber, then
    // clear out all of our block instruction states
    else
    {
        size_t lineNum = current->getLineNumber();
        setLocalVariable(GlobalNames::SIGL, VARIABLE_SIGL, new_integer(lineNum));
        // the target is the next instruction we execute.
        next = target;
        doStack = OREF_NULL;
        blockNest = 0;
        settings.traceIndent = 0;
    }
}


/**
 * Toggle the address setting between the current and alternate
 */
void RexxActivation::toggleAddress()
{
    RexxString *temp = settings.currentAddress;
    settings.currentAddress = settings.alternateAddress;
    settings.alternateAddress = temp;
}


/**
 * Set the new current address, moving the current one to the
 * alternate address
 *
 * @param address The new address setting.  The current setting becomes
 *                the alternate.
 */
void RexxActivation::setAddress(RexxString *address, CommandIOConfiguration *config)
{
    settings.alternateAddress = settings.currentAddress;
    settings.currentAddress = address;
    // if a config was specified, then make it the current for this
    // environment
    if (config != OREF_NULL)
    {
        // keep this in the table
        addIOConfig(address, config);
    }
}


/**
 * Set up a default address environment so that both the primary
 * and the alternate address are the same value
 *
 * @param address The new default address name.
 */
void RexxActivation::setDefaultAddress(RexxString *address)
{
    settings.alternateAddress = address;
    settings.currentAddress = address;
}


/**
 * Signal to a computed label target
 *
 * @param name   The computed string name.
 */
void RexxActivation::signalValue(RexxString *name)
{
    RexxInstruction *target = OREF_NULL;
    // get the label table from the current code context.
    // we might not have any labels in the code, so
    // only perform the lookup if the table is there.

    StringTable *labels = getLabels();
    if (labels != OREF_NULL)
    {
        target = (RexxInstruction *)labels->get(name);
    }

    // no table or an unknown label?  Raise the error now.
    if (target == OREF_NULL)
    {
        reportException(Error_Label_not_found_name, name);
    }
    // continue processing like a normal signal instruction.
    signalTo(target);
}


/**
 * Turn on the activation guarded state
 */
void RexxActivation::guardOn()
{
    // a guard on request while already guarded is a nop.
    if (objectScope == SCOPE_RELEASED)
    {
        // first access to the scope object variables?  Go get the variables.
        if (settings.objectVariables == OREF_NULL)
        {
            settings.objectVariables = receiver->getObjectVariables(scope);
        }
        // now lock the dictionary and flip our state.
        settings.objectVariables->reserve(activity);
        objectScope = SCOPE_RESERVED;
    }
}


/**
 * Return the current digits setting
 *
 * @return The package digits setting
 */
wholenumber_t RexxActivation::digits()
{
    return settings.packageSettings.getDigits();
}


/**
 * Return the current fuzz setting
 *
 * @return The package fuzz setting
 */
wholenumber_t RexxActivation::fuzz()
{
    return settings.packageSettings.getFuzz();
}


/**
 * Return the current form setting
 *
 * @return The package form setting
 */
bool RexxActivation::form()
{
    return settings.packageSettings.getForm();
}


/**
 * Set a new digits setting
 *
 * @param digitsVal The new digits value.
 */
void RexxActivation::setDigits(wholenumber_t digitsVal)
{
    settings.packageSettings.setDigits(digitsVal);
    if (isInterpret())
    {
        // .context in an INTERPRET should pick up changes too
        parent->setDigits(digitsVal);
    }
}


/**
 * Set a new fuzz setting
 *
 * @param fuzzVal   The new fuzz value.
 */
void RexxActivation::setFuzz(wholenumber_t fuzzVal)
{
    settings.packageSettings.setFuzz(fuzzVal);
    if (isInterpret())
    {
        // .context in an INTERPRET should pick up changes too
        parent->setFuzz(fuzzVal);
    }
}

/**
 * Set a new form setting
 *
 * @param formVal   The new form setting
 */
void RexxActivation::setForm(bool formVal)
{
    settings.packageSettings.setForm(formVal);
    if (isInterpret())
    {
        // .context in an INTERPRET should pick up changes too
        parent->setForm(formVal);
    }
}


/**
 * Return the Rexx context this operates under.  Depending on the
 * context, this could be null.
 *
 * @return The parent Rexx context.
 */
RexxActivation* RexxActivation::getRexxContext()
{
    return this;          // I am my own grampa...I mean Rexx context.
}


/**
 * Return the Rexx context this operates under.  Depending on the
 * context, this could be null.
 *
 * @return The parent Rexx context.
 */
RexxActivation* RexxActivation::findRexxContext()
{
    return this;          // I am my own grampa...I mean Rexx context.
}


/**
 * Indicate whether this activation is a Rexx context or not.
 *
 * @return true if this is a Rexx context, false otherwise.
 */
bool RexxActivation::isRexxContext()
{
    return true;
}


/**
 * Get the numeric settings for the current context.
 *
 * @return The new numeric settings.
 */
const NumericSettings* RexxActivation::getNumericSettings()
{
    return &settings.packageSettings.numericSettings;
}


/**
 * Get the message receiver
 *
 * @return The message receiver.  Returns OREF_NULL if this is not
 *         a message activation.
 */
RexxObject* RexxActivation::getReceiver()
{
    if (isInterpret())
    {
        return parent->getReceiver();
    }
    return receiver;
}


/**
 * Get the active method object (if a method call)
 *
 * @return The frame method object.  Returns OREF_NULL if this
 *         is not a message activation.
 */
MethodClass* RexxActivation::getMethod()
{
    // if this is an interpreter frame get this from the parent context.
    if (isInterpret())
    {
        return parent->getMethod();
    }
    // only return the executable if this is a method call
    if (isMethod())
    {
        return (MethodClass *)executable;
    }
    return OREF_NULL;
}


/**
 * Return the current state for a trap as either ON, OFF, or DELAY
 *
 * @param condition The condition name.
 *
 * @return The trap state, either ON, OFF, or DELAY.
 */
RexxString* RexxActivation::trapState(RexxString *condition)
{
    // default to OFF
    RexxString *state = GlobalNames::OFF;
    // no enabled traps?  must be off
    if (settings.traps != OREF_NULL)
    {
        // see if the trap is enabled, if we have this, query the state
        TrapHandler *trapHandler = (TrapHandler *)settings.traps->get(condition);
        if (trapHandler != OREF_NULL)
        {
            return trapHandler->getState();
        }
    }
    return state;
}


/**
 * Put a trap into the delay state while executing a CALL
 * ON.
 *
 * @param condition The condition name.
 */
void RexxActivation::trapDelay(RexxString *condition)
{
    checkTrapTable();
    // if we have a trap, put it into the delay state
    TrapHandler *trapHandler = (TrapHandler *)settings.traps->get(condition);
    if (trapHandler != OREF_NULL)
    {
        trapHandler->disable();
    }
}


/**
 * Remove a trap from the DELAY state
 *
 * @param condition The condition being processed.
 */
void RexxActivation::trapUndelay(RexxString *condition)
{
    checkTrapTable();
    // if we have a trap, put it into the delay state
    TrapHandler *trapHandler = (TrapHandler *)settings.traps->get(condition);
    if (trapHandler != OREF_NULL)
    {
        trapHandler->enable();
    }
}


/**
 * Check the activation to see if this is trapping a condition.
 * For SIGNAL traps, control goes back to the point of the trap
 * via throw.  For CALL ON traps, the condition is saved, and
 * the method returns true to indicate the trap was handled.
 *
 * @param condition The name of the raised condition.
 * @param exceptionObject
 *                  The exception object associated with the condition.
 *
 * @return true if the condition was trapped and handled, false otherwise.
 */
bool RexxActivation::trap(RexxString *condition, DirectoryClass *exceptionObject)
{
    // if we're in the act of processing a FORWARD instruction, then this
    // stack frame doesn't really exist any more.  We need to check the previous
    // stack frame to see if it can handle this.
    if (settings.isForwarded())
    {
        ActivationBase *activation = getPreviousStackFrame();
        // we can have multiple forwardings in process, so keep drilling until we
        // find a non-forwarded frame
        while (activation != OREF_NULL && isOfClass(Activation, activation))
        {
            // we've found a non-ghost frame, so have it try to handle this.
            if (!activation->isForwarded())
            {
                return activation->trap(condition, exceptionObject);
            }
            activation = activation->getPreviousStackFrame();
        }
        // we are not really here, so we can't handle this
        return false;
    }
    // do we need to notify a message object of a syntax error?
    // send it the notification message.
    if (notifyObject != OREF_NULL && condition->strCompare(GlobalNames::SYNTAX))
    {
        notifyObject->error(exceptionObject);
    }

    // are we in a debug pause?  ignore any condition other than a syntax error.
    if (debugPause)
    {
        if (!condition->strCompare(GlobalNames::SYNTAX))
        {
            return false;
        }
        // display the errors encountered during debug, then do the
        // error unwind to terminate the debug pause activation.
        activity->displayDebug(exceptionObject);
        throw this;
    }
    // no trap table set yet?  can't handle this
    if (settings.traps == OREF_NULL)
    {
        return false;
    }

    // see if we have a handler for this condition
    TrapHandler *trapHandler = (TrapHandler *)settings.traps->get(condition);

    // nothing there for the specific condition.  We could have an ANY
    // trap enabled, so check that.
    if (trapHandler == OREF_NULL)
    {

        trapHandler = (TrapHandler *)settings.traps->get(GlobalNames::ANY);
        // if we have a handler, but this can't handle this can condition, return false
        if (trapHandler != OREF_NULL && !trapHandler->canHandle(condition))
        {
            return false;
        }
    }
    // if the condition is being trapped, do the CALL or SIGNAL
    if (trapHandler != OREF_NULL)
    {
        // if this is a halt condition, we might need to call the system exit.
        if (condition->strCompare(GlobalNames::HALT))
        {
            activity->callHaltClearExit(this);
        }

        // create a pending queue if we don't have one yet
        if (conditionQueue == OREF_NULL)
        {
            conditionQueue = new_queue();
        }

        // add the instruction trap info
        exceptionObject->put(trapHandler->instructionName(), GlobalNames::INSTRUCTION);
        // set the condition object into the traphandler
        trapHandler->setConditionObject(exceptionObject);
        // add the handler to the condition queue
        conditionQueue->append(trapHandler);
        // clear this from the activity if we're trapping this here
        activity->clearCurrentCondition();

        // if the handler is a SIGNAL, then we unwind everything now without returning
        if (trapHandler->isSignal())
        {
            // if not an interpret, then we can just throw this and unwind the
            // stack.
            if (!isInterpret())
            {
                throw this;
            }
            // if we're interpreted, this needs to be handled in the parent
            // activaiton.
            else
            {
                parent->mergeTraps(conditionQueue);
                parent->unwindTrap(this);
            }
        }
        else
        {
            // we're going to need to process this trap at the clause boundary.
            clauseBoundary = true;
            // we've handled this
            return true;
        }
    }
    // not something we can handle.
    return false;
}


/**
 * Check the activation to see if this is trapping a condition.
 * This does not process the trap, but merely indicates that
 * this activation WILL trap the condition.  This is a
 * preliminary check that allows a condition to be raised
 * without having to construct a condition object first.  This
 * can be critical for conditions that are not normally trapped,
 * like NOSTRING or LOSTDIGITS.
 *
 * @param condition The name of the raised condition.
 * @param exceptionObject
 *                  The exception object associated with the condition.
 *
 * @return true if the condition was trapped and handled, false otherwise.
 */
bool RexxActivation::willTrap(RexxString *condition)
{
    // if we're in the act of processing a FORWARD instruction, then this
    // stack frame doesn't really exist any more.  We need to check the previous
    // stack frame to see if it can handle this.
    if (settings.isForwarded())
    {
        ActivationBase *activation = getPreviousStackFrame();
        // we can have multiple forwardings in process, so keep drilling until we
        // find a non-forwarded frame
        while (activation != OREF_NULL && isOfClass(Activation, activation))
        {
            // we've found a non-ghost frame, so have it try to handle this.
            if (!activation->isForwarded())
            {
                return activation->willTrap(condition);
            }
            activation = activation->getPreviousStackFrame();
        }
        // we are not really here, so we can't handle this
        return false;
    }

    // are we in a debug pause?  ignore any condition other than a syntax error.
    if (debugPause)
    {
        if (!condition->strCompare(GlobalNames::SYNTAX))
        {
            return false;
        }
        return true;
    }

    // no trap table set yet?  can't handle this
    if (settings.traps == OREF_NULL)
    {
        return false;
    }

    // see if we have a handler for this condition
    TrapHandler *trapHandler = (TrapHandler *)settings.traps->get(condition);

    // if we are trapping this specifically, let the caller know.
    if (trapHandler != OREF_NULL)
    {
        return true;
    }

    // now try for an ANY trap
    trapHandler = (TrapHandler *)settings.traps->get(GlobalNames::ANY);
    // if we have a handler, but this can't handle this can condition, return false
    if (trapHandler != OREF_NULL)
    {
        return trapHandler->canHandle(condition);
    }
    // no handler, return false
    return false;
}


/**
 * Process a NOVALUE event for a variable.
 *
 * @param name     The variable name triggering the event.
 * @param variable The resolved variable object for the variable.
 *
 * @return A value for that variable.
 */
RexxObject* RexxActivation::handleNovalueEvent(RexxString *name, RexxObject *defaultValue, RexxVariable *variable)
{
    // have we specified via ::options that errors should be raised?
    if (isNovalueSyntaxEnabled())
    {
        reportException(Error_Execution_unassigned_variable, name);
    }

    RexxObject *value = novalueHandler(name);
    // If the handler returns anything other than .nil, this is a
    // value
    if (value != TheNilObject)
    {
        return value;
    }
    // give any external novalue handler a chance at this
    if (!activity->callNovalueExit(this, name, value))
    {
        // set this variable to the object found in the engine
        variable->set(value);
        return value;
    }
    // raise novalue?
    if (novalueEnabled())
    {
        reportNovalue(name);
    }

    // the provided default value is the returned value
    return defaultValue;
}


/**
 * Merge a list of trapped conditions from an interpret into the
 * parent activation's queues.
 *
 * @param sourceConditionQueue
 *               The interpret activations condition queue.
 */
void RexxActivation::mergeTraps(QueueClass *sourceConditionQueue)
{
    if (sourceConditionQueue != OREF_NULL)
    {
        // if we don't have a condition queue at this level yet, then
        // just inherit the interpret one
        if (conditionQueue == OREF_NULL)
        {
            conditionQueue = sourceConditionQueue;
        }
        else
        {
            // copy all of the items over.
            while (!sourceConditionQueue->isEmpty())
            {
                conditionQueue->append(sourceConditionQueue->pull());
            }
        }
    }
}


/**
 * Unwind a chain of interpret activations to process a SIGNAL ON
 * or PROPAGATE condition trap.  This ensures that the SIGNAL
 * or PROPAGATE returns to the correct condition level
 *
 * @param child  The child activaty.
 */
void RexxActivation::unwindTrap(RexxActivation *child)
{
    // still an interpret level?  Merge, and try again
    if (isInterpret())
    {
        parent->mergeTraps(conditionQueue);
        parent->unwindTrap(child);
    }
    // reached the base non-interpret level
    else
    {
        // pull back the settings from the child
        child->putSettings(settings);
        // unwind and process the trap
        throw this;
    }
}


/**
 * Retrieve the activation that activated this activation (whew)
 * with the intent of doing condition trapping.
 *
 * @return The parent activation.
 */
ActivationBase* RexxActivation::senderActivation(RexxString *conditionName)
{
    // get the sender from the activity
    ActivationBase *_sender = getPreviousStackFrame();
    // spin down to non-native activation
    while (_sender != OREF_NULL && isOfClass(NativeActivation, _sender))
    {
        // if this is a native activation that is actively trapping conditions,
        // then use that as the condition trap target
        if (_sender->willTrap(conditionName))
        {
            return _sender;
        }
        _sender = _sender->getPreviousStackFrame();
    }
    // that is our sender
    return _sender;
}


/**
 * Translate and interpret a string of data as a piece
 * of Rexx code within the current program context.
 *
 * @param codestring The source code string.
 */
void RexxActivation::interpret(RexxString *codestring)
{
    // check the stack space to see if we have room.
    ActivityManager::currentActivity->checkStackSpace();
    // translate the code as if it was located here.
    RexxCode *newCode = code->interpret(codestring, current->getLineNumber());
    // create a new activation to run this code
    RexxActivation *newActivation = ActivityManager::newActivation(activity, this, newCode, INTERPRET);
    activity->pushStackFrame(newActivation);
    ProtectedObject r;
    // run this compiled code on the new activation
    newActivation->run(OREF_NULL, OREF_NULL, argList, argCount, OREF_NULL, r);
}


/**
 * Interpret a string of debug input.
 *
 * @param codestring The code string to interpret
 */
void RexxActivation::debugInterpret(RexxString *codestring)
{
    // mark that this is debug mode
    debugPause = true;
    try
    {
        // translate the code
        RexxCode *newCode = code->interpret(codestring, current->getLineNumber());
        // get a new activation to execute this
        RexxActivation *newActivation = ActivityManager::newActivation(activity, this, newCode, DEBUGPAUSE);
        activity->pushStackFrame(newActivation);
        ProtectedObject r;
        // go run the code
        newActivation->run(receiver, settings.messageName, argList, argCount, OREF_NULL, r);
        // turn this off when done executing
        debugPause = false;
    }
    catch (RexxActivation *t)
    {
        // turn this off unconditionally for any errors
        // if we're not the target of this throw, we've already been unwound
        // keep throwing this until it reaches the target activation.
        if (t != this)
        {
            throw;
        }

        // if we're not the current kernel holder when things return, make sure we
        // get the lock before we continue
        if (ActivityManager::currentActivity != activity)
        {
            activity->requestAccess();
        }
    }
}


/**
 * Return a Rexx-defined "dot" variable na.e
 *
 * @param name   The target variable name.
 *
 * @return The variable value or OREF_NULL if this is not
 *         one of the special variables.
 */
RexxObject * RexxActivation::rexxVariable(RexxString * name )
{
    // .RS happens in our context, so process here.
    if (name->strCompare("RS"))
    {
        // if we've set the return status, return that value, otherwise
        // just return as the string name.
        if (settings.isReturnStatusSet())
        {
            return new_integer(settings.returnStatus);
        }
        else
        {
            return name->concatToCstring(".");
        }
    }
    // all other should be handled by the parent context
    if (isInterpret())
    {
        return parent->rexxVariable(name);
    }

    // .METHODS
    if (name->strCompare("METHODS"))
    {
        return settings.parentCode->getMethods();
    }

    // .RESOURCES
    if (name->strCompare("RESOURCES"))
    {
        return settings.parentCode->getResources();
    }

    // .ROUTINES
    else if (name->strCompare("ROUTINES"))
    {
        return settings.parentCode->getRoutines();
    }
    // .LINE
    else if (name->strCompare("LINE"))
    {
        return new_integer(current->getLineNumber());
    }
    // .CONTEXT
    else if (name->strCompare("CONTEXT"))
    {
        // retrieve the context object (potentially creating it on the first request)
        return getContextObject();
    }

    // not one we know about
    return OREF_NULL;
}


/**
 * Get the context object for this activation.
 *
 * @return The created context object.
 */
RexxObject *RexxActivation::getContextObject()
{
    // the context object is created on demand...much of the time, this
    // is not needed for an actvation
    if (contextObject == OREF_NULL)
    {
        contextObject = new RexxContext(this);
    }
    return contextObject;
}


/**
 * Return the line context information for a context.
 *
 * @return The current execution line.
 */
RexxObject *RexxActivation::getContextLine()
{
    // if this is an interpret, we need to report the line number of
    // the context that calls the interpret.
    if (isInterpret())
    {
        return parent->getContextLine();
    }
    else
    {

        return new_integer(current->getLineNumber());
    }
}


/**
 * Return the line context information for a context.
 *
 * @return The current execution line.
 */
size_t RexxActivation::getContextLineNumber()
{
    // if this is an interpret, we need to report the line number of
    // the context that calls the interpret.
    if (isInterpret())
    {
        return parent->getContextLineNumber();
    }
    else
    {
        return current->getLineNumber();
    }
}


/**
 * Return the RS context information for a activation.
 *
 * @return The current execution line.
 */
RexxObject *RexxActivation::getContextReturnStatus()
{
    if (settings.isReturnStatusSet())
    {
        return new_integer(settings.returnStatus);
    }
    else
    {
        return TheNilObject;
    }
}


/**
 * Attempt to call a function stored in the macrospace.
 *
 * @param target    The target function name.
 * @param arguments The argument pointer.
 * @param argcount  The count of arguments,
 * @param calltype  The type of call (FUNCTION or SUBROUTINE)
 * @param order     The macrospace order flag.
 * @param result    The function result.
 *
 * @return true if the macrospace function was located and called.
 */
bool RexxActivation::callMacroSpaceFunction(RexxString *target, RexxObject **arguments,
    size_t argcount, RexxString *calltype, int order, ProtectedObject &_result)
{
    // the located macro search position
    unsigned short position;
    const char *macroName = target->getStringData();
    // did we find the one we want at the right time?
    if (RexxQueryMacro(macroName, &position) == 0)
    {
        // this was not found if the search order was different
        if (order != position)
        {
            return false;
        }
        // unflatten the code now
        Protected<RoutineClass> routine = getMacroCode(target);

        // not restoreable is a call failure
        if (routine == OREF_NULL)
        {
            return false;
        }
        // run as a call
        routine->call(activity, target, arguments, argcount, calltype, OREF_NULL, EXTERNALCALL, _result);
        // merge (class) definitions from macro with current settings
        getPackageObject()->mergeRequired(routine->getPackageObject());
        // we handled this
        return true;
    }
    return false;
}


/**
 * Call an already resolved external routine. This occurs if a
 * first call has suceeded using a package level resource. This
 * short cuts the full call chain
 *
 * @param target    The target function name.
 * @param routine   The resolved routine object
 * @param _argcount The count of arguments for the call.
 * @param _stack    The expression stack holding the arguments.
 * @param calltype  The type of call (FUNCTION or SUBROUTINE)
 * @param resultObj The returned result.
 *
 * @return The function result (also returned in the resultObj protected
 *         object reference.
 */
RexxObject *RexxActivation::externalCall(RexxString *target, RoutineClass *routine, RexxObject **arguments, size_t argcount,
    RexxString *calltype, ProtectedObject &resultObj)
{
    // call and return the result
    routine->call(activity, target, arguments, argcount, calltype, OREF_NULL, EXTERNALCALL, resultObj);
    return resultObj;
}


/**
 * Main method for performing an external routine call.  This
 * orchestrates the search order for locating an external routine.
 *
 * @param target    The target function name.
 * @param resolvedTarget
 *                  The resolved routine object for the call (if any), which
 *                  is passed back for caching.
 * @param arguments The pointer to the call arguments.
 * @param argcount  The count of arguments.
 * @param calltype  The type of call (FUNCTION or SUBROUTINE)
 * @param resultObj The returned result.
 *
 * @return The function result (also returned in the resultObj protected
 *         object reference.
 */
RexxObject *RexxActivation::externalCall(RoutineClass *&routine, RexxString *target, RexxObject **arguments, size_t argcount,
    RexxString *calltype, ProtectedObject &resultObj)
{
    // Step 1: used to be the functions directory, which has been deprecated.

    // Step 2:  Check for a ::ROUTINE definition in the local context
    // If found, this also passes the result back for caching
    routine = settings.parentCode->findRoutine(target);
    if (routine != OREF_NULL)
    {
        // call and return the result
        routine->call(activity, target, arguments, argcount, calltype, OREF_NULL, EXTERNALCALL, resultObj);
        return resultObj;
    }

    // Step 2a:  See if the function call exit fields this one
    if (!activity->callObjectFunctionExit(this, target, calltype == GlobalNames::FUNCTION, resultObj, arguments, argcount))
    {
        return resultObj;
    }

    // Step 2b:  See if the function call exit fields this one
    if (!activity->callFunctionExit(this, target, calltype == GlobalNames::FUNCTION, resultObj, arguments, argcount))
    {
        return resultObj;
    }

    // Step 3:  Perform all platform-specific searches
    if (SystemInterpreter::invokeExternalFunction(this, activity, target, arguments, argcount, calltype, resultObj))
    {
        return resultObj;
    }

    // Step 4:  Check scripting exit, which is after most of the checks
    if (!activity->callScriptingExit(this, target, calltype == GlobalNames::FUNCTION, resultObj, arguments, argcount))
    {
        return resultObj;
    }

    // if it's made it through all of these steps without finding anything, we
    // finally have a routine non found situation
    reportException(Error_Routine_not_found_name, target);
    return OREF_NULL;     // prevent compile error
}


/**
 * Call an external program as a function or subroutine.
 *
 * @param target     The target function name.
 * @param parent     The name of the parent program (used for resolving extensions).
 * @param _arguments The arguments to the call.
 * @param _argcount  The count of arguments for the call.
 * @param calltype   The type of call (FUNCTION or SUBROUTINE)
 * @param resultObj  The returned result.
 *
 * @return True if an external program was located and called.  false for
 *         any failures.
 */
bool RexxActivation::callExternalRexx(RexxString *target, RexxObject **arguments,
    size_t argcount, RexxString *calltype, ProtectedObject  &resultObj)
{
    // the interpreted package is created in memory and doesn't have the original program
    // name source information.  Forward this to the parent for processing if it is an
    // interpreted external call
    if (isInterpret())
    {
        return parent->callExternalRexx(target, arguments, argcount, calltype, resultObj);
    }

    // Get full name including path
    Protected<RexxString> filename = resolveProgramName(target, RESOLVE_DEFAULT);
    if (!filename.isNull())
    {
        // try for a saved program or translate a anew

        Protected<RoutineClass> routine = LanguageParser::createProgramFromFile(filename);
        // do we have something?  return not found
        if (routine.isNull())
        {
            return false;
        }
        else
        {
            // run as a call
            routine->call(activity, target, arguments, argcount, calltype, settings.currentAddress, EXTERNALCALL, resultObj);
            // merge all of the public info
            settings.parentCode->mergeRequired(routine->getPackageObject());
            return true;
        }
    }
    // the external routine wasn't found
    else
    {
        return false;
    }
}


/**
 * Retrieve a macro image file from the macro space.
 *
 * @param macroName The name of the macro to retrieve.
 *
 * @return If available, the unflattened method image.
 */
RoutineClass *RexxActivation::getMacroCode(RexxString *macroName)
{
    RXSTRING       macroImage;
    RoutineClass * macroRoutine = OREF_NULL;

    macroImage.strptr = NULL;
    const char *name = macroName->getStringData();
    int rc;
    {
        UnsafeBlock releaser;

        rc = RexxResolveMacroFunction(name, &macroImage);
    }

    if (rc == 0)
    {
        macroRoutine = RoutineClass::restore(&macroImage, macroName);
        // return the allocated buffer
        if (macroImage.strptr != NULL)
        {
            SystemInterpreter::releaseResultMemory(macroImage.strptr);
        }
    }
    return macroRoutine;
}


/**
 * This is resolved in the context of the calling program.
 *
 * @param name   The name to resolve.
 * @param type   The resolve type, RESOLVE_DEFAULT or RESOLVE_REQUIRES.
 *
 * @return The fully resolved program name, or OREF_NULL if this can't be
 *         located.
 */
RexxString *RexxActivation::resolveProgramName(RexxString *name, ResolveType type)
{
    return code->resolveProgramName(activity, name, type);
}


/**
 * Resolve a class in this activation's context.
 *
 * @param name   The name to resolve.
 *
 * @return The resolved class, or OREF_NULL if not found.
 */
RexxClass *RexxActivation::findClass(RexxString *name)
{
    RexxObject *t = OREF_NULL;   // required for the findClass call

    RexxClass *classObject = getPackageObject()->findClass(name, t);
    // we need to filter this to always return a class object
    if (classObject != OREF_NULL && classObject->isInstanceOf(TheClassClass))
    {
        return classObject;
    }
    return OREF_NULL;
}


/**
 * Resolve a class in this activation's context.
 *
 * @param name   The name to resolve.
 *
 * @return The resolved class, or OREF_NULL if not found.
 */
RexxObject *RexxActivation::resolveDotVariable(RexxString *name, RexxObject *&cachedValue)
{
    // if not an interpret, then resolve directly.
    if (!isInterpret())
    {
        return getPackageObject()->findClass(name, cachedValue);
    }
    else
    {
        // otherwise, send this up the call chain and resolve in the
        // original source context
        return parent->resolveDotVariable(name, cachedValue);
    }
}


/**
 * Does not actually load a package for a requires, but sets the
 * requires instruction as the current instruction for error
 * reporting.
 *
 * @param instruction
 *               The directive instruction being processed.
 */
void RexxActivation::loadRequires(RequiresDirective *instruction)
{
    // this will cause the correct location to be used for error reporting
    current = instruction;
}


/**
 * Load a package defined by a ::REQUIRES name LIBRARY
 * directive.
 *
 * @param target  The name of the package.
 * @param instruction
 *                The ::REQUIRES directive being loaded.
 * @param package The package that is loading this library
 */
void RexxActivation::loadLibrary(RexxString *target, RexxInstruction *instruction, PackageClass *package)
{
    // this will cause the correct location to be used for error reporting
    current = instruction;
    // have the package manager resolve the package. We then merge the
    // routines into the package imported routines list
    LibraryPackage *library = PackageManager::getLibrary(target);
    // and merge this into the package name space
    package->mergeLibrary(library);
}


/**
 * Process an internal function or subroutine call.
 *
 * @param name      The name of the target label.
 * @param target    The target instruction where we start executing (this is the label)
 * @param _argcount The count of arguments
 * @param _stack    The context stack holding the arguments
 * @param returnObject
 *                  A holder for the return value
 *
 * @return The return value object
 */
RexxObject * RexxActivation::internalCall(RexxString *name, RexxInstruction *target,
    RexxObject **arguments, size_t argcount, ProtectedObject &returnObject)
{
    // we need to set SIGL to the caller's line number
    size_t lineNum = current->getLineNumber();
    setLocalVariable(GlobalNames::SIGL, VARIABLE_SIGL, new_integer(lineNum));
    // create a new activation for running this and add to the activity stack
    RexxActivation *newActivation = ActivityManager::newActivation(activity, this, settings.parentCode, INTERNALCALL);
    activity->pushStackFrame(newActivation);

    // and go run this
    return newActivation->run(receiver, name, arguments, argcount, target, returnObject);
}


/**
 * Processing a call to an internal trap subroutine.
 *
 * @param name      The label name of the internal call.
 * @param target    The target instruction for the call (the label)
 * @param conditionObj
 *                  The associated condition object
 * @param resultObj A holder for a result object
 *
 * @return Any return result
 */
RexxObject * RexxActivation::internalCallTrap(RexxString *name, RexxInstruction *target,
    DirectoryClass *conditionObj, ProtectedObject &resultObj)
{
    // protect the condition object from GC by pushing on to the expression stack.
    stack.push(conditionObj);
    // we need to set the SIGL variable for an internal call
    size_t lineNum = current->getLineNumber();
    setLocalVariable(GlobalNames::SIGL, VARIABLE_SIGL, new_integer(lineNum));

    // create a new activation to handle this
    RexxActivation *newActivation = ActivityManager::newActivation(activity, this, settings.parentCode, INTERNALCALL);
    // set the condition object into the context that will handle the call
    newActivation->setConditionObj(conditionObj);
    activity->pushStackFrame(newActivation);
    // and go run this.
    return newActivation->run(OREF_NULL, name, (RexxObject **)&conditionObj, 1, target, resultObj);
}


/**
 * Wait for a variable in a guard expression to get updated.
 */
void RexxActivation::guardWait()
{
    // we need to wait without locking the variables.  If we
    // held the lock before the wait, we reaquire it after we wake up.
    GuardStatus initial_state = objectScope;

    if (objectScope == SCOPE_RESERVED)
    {
        settings.objectVariables->release(activity);
        objectScope = SCOPE_RELEASED;
    }
    // wait to be woken up by an update
    activity->guardWait();
    // if we released the scope before waiting, then we need to get it
    // back before proceeding.
    if (initial_state == SCOPE_RESERVED)
    {
        settings.objectVariables->reserve(activity);
        objectScope = SCOPE_RESERVED;
    }
}


/**
 * Get a traceback line for the current instruction.
 *
 * @return The formatted string traceback.
 */
RexxString *RexxActivation::getTraceBack()
{
    return formatTrace(current, getPackageObject());
}


/**
 * Retrieve the current activation timestamp, retrieving a new
 * timestamp if this is the first call for a clause
 *
 * @return A RexxDateTime object with the full time stamp.
 */
RexxDateTime RexxActivation::getTime()
{
    // not a valid time stamp?
    if (!settings.timeStamp.valid)
    {
        // IMPORTANT:  If a time call resets the elapsed time clock, we don't
        // clear the value out.  The time needs to stay valid until the clause is
        // complete.  The time stamp value that needs to be used for the next
        // elapsed time call is the timstamp that was valid at the point of the
        // last call, which is our current old invalid one.  So, we need to grab
        // that value and set the elapsed time start point, then clear the flag
        // so that it will remain current.
        if (isElapsedTimerReset())
        {
            settings.elapsedTime = settings.timeStamp.getUTCBaseTime();
            setElapsedTimerValid();
        }
        // get a fresh time stamp
        SystemInterpreter::getCurrentTime(&settings.timeStamp);
        // got a new one
        settings.timeStamp.valid = true;
    }
    // return the current time
    return settings.timeStamp;
}


/**
 * Retrieve the current elapsed time counter start time, starting
 * the counter from the current time stamp if this is the first
 * call
 *
 * @return The elapsed time counter.
 */
int64_t RexxActivation::getElapsed()
{
    // no active elapsed time clock yet?
    if (settings.elapsedTime == 0)
    {
        settings.elapsedTime = settings.timeStamp.getUTCBaseTime();
    }
    return settings.elapsedTime;
}


/**
 * Reset the elapsed time counter for the activation.
 */
void RexxActivation::resetElapsed()
{
    // Just invalidate so that we'll refresh this the next time we
    // obtain a new timestamp value.
    setElapsedTimerInvalid();
}


/**
 * Get the random seed for the random number generator.
 *
 * @param seed   The starter seed (optional)
 *
 * @return The new seed value to use for the random numbers.
 */
uint64_t RexxActivation::getRandomSeed(RexxInteger *seed)
{
    // if we're in an internal routine or interpret, we need to have the parent
    // context process this.
    if (isInternalLevelCall())
    {
        return parent->getRandomSeed(seed);
    }
    // do we have a seed supplied?  process that, but it must be a non-negative integer.
    if (seed != OREF_NULL)
    {
        wholenumber_t seed_value = seed->getValue();
        if (seed_value < 0)
        {
            reportException(Error_Incorrect_call_nonnegative, "RANDOM", IntegerThree, seed);
        }

        // set the seed value, then randomize it a little bit.
        randomSeed = seed_value;
        // flipping all of the bits gives us a better spread.  Supplied seeds tend to
        // be smaller numbers with lots of zero bits.
        randomSeed = ~randomSeed;
        // and scramble it a bit further.
        for (size_t i = 0; i < 13; i++)
        {
            randomSeed = RANDOMIZE(randomSeed);
        }
    }

    // randomize the seed for generating the next number
    randomSeed = RANDOMIZE(randomSeed);
    return randomSeed;
}


/**
 * Process the random function, using the current activation
 * seed value.
 *
 * @param randmin  The lower bounds of the range
 * @param randmax  The upper bounds of the range.
 * @param randseed The optional seed value.
 *
 * @return A new random number.
 */
RexxInteger * RexxActivation::random(RexxInteger *randmin, RexxInteger *randmax, RexxInteger *randseed)
{
    // get a new seed value
    uint64_t seed = getRandomSeed(randseed);

    wholenumber_t minimum = DefaultRandomMin;
    wholenumber_t maximum = DefaultRandomMax;

    // now process the min and max arguments
    if (randmin != OREF_NULL)
    {
        // no maximum value specified and no seed specified,
        // then the minimum is actually the max value value
        if ((randmax == OREF_NULL) && (randseed == OREF_NULL))
        {
            maximum = randmin->getValue();
        }
        // we have a min specified, no max, but do have a seed.  The minimum IS a
        // minimum in this situation.
        else if ((randmin != OREF_NULL) && (randmax == OREF_NULL) && (randseed != OREF_NULL))
        {
            minimum = randmin->getValue();
        }
        // we have both a min and a max.
        else
        {
            minimum = randmin->getValue();
            maximum = randmax->getValue();
        }
    }
    // no minimum, but we have a max?  That is a max.
    else if (randmax != OREF_NULL)
    {
        maximum = randmax->getValue();
    }

    if (maximum < minimum)
    {
        reportException(Error_Incorrect_call_random, randmin, randmax);
    }

    // we have a maximum range allowed
    if (maximum - minimum > MaxRandomRange)
    {
        reportException(Error_Incorrect_call_random_range, randmin, randmax);
    }

    // if there is a real range involved, we need to generate a new random value
    if (minimum != maximum)
    {
        // this will invert the bits of the value, start from zero and
        // add in each bit from the random seed
        uint64_t work = 0;
        for (size_t i = 0; i < sizeof(uint64_t) * 8; i++)
        {
            work <<= 1;                     // shift working num left one
                                            // add in next seed bit value
            work = work | (seed & 0x01LL);
            seed >>= 1;                     // shift off the right most seed bit
        }
        // we have a random value, now adjust the returned result for the spread of the range.
        minimum += (wholenumber_t)(work % (uint64_t)(maximum - minimum + 1));
    }
    return new_integer(minimum);
}


// table of trace prefixes.  Note that these need to match
// the TracePrefix enumeration tupe.
static const char * trace_prefix_table[] =
{
  "*-*",                               // TRACE_PREFIX_CLAUSE
  "+++",                               // TRACE_PREFIX_ERROR
  ">>>",                               // TRACE_PREFIX_RESULT
  ">.>",                               // TRACE_PREFIX_DUMMY
  ">V>",                               // TRACE_PREFIX_VARIABLE
  ">E>",                               // TRACE_PREFIX_DOTVARIABLE
  ">L>",                               // TRACE_PREFIX_LITERAL
  ">F>",                               // TRACE_PREFIX_FUNCTION
  ">P>",                               // TRACE_PREFIX_PREFIX
  ">O>",                               // TRACE_PREFIX_OPERATOR
  ">C>",                               // TRACE_PREFIX_COMPOUND
  ">M>",                               // TRACE_PREFIX_MESSAGE
  ">A>",                               // TRACE_PREFIX_ARGUMENT
  ">=>",                               // TRACE_PREFIX_ASSIGNMENT
  ">I>",                               // TRACE_PREFIX_INVOCATION
  ">N>",                               // TRACE_PREFIX_NAMESPACE
  ">K>",                               // TRACE_PREFIX_KEYWORD
  ">R>",                               // TRACE_PREFIX_ALIAS
};

// size of a line number
const size_t LINENUMBER = 6;
// location of the prefix field
const size_t PREFIX_OFFSET = (LINENUMBER + 1);
// length of the prefix field
const size_t PREFIX_LENGTH = 3;
// spaces per indentation amount
const size_t INDENT_SPACING = 2;
// over head for adding quotes
const size_t QUOTES_OVERHEAD = 2;

// extra space required to format a  result line.  This overhead is
// 6 leading spaces for the line  number, + 1 space + length of the
// message prefix (3) + 1 space +  2 for an indent + 2 for the
// quotes surrounding the value
const size_t TRACE_OVERHEAD = LINENUMBER + 1 + PREFIX_LENGTH + 1 + INDENT_SPACING + QUOTES_OVERHEAD;

// overhead for a traced instruction
// (6 digit line number, blank,
// 3 character prefix, and a blank
const size_t INSTRUCTION_OVERHEAD = LINENUMBER + 1 + PREFIX_LENGTH + 1;

// our maximum indentation amount for tracebacks
const size_t MAX_TRACEBACK_INDENT = 20;


// marker used for tagged traces to separate tag from the value
const char *RexxActivation::VALUE_MARKER = " => ";
// marker used for tagged traces to separate tag from the value
const char *RexxActivation::ASSIGNMENT_MARKER = " <= ";


/**
 * Trace program entry for a method or routine
 */
void RexxActivation::traceEntry()
{
    // since we're advertising the entry location up front, we want to disable
    // the normal trace-turn on notice.  We'll get one or the other, but not
    // both
    settings.setSourceTraced(true);

    ArrayClass *info = OREF_NULL;

    // the substitution information is different depending on whether this
    // is a method or a routine.
    if (isMethod())
    {
        info = new_array(getMessageName(), ((MethodClass *)executable)->getScopeName(), getPackage()->getProgramName());
    }
    else
    {
        info = new_array(getExecutable()->getName(), getPackage()->getProgramName());
    }
    ProtectedObject p(info);

    // and the message is slightly different
    RexxString *message = activity->buildMessage(isRoutine() ? Message_Translations_routine_invocation : Message_Translations_method_invocation, info);
    p = message;

    // we build this directly into a raw character string.
    size_t outlength = message->getLength() + INSTRUCTION_OVERHEAD;
    RexxString *buffer = raw_string(outlength);
    // insert the leading blanks for the prefix area
    buffer->set(0, ' ', INSTRUCTION_OVERHEAD);
    // copy in the prefix information
    buffer->put(PREFIX_OFFSET, trace_prefix_table[TRACE_PREFIX_INVOCATION], PREFIX_LENGTH);
    // copy the message stuff over this
    buffer->put(INSTRUCTION_OVERHEAD, message->getStringData(), message->getLength());
    // and write out the trace line
    activity->traceOutput(this, buffer);
}


/**
 * Trace an intermediate or result value.
 *
 * @param value  the value to trace.
 * @param prefix The prefix value for the item.
 */
void RexxActivation::traceValue(RexxObject *value, TracePrefix prefix)
{
    // nothing to trace here?  Just ignore
    if (noTracing(value))
    {
        return;
    }

    // get the string value for the traced item (this is the "safe" string value)
    RexxString *stringvalue = value->stringValue();
    // get a string large enough for the result
    size_t outlength = stringvalue->getLength() + TRACE_OVERHEAD + settings.traceIndent * INDENT_SPACING;
    RexxString *buffer = raw_string(outlength);
    ProtectedObject p(buffer);
    buffer->set(0, ' ', TRACE_OVERHEAD + settings.traceIndent * INDENT_SPACING);
    buffer->put(PREFIX_OFFSET, trace_prefix_table[prefix], PREFIX_LENGTH);
    buffer->putChar(TRACE_OVERHEAD - 2 + settings.traceIndent * INDENT_SPACING, '\"');
    buffer->put(TRACE_OVERHEAD - 1 + settings.traceIndent * INDENT_SPACING, stringvalue->getStringData(), stringvalue->getLength());
    buffer->putChar(outlength - 1, '\"');
    activity->traceOutput(this, buffer);
}


/**
 * Trace an entry that's of the form 'tag => "value"'.
 *
 * @param prefix    The trace prefix tag to use.
 * @param tagPrefix Any prefix string added to the tag.  Use mostly for adding
 *                  the "." to traced environment variables.
 * @param quoteTag  Indicates whether the tag should be quoted or not.  Operator
 *                  names are quoted.
 * @param tag       The tag name.
 * @param marker    An additional string marker for the value.
 * @param value     The associated trace value.
 */
void RexxActivation::traceTaggedValue(TracePrefix prefix, const char *tagPrefix, bool quoteTag,
     RexxString *tag, const char *marker, RexxObject *value)
{
    // the trace settings would normally require us to trace this, but there are conditions
    // where we just skip doing this anyway.
    if (noTracing(value))
    {
        return;
    }

    // get the string value from the traced object.
    RexxString *stringVal = value->stringValue();

    // now calculate the length of the traced string
    size_t outLength = tag->getLength() + stringVal->getLength();
    // these are fixed overheads
    outLength += TRACE_OVERHEAD + strlen(marker);
    // now the indent spacing
    outLength += settings.traceIndent * INDENT_SPACING;
    // now other conditionals
    outLength += quoteTag ? QUOTES_OVERHEAD : 0;
    // this is usually null, but dot variables add a "." to the tag.
    outLength += tagPrefix == NULL ? 0 : strlen(tagPrefix);

    // now get a buffer to write this out into
    RexxString *buffer = raw_string(outLength);
    ProtectedObject p(buffer);

    // get a cursor for filling in the formatted data
    size_t dataOffset = TRACE_OVERHEAD + settings.traceIndent * INDENT_SPACING - 2;
    buffer->set(0, ' ', TRACE_OVERHEAD + settings.traceIndent * INDENT_SPACING);
    buffer->put(PREFIX_OFFSET, trace_prefix_table[prefix], PREFIX_LENGTH);

    // if this is a quoted tag (operators do this), add quotes before coping the tag
    if (quoteTag)
    {
        buffer->putChar(dataOffset, '\"');
        dataOffset++;
    }
    // is the tag prefixed?  Add this before the name
    if (tagPrefix != NULL)
    {
        size_t prefixLength = strlen(tagPrefix);
        buffer->put(dataOffset, tagPrefix, prefixLength);
        dataOffset += prefixLength;
    }

    // add in the tag name
    buffer->put(dataOffset, tag);
    dataOffset += tag->getLength();

    // might need a closing quote.
    if (quoteTag)
    {
        buffer->putChar(dataOffset, '\"');
        dataOffset++;
    }

    // now add the data marker
    buffer->put(dataOffset, marker, strlen(marker));
    dataOffset += strlen(marker);

    // the leading quote around the value
    buffer->putChar(dataOffset, '\"');
    dataOffset++;

    // the traced value
    buffer->put(dataOffset, stringVal);
    dataOffset += stringVal->getLength();

    // and finally, the trailing quote
    buffer->putChar(dataOffset, '\"');
    dataOffset++;
                                       /* write out the line                */
    activity->traceOutput(this, buffer);
}


/**
 * Trace an entry that's of the form 'tag => "value"'.
 *
 * @param prefix    The trace prefix tag to use.
 * @param tagPrefix Any prefix string added to the tag.  Use mostly for adding
 *                  the "." to traced environment variables.
 * @param quoteTag  Indicates whether the tag should be quoted or not.  Operator
 *                  names are quoted.
 * @param tag       The tag name.
 * @param value     The associated trace value.
 */
void RexxActivation::traceOperatorValue(TracePrefix prefix, const char *tag, RexxObject *value)
{
    // the trace settings would normally require us to trace this, but there are conditions
    // where we just skip doing this anyway.
    if (noTracing(value))
    {
        return;
    }

    // get the string value from the traced object.
    RexxString *stringVal = value->stringValue();

    // now calculate the length of the traced string
    size_t outLength = strlen(tag) + stringVal->getLength();
    // these are fixed overheads
    outLength += TRACE_OVERHEAD + strlen(VALUE_MARKER);
    // now the indent spacing
    outLength += settings.traceIndent * INDENT_SPACING;
    // now other conditionals
    outLength += QUOTES_OVERHEAD;

    // now get a buffer to write this out into
    RexxString *buffer = raw_string(outLength);
    ProtectedObject p(buffer);

    // get a cursor for filling in the formatted data
    size_t dataOffset = TRACE_OVERHEAD + settings.traceIndent * INDENT_SPACING - 2;
    buffer->set(0, ' ', TRACE_OVERHEAD + settings.traceIndent * INDENT_SPACING);
    buffer->put(PREFIX_OFFSET, trace_prefix_table[prefix], PREFIX_LENGTH);

    // operators are quoted.
    buffer->putChar(dataOffset, '\"');
    dataOffset++;

    // add in the tag name
    buffer->put(dataOffset, tag, strlen(tag));
    dataOffset += strlen(tag);

    // need a closing quote.
    buffer->putChar(dataOffset, '\"');
    dataOffset++;

    // now add the data marker
    buffer->put(dataOffset, VALUE_MARKER, strlen(VALUE_MARKER));
    dataOffset += strlen(VALUE_MARKER);

    // the leading quote around the value
    buffer->putChar(dataOffset, '\"');
    dataOffset++;

    // the traced value
    buffer->put(dataOffset, stringVal);
    dataOffset += stringVal->getLength();

    // and finally, the trailing quote
    buffer->putChar(dataOffset, '\"');
    dataOffset++;
                                       /* write out the line                */
    activity->traceOutput(this, buffer);
}


/**
 * Trace a compound variable entry that's of the form 'tag =>
 * "value"'.
 *
 * @param prefix    The trace prefix tag to use.
 * @param stem      The stem name of the compound.
 * @param tails     The array of tail elements (unresolved).
 * @param tailCount The count of tail elements.
 * @param value     The resolved tail element
 */
void RexxActivation::traceCompoundValue(TracePrefix prefix, RexxString *stemName, RexxInternalObject **tails, size_t tailCount,
     CompoundVariableTail &tail)
{
    traceCompoundValue(prefix, stemName, tails, tailCount, VALUE_MARKER, tail.createCompoundName(stemName));
}


/**
 * Trace a compound variable entry that's of the form 'tag =>
 * "value"'.
 *
 * @param prefix    The trace prefix tag to use.
 * @param stem      The stem name of the compound.
 * @param tails     The array of tail elements (unresolved).
 * @param tailCount The count of tail elements.
 * @param value     The associated trace value.
 */
void RexxActivation::traceCompoundValue(TracePrefix prefix, RexxString *stemName, RexxInternalObject **tails, size_t tailCount, const char *marker,
     RexxObject *value)
{
    // the trace settings would normally require us to trace this, but there are conditions
    // where we just skip doing this anyway.
    if (noTracing(value))
    {
        return;
    }

    // get the string value from the traced object.
    RexxString *stringVal = value->stringValue();

    // now calculate the length of the traced string
    size_t outLength = stemName->getLength() + stringVal->getLength();

    // build an unresolved tail name
    CompoundVariableTail tail(tails, tailCount, false);

    outLength += tail.getLength();

    // add in the number of added dots
    outLength += tailCount - 1;

    // these are fixed overheads
    outLength += TRACE_OVERHEAD + strlen(marker);
    // now the indent spacing
    outLength += settings.traceIndent * INDENT_SPACING;

    // now get a buffer to write this out into
    RexxString *buffer = raw_string(outLength);
    ProtectedObject p(buffer);

    // get a cursor for filling in the formatted data
    size_t dataOffset = TRACE_OVERHEAD + settings.traceIndent * INDENT_SPACING - 2;
    buffer->set(0, ' ', TRACE_OVERHEAD + settings.traceIndent * INDENT_SPACING);
    buffer->put(PREFIX_OFFSET, trace_prefix_table[prefix], PREFIX_LENGTH);

    // add in the stem name
    buffer->put(dataOffset, stemName);
    dataOffset += stemName->getLength();

    // copy the tail portion of the compound name
    buffer->put(dataOffset, tail.getTail(), tail.getLength());
    dataOffset += tail.getLength();

    // now add the data marker
    buffer->put(dataOffset, marker, strlen(marker));
    dataOffset += strlen(marker);

    // the leading quote around the value
    buffer->putChar(dataOffset, '\"');
    dataOffset++;

    // the traced value
    buffer->put(dataOffset, stringVal);
    dataOffset += stringVal->getLength();

    // and finally, the trailing quote
    buffer->putChar(dataOffset, '\"');
    dataOffset++;
                                       /* write out the line                */
    activity->traceOutput(this, buffer);
}


/**
 * Trace the source string at debug mode start
 */
void RexxActivation::traceSourceString()
{
    // only once per customer...maybe next time.
    if (settings.wasSourceTraced())
    {
        return;
    }

    // now tag this as having been done.
    settings.setSourceTraced(true);
    // get the source string for the file
    RexxString *string = sourceString();
    // and build the trace line into a raw string
    size_t outlength = string->getLength() + INSTRUCTION_OVERHEAD + 2;
    RexxString *buffer = raw_string(outlength);
    buffer->set(0, ' ', INSTRUCTION_OVERHEAD);
    buffer->put(PREFIX_OFFSET, trace_prefix_table[TRACE_PREFIX_ERROR], PREFIX_LENGTH);
    buffer->putChar(INSTRUCTION_OVERHEAD, '\"');
    buffer->put(INSTRUCTION_OVERHEAD + 1, string->getStringData(), string->getLength());
    buffer->putChar(outlength - 1, '\"');
    activity->traceOutput(this, buffer);
}


/**
 * Format a source line for traceback or tracing
 *
 * @param instruction
 *                The target instruction.
 * @param package The package this instruction belongs to.
 *
 * @return The formatted traceback line.
 */
RexxString *RexxActivation::formatTrace(RexxInstruction *instruction, PackageClass *package)
{
    // nothing to do if we don't have an instruction
    if (instruction == OREF_NULL)
    {
        return OREF_NULL;
    }
    // get the instruction location
    SourceLocation location = instruction->getLocation();
    return package->traceBack(this, location, std::min(settings.traceIndent, MAX_TRACEBACK_INDENT), true);
}


/**
 * Handle all clause boundary processing (raising of halt
 * conditions, turning on of external traces, and calling of halt
 * and trace clause boundary exits
 */
void RexxActivation::processClauseBoundary()
{
    // do we have any pending CALL ON conditions?  Dispatch those
    // now.
    if (conditionQueue != OREF_NULL && !conditionQueue->isEmpty())
    {
        processTraps();
    }

    // test any halt exit wants to raise a halt condition.
    activity->callHaltTestExit(this);
    // check for external traces
    if (!activity->callTraceTestExit(this, settings.haveExternalTraceOn()))
    {
        // remember how this flipped
        if (settings.haveExternalTraceOn())
        {
            enableExternalTrace();
        }
        else
        {
            disableExternalTrace();
        }
    }

    // have a halt condition?
    if (settings.haveHaltCondition())
    {
        // flip this off and raise the condition
        // if not handled as a condition, turn into a syntax error
        settings.setHaltCondition(false);
        if (!activity->raiseCondition(GlobalNames::HALT, OREF_NULL, settings.haltDescription, OREF_NULL, OREF_NULL))
        {
            reportException(Error_Program_interrupted_condition, GlobalNames::HALT);
        }
    }

    // been asked to turn on tracing?
    if (settings.haveExternalTraceOn())
    {
        settings.setExternalTraceOn(false);
        enableExternalTrace();
    }

    // maybe turning tracing off?
    if (settings.haveExternalTraceOff())
    {
        settings.setExternalTraceOff(false);
        disableExternalTrace();
    }

    // now set the boundary flag based on whether we still have pending stuff for
    // next go around.
    clauseBoundary = settings.haveClauseExits() || !(conditionQueue == OREF_NULL || conditionQueue->isEmpty());
}


/**
 * Turn on external trace at program startup (e.g, because
 * RXTRACE is set)
 */
void RexxActivation::enableExternalTrace()
{
    TraceSetting setting;
    setting.setExternalTrace();

    setTrace(setting);
}


/**
 * Turn on external trace at program startup (e.g, because
 * RXTRACE is set)
 */
void RexxActivation::disableExternalTrace()
{
    TraceSetting setting;
    setting.setTraceOff();

    setTrace(setting);
}


/**
 * Halt the activation
 *
 * @param description
 *               The description for the halt condition (if any).
 *
 * @return true if this halt was recognized, false if there is a
 *         previous halt condition still to be processed.
 */
bool RexxActivation::halt(RexxString *description )
{
    // if there's no halt condition pending, set this
    if (!settings.haveHaltCondition())
    {
        // store the description
        settings.haltDescription = description;
        // turn on the halt flag and also the clause boundary
        // flag so that this gets processed.
        settings.setHaltCondition(true);
        clauseBoundary = true;
        return true;
    }
    else
    {
        // we're not in a good position to process this
        return false;
    }
}


/**
 * Flip ON the externally activated TRACE bit.
 */
void RexxActivation::yield()
{
    // max the instruction counter so that we will check immediately.
    instructionCount = yieldInstructions;
}


/**
 * Flip ON the externally activated TRACE bit.
 */
void RexxActivation::externalTraceOn()
{
    // turn on the tracing flags and have this checked at the next clause boundary
    settings.setExternalTraceOn(true);
    clauseBoundary = true;
}


/**
 * Flip on the externally activated TRACE OFF bit.
 */
void RexxActivation::externalTraceOff()
{
    // turn on the tracing flags and have this checked at the next clause boundary
    settings.setExternalTraceOff(true);
    clauseBoundary = true;
}


/**
 * Process an individual debug pause for an instruction
 *
 * @return true if the instruction should re-execute, false otherwise.
 */
bool RexxActivation::doDebugPause()
{
    // already in debug pause?  just skip pausing
    if (debugPause)
    {
        return false;
    }

    // asked to bypass...turn this off for the next time.
    if (settings.isDebugBypassed())
    {
        settings.setDebugBypass(false);
    }
    // debug pauses suppressed?  Reduce the count and turn debug pausing
    // back on for the next time.
    else if (settings.traceSkip > 0)
    {
        settings.traceSkip--;
        if (settings.traceSkip == 0)
        {
            // turn tracing back on again (this
            // ensures the next pause also has
            // the instruction traced
            settings.setTraceSuppressed(false);
        }
    }
    // normal pause
    else
    {
        // if we don't have real source code for this instruction, we can't pause.
        if (!code->isTraceable())
        {
            return false;
        }
        // first time paused?
        if (!settings.wasDebugPromptIssued())
        {
            // write the initial prompt and turn off for the next time.
            activity->traceOutput(this, Interpreter::getMessageText(Message_Translations_debug_prompt));
            settings.setDebugPromptIssued(true);
        }
        // save the next instruction in case we're asked to re-execute
        RexxInstruction *currentInst = next;
        for (;;)
        {
            RexxString *response = activity->traceInput(this);
            // a null line just advances
            if (response->getLength() == 0)
            {
                break;
            }
            // a re-execute request ("=")?
            // we reset the next instruction and return an indicator that
            // we need to re-execute.  Some instructions (e.g., block instructions)
            // need to undo some side effects of execution.
            else if (response->getLength() == 1 && response->getChar(0) == '=')
            {
                next = current;
                return true;
            }
            else
            {
                // interpret the instruction
                debugInterpret(response);
                // if we've had a flow of control change, we're done.
                if (currentInst != next)
                {
                    break;
                }
                // the trace setting may have changed on us.
                else if (settings.isDebugBypassed())
                {
                    // turn off the bypass setting.  Is for situations where a
                    // trace in normal code turns on debug.  The debug pause is
                    // skipped until the next instruction
                    settings.setDebugBypass(false);
                    break;
                }
            }
        }
    }
    // no re-execution needed
    return false;
}

/**
 * Trace an individual line of a source file
 *
 * @param clause the clause to trace.
 * @param prefix
 */
void RexxActivation::traceClause(RexxInstruction *clause, TracePrefix prefix)
{
    // we might be in a state where tracing is suppressed, or there's no source available.
    if (noTracing())
    {
        return;
    }
    // format the trace line
    RexxString *line = formatTrace(clause, code->getPackageObject());
    // do we have a real source line we can trace, go output it.
    if (line != OREF_NULL)
    {
        // if we've just dropped into debug mode, we need to put out the extra context line.
        if (inDebug() && !settings.wasSourceTraced())
        {
            traceSourceString();
        }
        activity->traceOutput(this, line);
    }
}


/**
 * resolve an IO configuration from an address name.
 *
 * @param address The address environment name
 * @param localConfig
 *                The configuration that may have been specified on an
 *                ADDRESS instruction.
 *
 * @return The IO context created by the potential merger of local
 *         and global IO configurations.
 */
CommandIOContext *RexxActivation::resolveAddressIOConfig(RexxString *address, CommandIOConfiguration *localConfig)
{
    // see if we have something globally set, merge with any
    // local settings that have been specified
    CommandIOConfiguration *globalConfig = getIOConfig(address);
    if (globalConfig != OREF_NULL)
    {
        return globalConfig->createIOContext(this, &stack, localConfig);
    }
    // no global, but might have a local one
    if (localConfig != OREF_NULL)
    {
        return localConfig->createIOContext(this, &stack, OREF_NULL);
    }
    // no configuration, nothing to do
    return OREF_NULL;
}


/**
 * Issue a command to a named host evironment
 *
 * @param address  The target address
 * @param commandString
 *                 The command to issue
 * @param ioConfig A potential I/O redirection setup for this command.
 */
void RexxActivation::command(RexxString *address, RexxString *commandString, CommandIOConfiguration *ioConfig)
{
    // if we are tracing command instructions, then we need to add some
    // additional trace information afterward.  Also, if we're tracing errors or
    // failures, we similarly need to know if the command has already been traced.
    bool instruction_traced = tracingAll() || tracingCommands();
    ProtectedObject condition;
    ProtectedObject commandResult;

    // we possibly have local or global IO configurations in place for
    Protected<CommandIOContext> ioContext = resolveAddressIOConfig(address, ioConfig);


    // give the command exit first pass at this.
    if (activity->callCommandExit(this, address, commandString, commandResult, condition))
    {
        // first check for registered command handlers.  If we have a handler, then
        // call it.  This includes the default system command handlers.
        CommandHandler *handler = activity->resolveCommandHandler(address);
        if (handler != OREF_NULL)
        {
            handler->call(activity, this, address, commandString, commandResult, condition, ioContext);
        }
        else
        {
            // No handler for this environment.  Give a default return code and
            // raise a failure condition.
            commandResult = new_integer(RXSUBCOM_NOTREG);   // just use the not registered return code
            condition = activity->createConditionObject(GlobalNames::FAILURE, commandResult, commandString, OREF_NULL, OREF_NULL);
        }
    }


    // now process the command result.
    RexxObject *rc = commandResult;
    DirectoryClass *conditionObj = (DirectoryClass *)(RexxObject *)condition;

    bool failureCondition = false;    // don't have a failure condition yet

    ReturnStatus returnStatus = RETURN_STATUS_NORMAL;
    // did a handler raise a condition?  We need to pull the rc value from the
    // condition object
    if (conditionObj != OREF_NULL)
    {
        RexxObject *temp = (RexxObject *)conditionObj->get(GlobalNames::RC);
        if (temp == OREF_NULL)
        {
            // see if we have a result and make sure the condition object
            // fills this as the RC value
            temp = (RexxObject *)conditionObj->get(GlobalNames::RESULT);
            if (temp != OREF_NULL)
            {
                conditionObj->put(temp, GlobalNames::RC);
            }
        }
        // replace the RC value
        if (temp != OREF_NULL)
        {
            rc = temp;
        }

        // now check for ERROR or FAILURE conditions
        RexxString *conditionName = (RexxString *)conditionObj->get(GlobalNames::CONDITION);
        // check for an error or failure condition, since these get special handling
        if (conditionName->strCompare("FAILURE"))
        {
            // unconditionally update the RC value
            conditionObj->put(temp, GlobalNames::RC);
            // failure conditions require special handling when raising the condition
            // we'll need to reraise this as an ERROR condition if not trapped.
            failureCondition = true;
            // set the appropriate return status
            returnStatus = RETURN_STATUS_FAILURE;
        }
        if (conditionName->strCompare("ERROR"))
        {
            // unconditionally update the RC value
            conditionObj->put(temp, GlobalNames::RC);
            // set the appropriate return status
            returnStatus = RETURN_STATUS_ERROR;
        }
    }

    // a handler might not return a value, so default the return code to zero
    // if nothing is received.
    if (rc == OREF_NULL)
    {
        rc = TheFalseObject;
    }

    // if this was done during a debug pause, we don't update RC
    // and .RS.
    if (!debugPause)
    {
        // set the RC value before anything
        setLocalVariable(GlobalNames::RC, VARIABLE_RC, rc);
        // tracing command errors or fails?
        if ((returnStatus == RETURN_STATUS_ERROR && tracingErrors()) ||
            (returnStatus == RETURN_STATUS_FAILURE && (tracingFailures())))
        {
            // trace the current instruction
            traceClause(current, TRACE_PREFIX_CLAUSE);
            // then we always trace full command
            traceValue(commandString, TRACE_PREFIX_RESULT);
            // this has been traced
            instruction_traced = true;
        }

        wholenumber_t rcValue;
        // need to trace the RC info too?
        if (instruction_traced && rc->numberValue(rcValue) && rcValue != 0)
        {
            // this has a special RC(val) format
            RexxString *rc_trace = rc->stringValue();
            rc_trace = rc_trace->concatToCstring("RC(");
            rc_trace = rc_trace->concatWithCstring(")");
            traceValue(rc_trace, TRACE_PREFIX_ERROR);
        }
        // set the return status
        setReturnStatus(returnStatus);

        // now handle any conditions we might need to raise
        // these are also not raised if it's a debug pause.
        if (conditionObj != OREF_NULL)
        {
            // first check for an ::OPTIONS FAILURE SYNTAX override
            if (failureCondition && isFailureSyntaxEnabled())
            {
                // we could rework our condition object, but it's easier
                // to just raise a SYNTAX exception
                reportException(Error_Execution_failure_syntax,
                   (RexxString *)conditionObj->get(GlobalNames::DESCRIPTION),
                   (RexxString *)conditionObj->get(GlobalNames::RC));
            }
            // next we check for an ::OPTIONS ERROR SYNTAX override
            if (!failureCondition && isErrorSyntaxEnabled())
            {
                // just raise a SYNTAX exception
                reportException(Error_Execution_error_syntax,
                   (RexxString *)conditionObj->get(GlobalNames::DESCRIPTION),
                   (RexxString *)conditionObj->get(GlobalNames::RC));
            }

            // try to raise the condition, and if it isn't handled, we might
            // munge this into an ERROR condition
            if (!activity->raiseCondition(conditionObj))
            {
                // untrapped failure condition?  Turn into an ERROR condition and
                // reraise
                if (failureCondition)
                {
                    // again, first check for an ::OPTIONS ERROR SYNTAX override
                    if (isErrorSyntaxEnabled())
                    {
                        // if so, just raise a SYNTAX exception
                        reportException(Error_Execution_error_syntax,
                           (RexxString *)conditionObj->get(GlobalNames::DESCRIPTION),
                           (RexxString *)conditionObj->get(GlobalNames::RC));
                    }

                    // just change the condition name
                    conditionObj->put(GlobalNames::ERRORNAME, GlobalNames::CONDITION);
                    activity->raiseCondition(conditionObj);
                }
            }
        }

        // do debug pause if necessary.  necessary is defined by:  we are
        // tracing ALL or COMMANDS, OR, we are using TRACE NORMAL and a FAILURE
        // return code was received OR we receive an ERROR return code and
        // have TRACE ERROR in effect.
        if (instruction_traced && inDebug())
        {
            doDebugPause();
        }
    }
}


/**
 * Set the return status flag for an activation context.
 *
 * @param status The new status value.
 */
void RexxActivation::setReturnStatus(ReturnStatus status)
{
    settings.returnStatus = status;
    settings.setReturnStatus(true);
}


/**
 * Return the name of the current program file
 *
 * @return The string name of the program source.
 */
RexxString *RexxActivation::getProgramName()
{
    return code->getProgramName();
}


/**
 * Handy method for displaying the current program name in the
 * debugger.
 *
 * @return The string name of the program source.
 */
const char *RexxActivation::displayProgramName()
{
    return code->getProgramName()->getStringData();
}


/**
 * Return the directory of labels for this block of code.
 *
 * @return The string table of labels (returns null if there are no labels in this code section)
 */
StringTable *RexxActivation::getLabels()
{
   return code->getLabels();
}


/**
 * Create the source string returned by parse source
 *
 * @return The constructed source string.
 */
RexxString *RexxActivation::sourceString()
{
    // if this is an interpret, have the parent context handle this.
    if (isInterpret())
    {
        return parent->sourceString();
    }

    // first token is a platform name
    const char *platform = SystemInterpreter::getPlatformName();

    RexxString *programName = code->getProgramName();

    // get a raw string we can build this into
    RexxString *sourceString = raw_string(strlen(platform) + settings.calltype->getLength() + programName->getLength() + 2);
    RexxString::StringBuilder builder(sourceString);

    builder.append(platform);
    builder.append(' ');
    builder.append(settings.calltype);
    builder.append(' ');
    builder.append(programName);

    return sourceString;

    // build this here.
}


/**
 * Retrieve the directory of public routines associated with the
 * current activation.
 *
 * @return A directory of the public routines.
 */
StringTable *RexxActivation::getPublicRoutines()
{
    return code->getPublicRoutines();
}


/**
 * Set an error notification tag on the activation.
 *
 * @param notify The notification object.
 */
void RexxActivation::setObjNotify(MessageClass *notify)
{
    notifyObject = notify;
}


/**
 * Push the new environment buffer onto the environment list.
 *
 * @param environment
 *               The new saved environment.
 */
void RexxActivation::pushEnvironment(RexxObject *environment)
{
    // only process if we're at the top level.
    if (isTopLevelCall())
    {
        // create an environment list if this is the first request
        if (!environmentList)
        {
            environmentList = new_queue();
        }
        // push this on the top of the stack.
        environmentList->push(environment);
    }
    // interprets and internal routines pass on to the parent activation
    else
    {
        parent->pushEnvironment(environment);
    }
}


/**
 * return the top level local Environemnt
 *
 * @return Pop the top local environment and restore it.
 */
RexxObject *RexxActivation::popEnvironment()
{
    // only process if the top level
    if (isTopLevelCall())
    {
        if (environmentList)
        {
            return (RexxObject *)environmentList->pull();

        }
        else
        {
            return TheNilObject;
        }
    }
    else
    {
        return parent->popEnvironment();
    }
}


/**
 * Close any streams opened by the I/O builtin functions
 */
void RexxActivation::closeStreams()
{
                                         /* exiting a bottom level?           */
    if (isProgramOrMethod())
    {
        StringTable *streams = settings.streams;  /* get the streams directory         */
        /* actually have a table?            */
        if (streams != OREF_NULL)
        {
            // send each of the streams in the table a CLOSE message.
            for (HashContents::TableIterator iterator = streams->iterator(); iterator.isAvailable(); iterator.next())
            {
                ProtectedObject result;
                ((RexxObject *)iterator.value())->sendMessage(GlobalNames::CLOSE, result);
            }
        }
    }
}


/**
 * Handle SAY output for a SAY instruction.
 *
 * @param line   The line to write out.
 */
void RexxActivation::sayOutput(RexxString *line)
{
    activity->sayOutput(this, line);
}


/**
 * Handle PULL or PARSE PULL input
 */
RexxString *RexxActivation::pullInput()
{
    return activity->pullInput(this);
}


/**
 * Handle PARSE LINEIN input
 */
RexxString *RexxActivation::lineIn()
{
    return activity->lineIn(this);
}


/**
 * Handle PUSH/QUEUE output
 */
void RexxActivation::queue(RexxString *line, Activity::QueueOrder order)
{
    activity->queue(this, line, order);
}


/**
 * process unitialized variable overrides
 *
 * @param name   The uninitialized variable name.
 *
 * @return The handler return result.
 */
RexxObject *RexxActivation::novalueHandler(RexxString *name)
{
    // the handler, if it exists, is stored in .local.
    RexxObject *novalue_handler = getLocalEnvironment(GlobalNames::NOVALUE);
    if (novalue_handler != OREF_NULL)
    {
        ProtectedObject result;
        return resultOrNil(novalue_handler->sendMessage(GlobalNames::NOVALUE, name, result));
    }
    return TheNilObject;
}


/**
 * Retrieve the package for the current execution context.
 *
 * @return The Package holding the code for the current execution
 *         context.
 */
PackageClass *RexxActivation::getPackage()
{
    return executable->getPackage();
}


/**
 * Evaluate a compound variable from the local context.
 *
 * @param stemName  The name of the stem.
 * @param index     The stem variable index (if known)
 * @param tail      The evaluated tail.
 * @param tailCount The count of tail elements.
 *
 * @return The value of the variable.
 */
RexxObject *RexxActivation::evaluateLocalCompoundVariable(RexxString *stemName, size_t index, RexxInternalObject **tail, size_t tailCount)
{
    CompoundVariableTail resolved_tail(this, tail, tailCount);

    // locate the stem variable and get the value from there.
    StemClass *stem_table = getLocalStem(stemName, index);
    RexxObject *value = stem_table->evaluateCompoundVariableValue(this, stemName, resolved_tail);
    if (tracingIntermediates())
    {
        traceCompoundName(stemName, tail, tailCount, resolved_tail);
        traceCompound(stemName, tail, tailCount, value);
    }
    return value;
}


/**
 * Get the value of a compound variable from the local context.
 *
 * @param stemName  The stem variable name.
 * @param index     The slot index of the stem variable (if known)
 * @param tail      The array of tail elements.
 * @param tailCount The count of tail elements.
 *
 * @return The variable value (or OREF_NULL if not found)
 */
RexxObject *RexxActivation::getLocalCompoundVariableValue(RexxString *stemName, size_t index, RexxInternalObject **tail, size_t tailCount)
{
    CompoundVariableTail resolved_tail(this, tail, tailCount);

    StemClass *stem_table = getLocalStem(stemName, index);   /* get the stem entry from this dictionary */
    return stem_table->getCompoundVariableValue(resolved_tail);
}


/**
 * Get the value of a compound variable from the local context.
 *
 * @param stemName  The stem variable name.
 * @param index     The slot index of the stem variable (if known)
 * @param tail      The array of tail elements.
 * @param tailCount The count of tail elements.
 *
 * @return The variable value (or OREF_NULL if not found)
 */
RexxObject *RexxActivation::getLocalCompoundVariableRealValue(RexxString *localstem, size_t index, RexxInternalObject **tail, size_t tailCount)
{
    CompoundVariableTail resolved_tail(this, tail, tailCount);

    StemClass *stem_table = getLocalStem(localstem, index);
    return stem_table->getCompoundVariableRealValue(resolved_tail);
}


/**
 * Get the value of a compound variable from the local context.
 *
 * @param stemName  The stem variable name.
 * @param index     The slot index of the stem variable (if known)
 * @param tail      The array of tail elements.
 * @param tailCount The count of tail elements.
 *
 * @return The variable value (or OREF_NULL if not found)
 */
CompoundTableElement *RexxActivation::getLocalCompoundVariable(RexxString *stemName, size_t index, RexxInternalObject **tail, size_t tailCount)
{
    CompoundVariableTail resolved_tail(this, tail, tailCount);

    StemClass *stem_table = getLocalStem(stemName, index);
    return stem_table->getCompoundVariable(resolved_tail);
}


/**
 * Expose a compound variable in the local context.
 *
 * @param stemName  The stem variable name.
 * @param index     The slot index of the stem variable (if known)
 * @param tail      The array of tail elements.
 * @param tailCount The count of tail elements.
 *
 * @return The variable value (or OREF_NULL if not found)
 */
CompoundTableElement *RexxActivation::exposeLocalCompoundVariable(RexxString *stemName, size_t index, RexxInternalObject **tail, size_t tailCount)
{
    CompoundVariableTail resolved_tail(this, tail, tailCount);

    StemClass *stem_table = getLocalStem(stemName, index);
    return stem_table->exposeCompoundVariable(resolved_tail);
}


/**
 * Test if a compound variable exists in the local context.
 *
 * @param stemName  The stem variable name.
 * @param index     The slot index of the stem variable (if known)
 * @param tail      The array of tail elements.
 * @param tailCount The count of tail elements.
 *
 * @return true if the variable exists, false if not.
 */
bool RexxActivation::localCompoundVariableExists(RexxString *stemName, size_t index, RexxInternalObject **tail, size_t tailCount)
{
    CompoundVariableTail resolved_tail(this, tail, tailCount);

    StemClass *stem_table = getLocalStem(stemName, index);
    return stem_table->compoundVariableExists(resolved_tail);
}


/**
 * Perform an assignment to a compound variable in the local context.
 *
 * @param stemName  The stem variable name.
 * @param index     The index of the stem variable (if known)
 * @param tail      The array of tail elements.
 * @param tailCount The count of tail elements.
 * @param value     The value to assign.
 */
void RexxActivation::assignLocalCompoundVariable(RexxString *stemName, size_t index, RexxInternalObject **tail, size_t tailCount, RexxObject *value)
{
    CompoundVariableTail resolved_tail(this, tail, tailCount);

    StemClass *stem_table = getLocalStem(stemName, index);
    stem_table->setCompoundVariable(resolved_tail, value);
    if (tracingIntermediates())
    {
        traceCompoundName(stemName, tail, tailCount, resolved_tail);
        traceCompoundAssignment(stemName, tail, tailCount, value);
    }
}


/**
 * set a compound variable in the local context.
 *
 * @param stemName  The stem variable name.
 * @param index     The index of the stem variable (if known)
 * @param tail      The array of tail elements.
 * @param tailCount The count of tail elements.
 * @param value     The value to assign.
 */
void RexxActivation::setLocalCompoundVariable(RexxString *stemName, size_t index, RexxInternalObject **tail, size_t tailCount, RexxObject *value)
{
    CompoundVariableTail resolved_tail(this, tail, tailCount);

    StemClass *stem_table = getLocalStem(stemName, index);
    stem_table->setCompoundVariable(resolved_tail, value);
}


/**
 * Drop a compound variable in the local context.
 *
 * @param stemName  The name of the stem variable.
 * @param index     The index (if known)
 * @param tail      The array of tail elements.
 * @param tailCount The count of tail elements.
 */
void RexxActivation::dropLocalCompoundVariable(RexxString *stemName, size_t index, RexxInternalObject **tail, size_t tailCount)
{
    CompoundVariableTail resolved_tail(this, tail, tailCount);
    StemClass *stem_table = getLocalStem(stemName, index);
    stem_table->dropCompoundVariable(resolved_tail);
}


/**
 * Get the security manager in effect for a given context.
 *
 * @return The security manager defined for this activation
 *         context.
 */
SecurityManager *RexxActivation::getSecurityManager()
{
    return settings.securityManager;
}


/**
 * Get the security manager in used by this activation.
 *
 * @return Either the defined security manager or the instance-global security
 *         manager.
 */
SecurityManager *RexxActivation::getEffectiveSecurityManager()
{
    SecurityManager *manager = settings.securityManager;
    if (manager != OREF_NULL)
    {
        return manager;
    }
    return activity->getInstanceSecurityManager();
}


/**
 * Retrieve a value from the instance local environment.
 *
 * @param name   The name of the .local object.
 *
 * @return The object stored at the given name.
 */
RexxObject *RexxActivation::getLocalEnvironment(RexxString *name)
{
    return activity->getLocalEnvironment(name);
}


/**
 * Create a stack frame for exception tracebacks.
 *
 * @return A StackFrame instance for this activation.
 */
StackFrameClass *RexxActivation::createStackFrame()
{
    const char *type = StackFrameClass::FRAME_METHOD;
    ArrayClass *arguments = OREF_NULL;
    RexxObject *target = OREF_NULL;

    if (isInterpret())
    {
        type = StackFrameClass::FRAME_INTERPRET;
    }
    else if (isInternalCall())
    {
        type = StackFrameClass::FRAME_INTERNAL_CALL;
        arguments = getArguments();
    }
    else if (isMethod())
    {
        type = StackFrameClass::FRAME_METHOD;
        arguments = getArguments();
        target = receiver;
    }
    else if (isProgram())
    {
        type = StackFrameClass::FRAME_PROGRAM;
        arguments = getArguments();
    }
    else if (isRoutine())
    {
        type = StackFrameClass::FRAME_ROUTINE;
        arguments = getArguments();
    }

    // construct the traceback line before we allocate the stack frame object.
    // calling this in the constructor argument list can cause the stack frame instance
    // to be inadvertently reclaimed if a GC is triggered while evaluating the constructor
    // arguments.
    RexxString *traceback = getTraceBack();
    return new StackFrameClass(type, getMessageName(), getExecutableObject(), target, arguments, traceback, getContextLineNumber());
}

/**
 * Format a more informative trace line when giving
 * traceback information for code when no source code is
 * available.
 *
 * @param packageName
 *               The package name to use (could be "REXX" for internal code)
 *
 * @return A formatted descriptive string for the invocation.
 */
RexxString *RexxActivation::formatSourcelessTraceLine(RexxString *packageName)
{
    // if this is a method invocation, then we can give the method name and scope.
    if (isMethod())
    {
        ArrayClass *info = new_array(getMessageName(), ((MethodClass *)executable)->getScopeName(), packageName);
        ProtectedObject p(info);

        return activity->buildMessage(Message_Translations_sourceless_method_invocation, info);
    }
    else if (isRoutine())
    {
        ArrayClass *info = new_array(getMessageName(), packageName);
        ProtectedObject p(info);

        return activity->buildMessage(Message_Translations_sourceless_routine_invocation, info);
    }
    else
    {
        ArrayClass *info = new_array(packageName);
        ProtectedObject p(info);

        return activity->buildMessage(Message_Translations_sourceless_program_invocation, info);
    }
}


/**
 * Generate the stack frames for the current context.
 *
 * @return A list of the stackframes.
 */
ArrayClass *RexxActivation::getStackFrames(bool skipFirst)
{
    return activity->generateStackFrames(skipFirst);
}


/**
 * Return the associated object variables fileNames table
 *
 * @return The table of opened streams file names.
 */
StringTable* RexxActivation::getFileNames()
{
    // first request for a file name?  We need to
    // create the table.
    if (settings.fileNames == OREF_NULL)
    {
        settings.fileNames = new_string_table();
    }
    return settings.fileNames;
}


/**
 * Remove a filename from the short name lookup table
 *
 * @param fullName   The fully qualified name
 */
void RexxActivation::removeFileName(RexxString *fullName)
{
    // remove from the direct stream table
    getStreams()->remove(fullName);
    // if we have other lookups possible, remove all of the shortcuts
    if (notCaseSensitive() && settings.fileNames != OREF_NULL)
    {
        RexxInternalObject *removed;
        do
        {
            removed = settings.fileNames->removeItem(fullName);
        }
        while (removed != OREF_NULL);
    }
}
