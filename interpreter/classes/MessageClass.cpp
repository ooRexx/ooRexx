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
 * Add a message object to the completion notification list.
 *
 * @param _message The message object wishing to be notified.
 *
 * @return Instruction message type that returns nothing.
 */
RexxObject *MessageClass::notify(MessageClass *_message)
{
    // this is a required argument
    if ( message == OREF_NULL)
    {
        reportException(Error_Incorrect_method_noarg, IntegerOne);
    }
    // this must also be a real message object
    else if (!isOfClass(Message, _message))
    {
        reportException(Error_Incorrect_method_nomessage, _message);
    }

    // got a real message object...now determine if we add this to the
    // pending list or send an immediate notification.
    if (allNotified())
    {
        // an immediate notification
        _message->send(OREF_NULL);
    }
    else
    {
        // get a new array if this is the first one added.
        if (interestedParties == OREF_NULL)
        {
            interestedParties = new_array();
        }
        // to be notified later
        interestedParties->append(_message);
    }
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
    // did running this message cause an error?  If so, we raise the same error
    // condition here.
    if (raiseError())
    {
        ActivityManager::currentActivity->reraiseException(condition);
    }
    else
    {
        // ok, no result ready yet, so we'll have to wait
        if (!resultReturned())
        {
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
            // and wait for the wake up call.
            ActivityManager::currentActivity->waitReserve(this);
            // the message has now completed, but this could now be an error.
            if (raiseError())
            {
                // make sure the error is recored and we've raised the error,
                // then raise the condition on this thread.
                setErrorReported();
                ActivityManager::currentActivity->reraiseException(this->condition);
            }
        }
    }

    // since this is requested via a method that will give an error if used
    // in an expression, return .nil if there is no return value.
    return resultOrNil(resultObject);
}


/**
 * Send a message to the target receiver object (optional).
 *
 * @param _receiver The optional receiver object.
 *
 * @return Returns the message result.
 */
RexxObject *MessageClass::send(RexxObject *_receiver)
{
    // only one send per customer...
    if (msgSent())
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

    // we're sending this synchronously, so mark this as used
    setMsgSent();

    // if we've been given a new receiver, then use that
    if (_receiver != OREF_NULL)
    {
        setField(receiver, _receiver);
    }

    // validate the starting scope if we've been given one
    if (startscope != OREF_NULL)
    {
        if (!receiver->behaviour->hasScope((RexxClass *)startscope))
        {
            reportException(Error_Incorrect_method_array_noclass, IntegerTwo);
        }
    }


    // ok, now tell the stack frame we're running under that
    // we want to be notified any errors here.
    myActivity->getTopStackFrame()->setObjNotify(this);

    // mark what activity we're running this under for
    // deadlock detection                */
    setField(startActivity, myActivity);

    // get a protected object for the result
    ProtectedObject result(myActivity);
    // now issue the message.
    if (startscope != OREF_NULL)
    {
        receiver->messageSend(message, (RexxObject **)args->data(), args->size(), startscope, result);
    }
    else                                 /* no over ride                      */
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
 * Execute this message asynchronously by spawning a
 * new thread and dispatching this message on the new thread.
 *
 * @param _receiver The optional receiver object.
 *
 * @return returns nothing as a instruction message send.
 */
RexxObject *MessageClass::start(RexxObject *_receiver)
{
    // We can only send this once, so if it has already been used
    // or is dispatched for sending, this is an error.
    if (msgSent() || startPending())
    {
        reportException(Error_Execution_message_reuse);
    }

    // ok, mark this as pending dispatch so that it can't be
    // started a second time.
    setStartPending();

    // if we have a new receiver, replace the creation one.
    if (_receiver != OREF_NULL)
    {
        setField(receiver, _receiver);
    }

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
            waitingActivity->postDispatch();
        }
        // clear the list so that we don't anchor those activities needlessly
        waitingActivities = OREF_NULL;
    }

    // now see if we have any interested parties to notify.
    if (interestedParties != OREF_NULL)
    {
        size_t count = interestedParties->items();
        for (size_t i = 1; i <= count; i++)
        {
            // get each message and give them a poke.
            MessageClass *waitingMessage = (MessageClass *)interestedParties->get(i);
            // trigger the message object in a cascade
            waitingMessage->send(OREF_NULL);
        }
        // clear the list so that we don't anchor those messages needlessly
        interestedParties = OREF_NULL;

    }

    // indicate we've notified everybody
    setAllNotified();
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

    size_t num_args = argCount;          /* get number of args passed         */

    // the first two arguments are required
    if (num_args < 2 )
    {
        reportException(Error_Incorrect_method_minarg,  IntegerTwo);
    }

    // The receiver and the message name are required
    RexxObject *_target   = msgArgs[0];
    requiredArgument(_target, ARG_ONE);
    RexxObject *_message  = msgArgs[1];
    requiredArgument(_message, ARG_TWO);
    RexxString *msgName;
    RexxClass *_startScope;
    // decode the message argument into name and scope
    RexxObject::decodeMessageName(_target, _message, msgName, _startScope);

    ArrayClass *argPtr;

    // are there arguments to be sent with the message?
    if (num_args > 2 )
    {
        /* get 3rd arg only concerned w/ 1st */
        RexxString *optionString = (RexxString *)msgArgs[2];
        /*  Did we really get an option      */
        /*passed?                            */
        if (optionString == OREF_NULL)
        {
            /* nope, use null array as argument  */
            argPtr = (ArrayClass *)TheNullArray->copy();
        }
        else
        {
            /* Convert it into a string.         */
            optionString = stringArgument(optionString, ARG_THREE);
            /*  char and make it lower case      */
            char option = tolower(optionString->getChar(0));
            if (option == 'a')               /* args passed as an array?          */
            {
                /* are there less than 4 required    */
                /*args?                              */
                if (num_args < 4)              /* this is an error                  */
                {
                    reportException(Error_Incorrect_method_minarg, IntegerFour);
                }

                /* are there more than 4 required    */
                /*args?                              */
                if (num_args > 4)              /* this is an error                  */
                {
                    reportException(Error_Incorrect_method_maxarg, IntegerFour);
                }

                /* get the array of arguments        */
                argPtr = (ArrayClass *)msgArgs[3];
                if (argPtr == OREF_NULL)       /* no array given?                   */
                {
                    /* this is an error                  */
                    reportException(Error_Incorrect_method_noarg, IntegerFour);
                }
                /* force to array form               */
                argPtr = argPtr->requestArray(); ;
                /* not an array?                     */
                if (argPtr == TheNilObject || argPtr->isMultiDimensional())
                {
                    reportException(Error_Incorrect_method_noarray, msgArgs[3]);
                }
            }
            else if (option == 'i' )         /* specified as individual?          */
            {
                /* yes, build array of all arguments */
                argPtr = new_array(argCount - 3, msgArgs + 3);
            }
            else
            {
                reportException(Error_Incorrect_method_option, "AI", msgArgs[2]);
            }
        }
    }
    else
    {
        /* no args, use a null array.        */
        argPtr = (ArrayClass *)TheNullArray->copy();
    }
    /* all args are parcelled out, go    */
    /*create the new message object...   */
    MessageClass *newMessage = new MessageClass(_target, msgName, _startScope, argPtr);
    ProtectedObject p(newMessage);

    // handle Rexx class completion
    classThis->completeNewObject(newMessage);

    return newMessage;                   /* return the new message            */
}

