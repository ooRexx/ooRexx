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
/* Primitive Message Class                                                    */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "DirectoryClass.hpp"
#include "ArrayClass.hpp"
#include "ListClass.hpp"
#include "MethodClass.hpp"
#include "RexxActivity.hpp"
#include "RexxActivation.hpp"
#include "RexxNativeActivation.hpp"
#include "MessageClass.hpp"
#include "RexxNativeCode.hpp"
#include "ProtectedObject.hpp"

// singleton class instance
RexxClass *RexxMessage::classInstance = OREF_NULL;


/**
 * Create initial class object at bootstrap time.
 */
void RexxMessage::createInstance()
{
    CLASS_CREATE(Message, "Message", RexxClass);
}


/**
 * Create a new message object.
 *
 * @param _target The receiver object.
 * @param messageName
 *                The invoked message name.
 * @param scope   The starting scope (can be OREF_NULL).
 * @param _args   An array of arguments to the message.
 */
RexxMessage::RexxMessage(RexxObject *_target, RexxString *msgName, RexxObject *scope, RexxArray *_args)
{
                                         /* defult target is target specified */
    OrefSet(this, this->receiver, _target);
    OrefSet(this, this->target, _target); /* Target specified on new           */
    /* Args to be sent wuth tmessage     */
    OrefSet(this, this->args, _args);
    OrefSet(this, this->message, msgName);
    OrefSet(this, this->startscope, scope);

    /* initialize a list of message to be*/
    /* once we have a result.            */
    OrefSet(this, this->interestedParties, new RexxList);
}

void RexxMessage::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
    memory_mark(this->receiver);
    memory_mark(this->target);
    memory_mark(this->message);
    memory_mark(this->startscope);
    memory_mark(this->args);
    memory_mark(this->resultObject);
    memory_mark(this->interestedParties);
    memory_mark(this->condition);
    memory_mark(this->startActivity);
    memory_mark(this->objectVariables);
    memory_mark(this->waitingActivities);
}

void RexxMessage::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
    memory_mark_general(this->receiver);
    memory_mark_general(this->target);
    memory_mark_general(this->message);
    memory_mark_general(this->startscope);
    memory_mark_general(this->args);
    memory_mark_general(this->resultObject);
    memory_mark_general(this->interestedParties);
    memory_mark_general(this->condition);
    memory_mark_general(this->startActivity);
    memory_mark_general(this->objectVariables);
    memory_mark_general(this->waitingActivities);
}

void RexxMessage::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxMessage)

   flatten_reference(newThis->receiver, envelope);
   flatten_reference(newThis->target, envelope);
   flatten_reference(newThis->message, envelope);
   flatten_reference(newThis->startscope, envelope);
   flatten_reference(newThis->args, envelope);
   flatten_reference(newThis->resultObject, envelope);
   flatten_reference(newThis->interestedParties, envelope);
   flatten_reference(newThis->condition, envelope);
   flatten_reference(newThis->startActivity, envelope);
   flatten_reference(newThis->objectVariables, envelope);
   flatten_reference(newThis->waitingActivities, envelope);

  cleanUpFlatten
}

RexxObject *RexxMessage::notify(RexxMessage *_message)
/******************************************************************************/
/* Function:  Add a message object to the notification list                   */
/******************************************************************************/
{
    /* is argument a real message object?*/
    if (message != OREF_NULL && isOfClass(Message, _message))
    {
        /* Yes, then add it to the           */
        /* toBeNotified list.                */

        if (this->allNotified())             /* Have all notifications been sent? */
        {
            /* Yes, then send notification right */
            _message->send(OREF_NULL);         /* away                              */
        }
        else
        {
            /* nope, add it to list and wait for */
            /*  for result.                      */
            this->interestedParties->addLast(_message);
        }
    }
    else
    {                                /* nope, its and error, report it.   */
        if ( message == OREF_NULL)
        {
            reportException(Error_Incorrect_method_noarg, IntegerOne);
        }
        else
        {
            reportException(Error_Incorrect_method_nomessage, _message);
        }
    }
    return OREF_NULL;                     /* all done, we return nothing       */
}

RexxObject *RexxMessage::result(void)
/******************************************************************************/
/* Function:  Return the message result...will wait if the message isn't done */
/******************************************************************************/
{

    /* Did send/satrt cause an error     */
    /*condition Yes, we need to raise it */
    /*here.                              */
    if (this->raiseError())
    {
        ActivityManager::currentActivity->reraiseException(this->condition);
    }
    else
    {
        /* Quick test to see if result       */
        /*already present                    */
        if (!this->resultReturned())
        {
            /* got an activity available?        */
            if (this->startActivity != OREF_NULL)
            {
                /* go perform dead lock checks       */
                this->startActivity->checkDeadLock(ActivityManager::currentActivity);
            }

            /* No result yet, now we need to wait*/
            /*  until we get a result.           */
            /* Is anyone else waiting ????       */
            if (this->waitingActivities == OREF_NULL)
            {
                /* No, Create a waiting list         */
                OrefSet(this, this->waitingActivities, new_list());
            }
            /* add this activity to the list     */
            this->waitingActivities->addLast((RexxObject *)ActivityManager::currentActivity);
            /* now go wait to be woken up        */
            ActivityManager::currentActivity->waitReserve((RexxObject *)this);
            if (this->raiseError())             /* do we need to raise an error.     */
            {
                /* yes,                              */
                this->setErrorReported();        /* indicate error was reported, and  */
                /*  report and error.                */
                ActivityManager::currentActivity->reraiseException(this->condition);
            }
        }
    }
    return this->resultObject;            /* ok, return the result.            */
}

RexxObject *RexxMessage::send(RexxObject *_receiver)
/******************************************************************************/
/* Function:  Send the message contained by this message object               */
/******************************************************************************/
{
    if (this->msgSent())
    {
        /* Yes, this is an error             */
        reportException(Error_Execution_message_reuse);
    }

    /* get the activity I'm running under*/
    RexxActivity *myActivity = (RexxActivity *)ActivityManager::currentActivity;
    /* If we have a pending start message*/
    /*  sure this send is a result of    */
    /*that message dispatch.             */
    if (this->startPending() && myActivity != this->startActivity )
    {
        /* Yes, this is an error             */
        reportException(Error_Execution_message_reuse);
    }
    this->setMsgSent();                  /* indicate we were sent a message   */

    if (_receiver != OREF_NULL)          /* new receiver specified?           */
    {
        /* Yes, indicate this is the receiver*/
        OrefSet(this, this->receiver, _receiver);
    }
    /* validate startscope               */
    if (startscope != OREF_NULL)
    {
        if (!this->receiver->behaviour->checkScope(this->startscope))
        {
            reportException(Error_Incorrect_method_array_noclass, IntegerTwo);
        }
    }
    /*  this is a primitive object?      */
    /* tell the activation/nativeact, we */
    /*are running under to let us know   */
    /*if an error occured.               */
    myActivity->getTopStackFrame()->setObjNotify(this);
    /* set this for resource deadlock    */
    /* checking purposes                 */
    OrefSet(this, this->startActivity, myActivity);
    ProtectedObject p(myActivity);
    /*  call message_send to do the send */
    /* and assign our result.            */
    if (this->startscope != OREF_NULL)/* have a starting scope?            */
    {
        /* send it with an override          */
        this->receiver->messageSend(this->message, (RexxObject **)this->args->data(), this->args->size(), this->startscope, p);
    }
    else                                 /* no over ride                      */
    {
        this->receiver->messageSend(this->message, (RexxObject **)this->args->data(), this->args->size(), p);
    }
    this->resultObject = (RexxObject *)p;
    this->setResultReturned();           /* Indicate we have a result.        */
    this->sendNotification();
    return this->resultObject;           /* return the result of the send.    */
}

RexxObject *RexxMessage::start(RexxObject *_receiver)
/******************************************************************************/
/* Function:  Since a start is to happen Async, we create a new activity      */
/*            using ourselves as a dispatch target.                           */
/******************************************************************************/
{
    /* has message already been sent or  */
    /* is another start message pending? */
    if (this->msgSent() || this->startPending())
    {
        /* Yes, this is an error             */
        reportException(Error_Execution_message_reuse);
    }
    /* indicate object has received a    */
    /*start we need this additional bit  */
    /*so that the send message will      */
    /*accept this msg                    */
    this->setStartPending();


    if (_receiver != OREF_NULL)           /* new receiver specified?           */
    {
        /* Yes, indicate this is the receiver*/
        OrefSet(this, this->receiver, _receiver);
    }

    /* get the current activity          */
    RexxActivity *oldActivity = ActivityManager::currentActivity;
    /* Create the new activity           */
    RexxActivity *newActivity = oldActivity->spawnReply();
    /* indicate the activity the send    */
    /*message should come in on.         */
    OrefSet(this, this->startActivity, newActivity);
    // tell the activity to run this
    newActivity->run(this);
    return OREF_NULL;                    /* all done here, return to caller.  */
}

void RexxMessage::sendNotification(void)
/******************************************************************************/
/* Function : we now have a result from message send/start, so notify         */
/*   all interested parties, and post the waitResult semopohore if it exists  */
/******************************************************************************/
{
    /* no longer care about any error    */
    /*condition                          */
    ActivityManager::currentActivity->getTopStackFrame()->setObjNotify(OREF_NULL);
    /* others waiting for a result?      */
    if (this->waitingActivities != OREF_NULL)
    {
        size_t i = this->waitingActivities->getSize();/* get the waiting count             */
        while (i--)                        /* while we have items               */
        {
            /* get the first item                */
            RexxActivity *waitingActivity = (RexxActivity *)this->waitingActivities->removeFirst();
            waitingActivity->postDispatch();  /* go wake it up                     */
        }
    }
    /* now traverse the list of Iterested*/
    /*  parties, and let them know we    */
    /*have a result                      */
    for (size_t listIndex = this->interestedParties->firstIndex() ;
        listIndex != LIST_END;
        listIndex = this->interestedParties->nextIndex(listIndex) )
    {
        /* Get the next message object to    */
        /*process                            */
        RexxMessage *thisMessage = (RexxMessage *)this->interestedParties->getValue(listIndex);
        /* now just have this message send   */
        /*its message                        */
        thisMessage->send(OREF_NULL);
    }

    /* indicate we notified all          */
    /*Interested parties.  Not used      */
    /*yet....                            */
    this->setAllNotified();
}


void RexxMessage::error(
    RexxDirectory *_condition)         /* error condition object            */
/******************************************************************************/
/* Function : called from nativaAct/Activation to notify us that the message  */
/*   from SEND/START.                                                         */
/******************************************************************************/
{
    this->setRaiseError();               /* indicate we had an error condition*/
                                         /* save the condition object in case */
                                         /*we want it.                        */
    OrefSet(this, this->condition, _condition);
    this->sendNotification();            /* do cleanup items.                 */
}

RexxObject *RexxMessage::completed(void)
/******************************************************************************/
/* Function:  Give a completed polling status                                 */
/******************************************************************************/
{
    /* Test to see if result already     */
    /*present or error occured in send?  */
    if (this->resultReturned() || this->raiseError())
    {
        return(RexxObject *)TheTrueObject; /* Yes, return true                  */
    }
    else
    {
        return(RexxObject *)TheFalseObject;/* nope return false.                */
    }
}


/**
 * Check to see if a message has an error condition.  Return false
 * if the message has not completed yet.
 *
 * @return True if the message has terminated with an error condition,
 *         false if it is still running or has completed without error.
 */
RexxObject *RexxMessage::hasError()
{
    if (this->raiseError())
    {
        return TheTrueObject;
    }
    else
    {
        return TheFalseObject;
    }
}

/**
 * Return any error condition information associated with the
 * message.  This method will not block until completion, and
 * will return .nil if the message is still running.
 *
 * @return Any condition object from a terminating error, or .nil if
 *         there was no error or the message is still running.
 */
RexxObject *RexxMessage::errorCondition()
{
    if (this->condition == OREF_NULL)
    {
        return TheNilObject;
    }
    else
    {
        return this->condition;
    }

}


/**
 * Retrieve the target of the message object.  This will be either
 * the target object specified when the message object is created
 * or a target override object specified on SEND or START.
 *
 * @return The current message target.
 */
RexxObject *RexxMessage::messageTarget()
{
    return receiver;

}


/**
 * Return the name of the message.
 *
 * @return The string name of the message.
 */
RexxString *RexxMessage::messageName()
{
    return message;
}


/**
 * Return a copy of the message argument array.
 *
 * @return A copy of the message arguments array.
 */
RexxArray *RexxMessage::arguments()
{
    return (RexxArray *)args->copy();
}


void *RexxMessage::operator new(size_t size)
/******************************************************************************/
/* Function:  Construct a new message object                                  */
/******************************************************************************/
{
    return new_object(size, T_Message);       /* Get new object                    */
}


RexxObject *RexxMessage::newRexx(
    RexxObject **msgArgs,              /* message argument array            */
    size_t       argCount)             /* the number of arguments           */
/******************************************************************************/
/* Function:  Rexx level new routine                                          */
/******************************************************************************/
{
    RexxArray  *argPtr = NULL;           // the arguments used with the message.

    size_t num_args = argCount;          /* get number of args passed         */

    if (num_args < 2 )                   /* passed less than 2 args?          */
    {
        /* Yes, this is an error.            */
        reportException(Error_Incorrect_method_minarg,  IntegerTwo);
    }
    RexxObject *_target   = msgArgs[0];              /* Get the receiver object           */
    requiredArgument(_target, ARG_ONE);
    RexxObject *_message  = msgArgs[1];              /* get the message .                 */
    RexxString *msgName;
    RexxObject *_startScope;
    // decode the message argument into name and scope
    RexxObject::decodeMessageName(_target, _message, msgName, _startScope);

    /* are there arguments to be sent    */
    /*with the message?                  */
    if (num_args > 2 )
    {
        /* get 3rd arg only concerned w/ 1st */
        RexxString *optionString = (RexxString *)msgArgs[2];
        /*  Did we really get an option      */
        /*passed?                            */
        if (optionString == OREF_NULL)
        {
            /* nope, use null array as argument  */
            argPtr = (RexxArray *)TheNullArray->copy();
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
                argPtr = (RexxArray *)msgArgs[3];
                if (argPtr == OREF_NULL)       /* no array given?                   */
                {
                    /* this is an error                  */
                    reportException(Error_Incorrect_method_noarg, IntegerFour);
                }
                /* force to array form               */
                argPtr = (RexxArray *)REQUEST_ARRAY(argPtr);
                /* not an array?                     */
                if (argPtr == TheNilObject || argPtr->getDimension() != 1)
                {
                    /* raise an error                    */
                    reportException(Error_Incorrect_method_noarray, msgArgs[3]);
                }
            }
            else if (option == 'i' )         /* specified as individual?          */
            {
                /* yes, build array of all arguments */
                argPtr = new (argCount - 3, msgArgs + 3) RexxArray;
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
        argPtr = (RexxArray *)TheNullArray->copy();
    }
    /* all args are parcelled out, go    */
    /*create the new message object...   */
    RexxMessage *newMessage = new RexxMessage(_target, msgName, _startScope, argPtr);
    /* actually a subclassed item?       */
    if (((RexxClass *)this)->isPrimitive())
    {
        /* Give new object its behaviour     */
        newMessage->setBehaviour(((RexxClass *)this)->getInstanceBehaviour());
        newMessage->sendMessage(OREF_INIT);/* call any rexx inits               */
    }
    return newMessage;                   /* return the new message            */
}

