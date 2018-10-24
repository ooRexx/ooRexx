/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2018 Rexx Language Association. All rights reserved.    */
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
/* Primitive Message Class                                                    */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "DirectoryClass.hpp"
#include "ArrayClass.hpp"
#include "ListClass.hpp"
#include "MethodClass.hpp"
#include "Activity.hpp"
#include "RexxActivation.hpp"
#include "NativeActivation.hpp"
#include "MessageClass.hpp"
#include "NativeCode.hpp"
#include "ProtectedObject.hpp"
#include "MethodArguments.hpp"

// singleton class instance
RexxClass *MessageClass::classInstance = OREF_NULL;


/**
 * Create an instance of a message object.
 *
 * @param size   The base size of the object.
 *
 * @return The storage for the object in question.
 */
void *MessageClass::operator new(size_t size)
{
    return new_object(size, T_Message);
}


/**
 * Create initial class object at bootstrap time.
 */
void MessageClass::createInstance()
{
    CLASS_CREATE(Message);
}


// TODO:  There are no tests for the message class.

/**
 * Create a new message object.
 *
 * @param _target The receiver object.
 * @param messageName
 *                The invoked message name.
 * @param scope   The starting scope (can be OREF_NULL).
 * @param _args   An array of arguments to the message.
 */
MessageClass::MessageClass(RexxObject *_target, RexxString *msgName, RexxClass *scope, ArrayClass *_args)
{
    // default target is the specified target
    receiver = _target;
    // the target specified on the new
    target = _target;
    // Args to be sent with themessage (optional)
    args = _args;
    message = msgName;
    startscope = scope;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void MessageClass::live(size_t liveMark)
{
    memory_mark(receiver);
    memory_mark(target);
    memory_mark(message);
    memory_mark(startscope);
    memory_mark(args);
    memory_mark(resultObject);
    memory_mark(interestedParties);
    memory_mark(condition);
    memory_mark(startActivity);
    memory_mark(objectVariables);
    memory_mark(waitingActivities);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void MessageClass::liveGeneral(MarkReason reason)
{
    memory_mark_general(receiver);
    memory_mark_general(target);
    memory_mark_general(message);
    memory_mark_general(startscope);
    memory_mark_general(args);
    memory_mark_general(resultObject);
    memory_mark_general(interestedParties);
    memory_mark_general(condition);
    memory_mark_general(startActivity);
    memory_mark_general(objectVariables);
    memory_mark_general(waitingActivities);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void MessageClass::flatten(Envelope *envelope)
{
    setUpFlatten(MessageClass)

    flattenRef(receiver);
    flattenRef(target);
    flattenRef(message);
    flattenRef(startscope);
    flattenRef(args);
    flattenRef(resultObject);
    flattenRef(interestedParties);
    flattenRef(condition);
    flattenRef(startActivity);
    flattenRef(objectVariables);
    flattenRef(waitingActivities);

    cleanUpFlatten
}


/**
 * Copy a message object.
 *
 * @return A new copy of the object, with internal state set to a clean state.
 */
RexxInternalObject *MessageClass::copy()
{
    // do the base copy
    MessageClass *newMessage = (MessageClass *)RexxObject::copy();
    // reset the completion state
    newMessage->clearCompletion();
    // clear any multithreading state
    newMessage->clearStartPending();
    newMessage->startActivity = OREF_NULL;
    newMessage->waitingActivities = OREF_NULL;

    // we inherit the interested parties list, but it needs to be a copy
    if (interestedParties != OREF_NULL)
    {
        newMessage->interestedParties = (ArrayClass *)interestedParties->copy();
    }
    return newMessage;
}


/**
 * Add a message object to the completion notification list.
 *
 * @param _message The message object wishing to be notified.
 *
 * @return Instruction message type that returns nothing.
 */
RexxObject *MessageClass::notify(RexxObject *notificationTarget)
{
    RexxObject *t = OREF_NULL;   // required for the findClass call

    classArgument(notificationTarget, TheRexxPackage->findClass(GlobalNames::MessageNotification, t), "notification target");

    // get a new array if this is the first one added.
    if (interestedParties == OREF_NULL)
    {
        interestedParties = new_array();
    }
    // we always add this because we can reuse the message object.
    interestedParties->append(notificationTarget);

    // now if we've already notified everything, send the notification now.
    if (allNotified())
    {
        ProtectedObject result;
        // send the message now
        notificationTarget->sendMessage(GlobalNames::MessageComplete, this, result);
    }
    return OREF_NULL;
}


/**
 * Wait until message completion, either because the message has
 * completed execution or because it terminated with an error.
 *
 * @return Always return nothing.
 */
RexxObject *MessageClass::wait()
{
    // we've not completed execution yet, so wait
    if (!isComplete())
    {
        // we can wait, even if the message has not been triggered
        // yet.  The message object keeps the list of waiting activities
        // and will wake them up when it finally is triggered and runs.

        // make sure we're not about to create a deadlock situation.
        if (startActivity != OREF_NULL)
        {
            startActivity->checkDeadLock(ActivityManager::currentActivity);
        }

        // we might be the first one to wait, so create the activities
        // list if we are
        if (waitingActivities == OREF_NULL)
        {
            setField(waitingActivities, new_array());
        }
        // add our activity to the list
        waitingActivities->append(ActivityManager::currentActivity);
        // and wait for the wake up call. We are handling this like
        // it is a guard wait, which has its own semaphore rules.
        ActivityManager::currentActivity->waitReserve(this);
    }

    // always return no result value
    return OREF_NULL;
}


/**
 * Return the message result object.  This will wait if
 * the message has not completed.
 *
 * @return The result object (if any)
 */
RexxObject *MessageClass::result()
{
    // go wait, if necessary
    wait();

    // did running this message cause an error?  If so, we raise the same error
    // condition here.
    if (raiseError())
    {
        ActivityManager::currentActivity->reraiseException(condition);
    }
    // since this is requested via a method that will give an error if used
    // in an expression, return .nil if there is no return value.
    return resultOrNil(resultObject);
}


/**
 * Do a dynamic invocation of an object method.
 *
 * @param arguments The variable arguments passed to the method.  The first
 *                  argument is a required message target, which can be either
 *                  a string method name or an array containing a name/scope
 *                  pair.  The remainder of the arguments are the message
 *                  arguments.
 * @param argCount
 *
 * @return The method result.
 */
RexxObject *MessageClass::sendRexx(RexxObject **arguments, size_t argCount)
{
    // we must have a message name argument
    if (argCount != 0)
    {
        // if we've been given a new receiver, then use that
        if (arguments[0] != OREF_NULL)
        {
            setField(receiver, arguments[0]);
        }
    }

    // given arguments with the message?  This replaces any arguments
    // we were created with.
    if (argCount > 1)
    {
        setField(args, new_array(argCount - 1, arguments + 1));
    }

    // now go perform the send
    return send();
}


/**
 * Do a dynamic invocation of an object method.
 *
 * @param receiver  An optional receiver target
 * @param arguments An array of arguments to used with the message invocation.
 *
 * @return The method result.
 */
RexxObject *MessageClass::sendWithRexx(RexxObject *newReceiver, ArrayClass *arguments)
{
    // if we've been given a new receiver, then use that
    if (newReceiver != OREF_NULL)
    {
        setField(receiver, newReceiver);
    }

    // with sendWith, the arguments are required
    arguments = arrayArgument(arguments, "message arguments");
    setField(args, arguments);

    // now go perform the send
    return send();
}


/**
 * Verify we're not reusing a message object incorrectly.
 */
void MessageClass::checkReuse()
{
    // Once start has been called, this can no longer be reused.
    if (isActivated())
    {
        reportException(Error_Execution_message_reuse);
    }

    // we need the current activity to handle this
    Activity *myActivity = ActivityManager::currentActivity;
    // if we are waiting for things to start on another activity, this
    // is also a reuse.
    if (startPending() && myActivity != startActivity )
    {
        reportException(Error_Execution_message_reuse);
    }
}


/**
 * Clear all of the state related to message completion.
 */
void MessageClass::clearCompletion()
{
    dataFlags.reset(flagResultReturned);
    dataFlags.reset(flagRaiseError);
    dataFlags.reset(flagErrorReported);
    dataFlags.reset(flagAllNotified);
    // clear execution-related fields
    clearField(resultObject);
    clearField(condition);
}


/**
 * Send a message to the target receiver object (optional).
 *
 * @param _receiver The optional receiver object.
 *
 * @return Returns the message result.
 */
RexxObject *MessageClass::send()
{
    // make sure we're not trying to reuse a message object after a start operation
    checkReuse();

    // clear all of the error/result flags before sending
    clearCompletion();

    // validate that the scope override is valid
    receiver->validateScopeOverride(startscope);
    receiver->validateOverrideContext(receiver, startscope);
    // go dispatch the message
    return dispatch();
}


/**
 * Dispatch the message object via a send()
 *
 * @return The result object.
 */
RexxObject *MessageClass::dispatch()
{
    // we need the current activity to handle this
    Activity *myActivity = ActivityManager::currentActivity;

    // ok, now tell the stack frame we're running under that
    // we want to be notified of any errors here.
    myActivity->getTopStackFrame()->setObjNotify(this);

    // mark what activity we're running this under for
    // deadlock detection
    setField(startActivity, myActivity);

    // get a protected object for the result
    ProtectedObject result(myActivity);
    // now issue the message.
    if (startscope != OREF_NULL)
    {
        receiver->messageSend(message, (RexxObject **)args->data(), args->size(), startscope, result);
    }
    else
    {
        receiver->messageSend(message, (RexxObject **)args->data(), args->size(), result);
    }
    resultObject = result;
    setResultReturned();
    // notify any waiters and return the result object
    sendNotification();
    return resultObject;
}


/**
 * Do a dynamic invocation of an object method.
 *
 * @param arguments The variable arguments passed to the method.  The first
 *                  argument is a required message target, which can be either
 *                  a string method name or an array containing a name/scope
 *                  pair.  The remainder of the arguments are the message
 *                  arguments.
 * @param argCount
 *
 * @return The method result.
 */
RexxObject *MessageClass::startRexx(RexxObject **arguments, size_t argCount)
{
    // we must have a message name argument
    if (argCount != 0)
    {
        // if we've been given a new receiver, then use that
        if (arguments[0] != OREF_NULL)
        {
            setField(receiver, arguments[0]);
        }
    }

    // given arguments with the message?  This replaces any arguments
    // we were created with.
    if (argCount > 1)
    {
        setField(args, new_array(argCount - 1, arguments + 1));
    }

    // now go perform the send
    return start();
}


/**
 * Do a dynamic invocation of an object method.
 *
 * @param receiver  An optional receiver target
 * @param arguments An array of arguments to used with the message invocation.
 *
 * @return The method result.
 */
RexxObject *MessageClass::startWithRexx(RexxObject *newReceiver, ArrayClass *arguments)
{
    // if we've been given a new receiver, then use that
    if (newReceiver != OREF_NULL)
    {
        setField(receiver, newReceiver);
    }

    // with sendWith, the arguments are required
    arguments = arrayArgument(arguments, "message arguments");
    setField(args, arguments);

    // now go perform the send
    return start();
}


/**
 * Execute this message asynchronously by spawning a
 * new thread and dispatching this message on the new thread.
 *
 * @param _receiver The optional receiver object.
 *
 * @return returns nothing as a instruction message send.
 */
RexxObject *MessageClass::start()
{
    // make sure we're not trying to reuse a message object after a start operation
    checkReuse();

    // clear all of the error/result flags before sending
    clearCompletion();

    // ok, mark this as pending dispatch so that it can't be
    // started a second time.
    setStartPending();

    // validate that the scope override is valid
    receiver->validateScopeOverride(startscope);
    receiver->validateOverrideContext(receiver, startscope);

    // spawn a new activity off of the old activity
    Activity *oldActivity = ActivityManager::currentActivity;
    Activity *newActivity = oldActivity->spawnReply();
    // mark which activity we're running on, then dispatch the message
    // on the new activity (which is sitting waiting for work to perform)
    setField(startActivity, newActivity);
    newActivity->run(this);
    // we have no return value.
    return OREF_NULL;
}


/**
 * Do a dynamic invocation of an object method.
 *
 * @param arguments The variable arguments passed to the method.  The first
 *                  argument is a required message target, which can be either
 *                  a string method name or an array containing a name/scope
 *                  pair.  The remainder of the arguments are the message
 *                  arguments.
 * @param argCount
 *
 * @return The method result.
 */
RexxObject *MessageClass::replyRexx(RexxObject **arguments, size_t argCount)
{
    // we must have a message name argument
    if (argCount != 0)
    {
        // if we've been given a new receiver, then use that
        if (arguments[0] != OREF_NULL)
        {
            setField(receiver, arguments[0]);
        }
    }

    // given arguments with the message?  This replaces any arguments
    // we were created with.
    if (argCount > 1)
    {
        setField(args, new_array(argCount - 1, arguments + 1));
    }

    // now go perform the send
    return reply();
}


/**
 * Do a dynamic invocation of an object method.
 *
 * @param receiver  An optional receiver target
 * @param arguments An array of arguments to used with the message invocation.
 *
 * @return The method result.
 */
RexxObject *MessageClass::replyWithRexx(RexxObject *newReceiver, ArrayClass *arguments)
{
    // if we've been given a new receiver, then use that
    if (newReceiver != OREF_NULL)
    {
        setField(receiver, newReceiver);
    }

    // with sendWith, the arguments are required
    arguments = arrayArgument(arguments, "message arguments");
    setField(args, arguments);

    // now go perform the send
    return reply();
}


/**
 * Execute this message asynchronously by spawning a
 * new thread and dispatching this message on the new thread.
 *
 * @param _receiver The optional receiver object.
 *
 * @return returns nothing as a instruction message send.
 */
MessageClass *MessageClass::reply()
{
    // make sure we're not trying to reuse a message object after a start operation
    checkReuse();

    // clear all of the error/result flags before sending
    clearCompletion();

    // validate that the scope override is valid
    receiver->validateScopeOverride(startscope);
    receiver->validateOverrideContext(receiver, startscope);

    // make a copy of this object to return as an execution tracker.  This is the
    // one that gets started.
    Protected<MessageClass> newMessage = (MessageClass *)copy();
    // start it running and return
    newMessage->start();
    return newMessage;
}


/**
 * Notify all interested parties after this message completed.
 */
void MessageClass::sendNotification()
{
    // we're no longer interested in any errors that occur.
    ActivityManager::currentActivity->getTopStackFrame()->setObjNotify(OREF_NULL);
    // if we have waiting activities, iterate over them and tell their activities to wake up
    if (waitingActivities != OREF_NULL)
    {
        size_t count = waitingActivities->items();
        for (size_t i = 1; i <= count; i++)
        {
            // get each activity and give them a poke.
            Activity *waitingActivity = (Activity *)waitingActivities->get(i);
            // we process this like it is a guard variable reserve, so
            // we wake it up as if it was a guard wait.
            waitingActivity->guardPost();
        }
        // clear the list so that we don't anchor those activities needlessly
        waitingActivities = OREF_NULL;
    }

    // now see if we have any interested parties to notify.
    if (interestedParties != OREF_NULL)
    {
        size_t count = interestedParties->lastIndex();
        for (size_t i = 1; i <= count; i++)
        {
            // get each message and give them a poke.
            RexxObject *waitingMessage = (RexxObject *)interestedParties->get(i);
            ProtectedObject result;
            waitingMessage->sendMessage(GlobalNames::MessageComplete, this, result);
        }
    }

    // indicate we've notified everybody
    setAllNotified();
}


/**
 * Field a message completion message notification.  This
 * triggers the message object to fire.
 *
 * @param messageSource
 *               The source of the notification message (ignored for message objects).
 *
 * @return always returns OREF_NULL
 */
RexxObject *MessageClass::messageCompleted(RexxObject *messageSource)
{
    // just trigger the message send and return
    send();
    return OREF_NULL;
}


/**
 * Receive an error notificatiion from an activation.
 *
 * @param _condition The error condition object.
 */
void MessageClass::error(DirectoryClass *_condition)
{
    // indicate we've had an error and save the condition object in case it
    // was requested.
    setRaiseError();
    setField(condition, _condition);
    // send out any completion notifications
    sendNotification();
}


/**
 * Give the message completion status.
 *
 * @return .true if this has completed, .false otherwise.
 */
RexxObject *MessageClass::completed()
{
    // we're complete if we have a result or have an error.
    return booleanObject(resultReturned() || raiseError());
}


/**
 * Check to see if a message has an error condition.  Return false
 * if the message has not completed yet.
 *
 * @return True if the message has terminated with an error condition,
 *         false if it is still running or has completed without error.
 */
RexxObject *MessageClass::hasError()
{
    return booleanObject(raiseError());
}


/**
 * Check to see if a message has a result available.  Returns
 * false if the message has not completed yet.
 *
 * @return True if the message has completed with a returned
 *         result, false if it is still running or has
 *         completed without returning a result.
 */
RexxObject *MessageClass::hasResult()
{
    return booleanObject(resultObject != OREF_NULL);
}


/**
 * Return any error condition information associated with the
 * message.  This method will not block until completion, and
 * will return .nil if the message is still running.
 *
 * @return Any condition object from a terminating error, or .nil if
 *         there was no error or the message is still running.
 */
RexxObject *MessageClass::errorCondition()
{
    return resultOrNil(condition);
}


/**
 * Retrieve the target of the message object.  This will be either
 * the target object specified when the message object is created
 * or a target override object specified on SEND or START.
 *
 * @return The current message target.
 */
RexxObject *MessageClass::messageTarget()
{
    return receiver;

}


/**
 * Return the name of the message.
 *
 * @return The string name of the message.
 */
RexxString *MessageClass::messageName()
{
    return message;
}


/**
 * Return a copy of the message argument array.
 *
 * @return A copy of the message arguments array.
 */
ArrayClass *MessageClass::arguments()
{
    return (ArrayClass *)args->copy();
}


RexxObject *MessageClass::halt(RexxString *description)
{
    description = optionalStringArgument(description, OREF_NULL, "description");

    // not started?  can't halt
    if (startActivity == OREF_NULL)
    {
        return TheFalseObject;
    }
    // try to halt...the activity tells us if this was successful.
    return booleanObject(startActivity->halt(description));
}


/**
 * Create a message object from Rexx code.
 *
 * @param msgArgs  The pointer to the new arguments.
 * @param argCount The argument count.
 *
 * @return A new instance of the .Message class.
 */
RexxObject *MessageClass::newRexx(RexxObject **msgArgs, size_t argCount)
{
    // this class is defined on the object class, but this is actually attached
    // to a class object instance.  Therefore, any use of the this pointer
    // will be touching the wrong data.  Use the classThis pointer for calling
    // any methods on this object from this method.
    RexxClass *classThis = (RexxClass *)this;

    size_t num_args = argCount;

    // the first two arguments are required
    if (num_args < 2 )
    {
        reportException(Error_Incorrect_method_minarg,  IntegerTwo);
    }

    // The receiver and the message name are required
    RexxObject *_target   = msgArgs[0];
    requiredArgument(_target, "message target");
    RexxObject *_message  = msgArgs[1];
    requiredArgument(_message, "message name");
    ProtectedObject msgName;
    ProtectedObject _startScope;
    // decode the message argument into name and scope
    RexxObject::decodeMessageName(_target, _message, msgName, _startScope);

    Protected<ArrayClass> argPtr;

    // are there arguments to be sent with the message?
    if (num_args > 2 )
    {
        // get 3rd arg only concerned w/ 1st
        // this is now required
        char option = optionArgument(msgArgs[2], "AI", "argument style");
        // arguments passed as an array
        if (option == 'A')
        {
            // this must be here, and there can only be one additional argument
            if (num_args < 4)
            {
                reportException(Error_Incorrect_method_minarg, IntegerFour);
            }
            if (num_args > 4)
            {
                reportException(Error_Incorrect_method_maxarg, IntegerFour);
            }

            // get this directly and this is required to be an array
            argPtr = arrayArgument(msgArgs[3], "message arguments");
        }
        // specified as individual arguments, so we just build an array
        // directly from that.  There need not be anything following the
        // option
        else if (option == 'I' )
        {
            argPtr = new_array(argCount - 3, msgArgs + 3);
        }
    }

    // if no arguments provided, default to a null array
    if (argPtr == OREF_NULL)
    {
        argPtr = new_array();
    }

    // we're all ready to create this now
    MessageClass *newMessage = new MessageClass(_target, msgName, _startScope, argPtr);
    ProtectedObject p(newMessage);

    // handle Rexx class completion
    classThis->completeNewObject(newMessage);

    return newMessage;
}

