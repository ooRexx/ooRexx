/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.ibm.com/developerworks/oss/CPLv1.0.htm                          */
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
/*****************************************************************************/
/* REXX Windows Support                                                      */
/*                                                                           */
/* Main interpreter control.  This is the preferred location for all         */
/* platform independent global variables.                                    */
/* The interpreter does not instantiate an instance of this                  */
/* class, so most variables and methods should be static.                    */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/

#include "Interpreter.hpp"
#include "ActivityManager.hpp"
#include "ListClass.hpp"
#include "SystemInterpreter.hpp"
#include "InterpreterInstance.hpp"
#include "DirectoryClass.hpp"
#include "ProtectedObject.hpp"
#include "RexxInternalApis.h"
#include "PackageManager.hpp"


// global resource lock
SysMutex Interpreter::resourceLock;

RexxList *Interpreter::interpreterInstances = OREF_NULL;

// the local server object
RexxObject *Interpreter::localServer = OREF_NULL;

// the interpreter active state flag
bool Interpreter::active = false;
// used for timeslice dispatching
bool Interpreter::timeSliceElapsed = false;


/**
 * Initialize the interpreter subsystem.
 */
void Interpreter::init()
{
    interpreterInstances = new_list();
}


void Interpreter::live(size_t liveMark)
{
    memory_mark(interpreterInstances);
    memory_mark(localServer);
    memory_mark(versionNumber);
}

void Interpreter::liveGeneral(int reason)
{
  if (!memoryObject.savingImage())
  {
      memory_mark_general(interpreterInstances);
      memory_mark_general(localServer);
      memory_mark_general(versionNumber);
  }
}

void Interpreter::processStartup()
{
    // the locks get create in order
    createLocks();
    ActivityManager::createLocks();
    RexxMemory::createLocks();
    // make sure we have a session queue created for this process
}

void Interpreter::processShutdown()
{
    // we destroy the locks in reverse order
    RexxMemory::closeLocks();
    ActivityManager::closeLocks();
    closeLocks();
}


/**
 * Perform interpreter startup processing.
 *
 * @param mode   The startup mode.  This indicates whether we're saving the
 *               image or in shutdown mode.
 */
void Interpreter::startInterpreter(InterpreterStartupMode mode)
{
    ResourceSection lock;

    // has everything been shutdown?
    if (!isActive())
    {
        SystemInterpreter::startInterpreter();   // perform system specific initialization
        // initialize the memory manager , and restore the
        // memory image
        memoryObject.initialize(mode == RUN_MODE);
        RexxCreateSessionQueue();
        // create our instances list
        interpreterInstances = new_list();
        // if we have a local server created already, don't recurse.
        if (localServer == OREF_NULL)
        {
            // Get an instance.  This also gives the root activity of the instance
            // the kernel lock.
            InstanceBlock instance;
            /* get the local environment         */
            /* get the server class              */
            RexxObject *server_class = env_find(new_string("!SERVER"));

            // NOTE:  This is a second block so that the
            // protected object's destructor gets run before
            // the activity is removed as the current activity.
            {
                ProtectedObject result;
                /* create a new server object        */
                server_class->messageSend(OREF_NEW, OREF_NULL, 0, result);
                localServer = (RexxObject *)result;
            }
        }
    }
    // we're live now
    active = true;
}


/**
 * Initialize an instance .local object.
 */
void Interpreter::initLocal()
{
    // only do this if the local server has already been created.
    if (localServer != OREF_NULL)
    {
        // this will insert the initial .local objects
        ProtectedObject result;
        localServer->messageSend(OREF_INITINSTANCE, OREF_NULL, 0, result);
    }
}


/**
 * Terminate the global interpreter environment, shutting down
 * all of the interpreter instances that we can and releasing
 * the object heap memory.
 *
 * @return true if everything was shutdown, false if there are reasons
 *         why this can't be shutdown.
 */
bool Interpreter::terminateInterpreter()
{
    {
        ResourceSection lock;   // lock in this section
        // if never even started up, then we've got a quick return
        if (!isActive())
        {
            return true;
        }

        // we can only shutdown interpreter instances from the
        // threads that created them.  If we have active instances,
        // this is a "no can do" situation
        if (interpreterInstances->items() != 0)
        {
            return false;
        }

        {
            try
            {
                // this may seem funny, but we need to create an instance
                // so shut down so that the package manager can unload
                // the libraries (it needs to pass a RexxThreadContext
                // pointer out to package unloaders, if they are defined)
                InstanceBlock instance;
                // run whatever uninits we can before we start releasing the libraries
                memoryObject.lastChanceUninit();

                PackageManager::unload();
            } catch (ActivityException)
            {
                // we're shutting down, so ignore any failures while processing this
            }
        }
        // perform system-specific cleanup
        SystemInterpreter::terminateInterpreter();

        // most interpreter resources will be cleanup automatically, but
        // we need to poke the rxapi daemon and tell it to clean up our session
        // resources.
        RexxDeleteSessionQueue();
    }
    return true;
}


/**
 * Quick test if we're down to just a single interpreter instance.
 *
 * @return true if we're down to a single interpreter instance.
 */
bool Interpreter::lastInstance()
{
    return interpreterInstances->items() == 1;
}


/**
 * Create a new instance and return the instance context pointers
 * and thread context pointer for the instance.
 *
 * @param instance The returned instance pointer.
 * @param threadContext
 *                 The returned thread context pointer.
 * @param options  Options to apply to this interpreter instance.
 *
 * @return 0 if the instance was created ok.
 */
int Interpreter::createInstance(RexxInstance *&instance, RexxThreadContext *&threadContext, RexxOption *options)
{
    try
    {
        // create the instance
        InterpreterInstance *newInstance = createInterpreterInstance(options);
        instance = newInstance->getInstanceContext();
        threadContext = newInstance->getRootThreadContext();
        // we need to ensure we release the kernel lock before returning
        RexxActivity *activity = newInstance->getRootActivity();
        activity->releaseAccess();
        // the activity needs to be in a deactivated state when we return.
        activity->deactivate();
        return 0;
    } catch (ActivityException)
    {
        // not everything works until an instance is actually created, so
        // it's possible we'll see a true failure here, so give back an
        // error condition.
        return RXAPI_MEMFAIL;
    }
}


/**
 * Create a new interpreter instance.  An interpreter instance
 * is an accessible set of threads that constitutes an interpreter
 * environment for the purposes API access.
 *
 * @param exits  The set of exits to use for this invocation.
 * @param defaultEnvironment
 *               The default addressible environment.
 *
 * @return The new interpreter instance.
 */
InterpreterInstance *Interpreter::createInterpreterInstance(RexxOption *options)
{
    // We need to ensure that the interpreter is initialized before we create an
    // interpreter instance.  There are some nasty recursion problems that can result,
    // so this needs to be done carefully and the initialization needs to be protected by
    // the resource lock during the entire process.
    {
        ResourceSection lock;
        // if our instances list has not been created yet, then the memory subsystem has not
        // been created yet.  Keep the lock during the entire process.
        if (interpreterInstances == OREF_NULL)
        {
            startInterpreter(RUN_MODE);
        }
    }


    // get a new root activity for this instance.  This might result in pushing a prior level down the
    // stack
    RexxActivity *rootActivity = ActivityManager::getRootActivity();
    // ok, we have an active activity here, so now we can allocate a new instance and bootstrap everything.
    InterpreterInstance *instance = new InterpreterInstance();

    {
        ResourceSection lock;

        // add this to the active list
        interpreterInstances->append((RexxObject *)instance);
    }

    // now that this is protected from garbage collection, go and initialize everything
    instance->initialize(rootActivity, options);
    return instance;
}


/**
 * Shutdown an interpreter instance and remove it from the list
 * of accessible items.
 *
 * @param instance The instance we're shutting down.
 *
 * @return true if this instance was in a state that could be terminated.
 */
bool Interpreter::terminateInterpreterInstance(InterpreterInstance *instance)
{
    // instance has already shut itself down....we need to remove it from
    // the active list.
    ResourceSection lock;

    interpreterInstances->removeItem((RexxObject *)instance);
    return true;
}


/**
 * Tell the interpreter to have all of the instances halt its activities.
 */
bool Interpreter::haltAllActivities()
{
    ResourceSection lock;
    bool result = true;

    for (size_t listIndex = interpreterInstances->firstIndex() ;
         listIndex != LIST_END;
         listIndex = interpreterInstances->nextIndex(listIndex) )
    {
                                         /* Get the next message object to    */
                                         /*process                            */
        InterpreterInstance *instance = (InterpreterInstance *)interpreterInstances->getValue(listIndex);
        // halt every thing
        result = result && instance->haltAllActivities();
    }
    return result;
}


/**
 * Manage a context where a new interpreter instance is created
 * for the purposes of acquiring an activity, and the activity
 * is released and the instance is terminated upon leaving the
 * block.
 */
InstanceBlock::InstanceBlock()
{
    // Get an instance.  This also gives the root activity of the instance
    // the kernel lock.
    instance = Interpreter::createInterpreterInstance();
    activity = instance->getRootActivity();
}


/**
 * Manage a context where a new interpreter instance is created
 * for the purposes of acquiring an activity, and the activity
 * is released and the instance is terminated upon leaving the
 * block.
 */
InstanceBlock::InstanceBlock(RexxOption *options)
{
    // Get an instance.  This also gives the root activity of the instance
    // the kernel lock.
    instance = Interpreter::createInterpreterInstance(options);
    activity = instance->getRootActivity();
}


/**
 * Manage a context where a new interpreter instance is created
 * for the purposes of acquiring an activity, and the activity
 * is released and the instance is terminated upon leaving the
 * block.
 */
InstanceBlock::InstanceBlock(PRXSYSEXIT exits, const char *env)
{
    size_t optionCount = 0;

    RexxOption options[3];

    // if we have exits specified, add this to the option set
    if (exits != NULL)
    {
        options[optionCount].optionName = REGISTERED_EXITS;
        options[optionCount].option = (void *)exits;
        optionCount++;
    }

    if (env != NULL)
    {
        options[optionCount].optionName = INITIAL_ADDRESS_ENVIRONMENT;
        options[optionCount].option = (CSTRING)env;
        optionCount++;
    }

    // set the marker for the end of the options
    options[optionCount].optionName = NULL;

    // Get an instance.  This also gives the root activity of the instance
    // the kernel lock.
    instance = Interpreter::createInterpreterInstance(&options[0]);
    activity = instance->getRootActivity();
}


/**
 * Release the kernal access and cleanup when the context block
 * goes out of scope.
 */
InstanceBlock::~InstanceBlock()
{
    activity->exitCurrentThread();
    // terminate the instance
    instance->terminate();
}


/**
 * Decode a condition directory into a easier to use
 * structure form for a native code user.  This breaks
 * the directory into its component pieces, including
 * converting values into primitive form using just a single
 * API call.
 *
 * @param conditionObj
 *               A directory object containing the condition information.
 * @param pRexxCondData
 *               The condition data structure that is populated with the
 *               condition information.
 */
void Interpreter::decodeConditionData(RexxDirectory *conditionObj, RexxCondition *condData)
{
    memset(condData, 0, sizeof(RexxCondition));
    condData->code = messageNumber((RexxString *)conditionObj->at(OREF_CODE));
    // just return the major part
    condData->rc = messageNumber((RexxString *)conditionObj->at(OREF_RC))/1000;
    condData->conditionName = (RexxStringObject)conditionObj->at(OREF_CONDITION);

    RexxObject *temp = conditionObj->at(OREF_NAME_MESSAGE);
    if (temp != OREF_NULL)
    {
        condData->message = (RexxStringObject)temp;
    }

    temp = conditionObj->at(OREF_ERRORTEXT);
    if (temp != OREF_NULL)
    {
        condData->errortext = (RexxStringObject)temp;
    }

    temp = conditionObj->at(OREF_DESCRIPTION);
    if (temp != OREF_NULL)
    {
        condData->description = (RexxStringObject)temp;
    }

    // this could be raised by a termination exit, so there might not be
    // position information available
    temp = conditionObj->at(OREF_POSITION);
    if (temp != OREF_NULL)
    {
        condData->position = ((RexxInteger *)temp)->wholeNumber();
    }
    else
    {
        condData->position = 0;
    }

    temp = conditionObj->at(OREF_PROGRAM);
    if (temp != OREF_NULL)
    {
        condData->program = (RexxStringObject)temp;
    }

    temp = conditionObj->at(OREF_ADDITIONAL);
    if (temp != OREF_NULL)
    {
        condData->additional = (RexxArrayObject)temp;
    }
}


/**
 * Default class resolution processing done without benefit of
 * a program context.
 *
 * @param className The class name.
 *
 * @return A resolved class object (if any).
 */
RexxClass *Interpreter::findClass(RexxString *className)
{
    RexxString *internalName = className->upper();   /* upper case it                     */
    /* send message to .local            */
    RexxClass *classObject = (RexxClass *)(ActivityManager::getLocalEnvironment(internalName));
    if (classObject != OREF_NULL)
    {
        return classObject;
    }

    /* last chance, try the environment  */
    return(RexxClass *)(TheEnvironment->at(internalName));
}


/**
 * Return the current queue name.
 *
 * @return The name of the current queue.
 */
RexxString *Interpreter::getCurrentQueue()
{
    RexxObject *queue = ActivityManager::getLocalEnvironment(OREF_REXXQUEUE);

    if (queue == OREF_NULL)              /* no queue?                         */
    {
        return OREF_SESSION;             // the session queue is the default
    }
    // get the current name from the queue object.
    return(RexxString *)queue->sendMessage(OREF_GET);
}


void Interpreter::logicError (const char *desc)
/******************************************************************************/
/* Function:  Raise a fatal logic error                                       */
/******************************************************************************/
{
    printf("Logic error: %s\n",desc);
    exit(RC_LOGIC_ERROR);
}

wholenumber_t Interpreter::messageNumber(
    RexxString *errorcode)             /* REXX error code as string         */
/******************************************************************************/
/* Function:  Parse out the error code string into the messagecode valuey     */
/******************************************************************************/
{
    const char *decimalPoint;            /* location of decimalPoint in errorcode*/
    wholenumber_t  primary = 0;          /* Primary part of error code, major */
    wholenumber_t  secondary = 0;        /* Secondary protion (minor code)    */
    wholenumber_t  count;

    /* make sure we get errorcode as str */
    errorcode = (RexxString *)errorcode->stringValue();
    /* scan to decimal Point or end of   */
    /* error code.                       */
    for (decimalPoint = errorcode->getStringData(), count = 0; *decimalPoint && *decimalPoint != '.'; decimalPoint++, count++);

    // must be a whole number in the correct range
    if (!new_string(errorcode->getStringData(), count)->numberValue(primary) || primary < 1 || primary >= 100)
    {
        /* Nope raise an error.              */
        reportException(Error_Expression_result_raise);

    }
    // now shift over the decimal position.
    primary *= 1000;


    if (*decimalPoint)
    {                 /* Was there a decimal point specified?*/
                      /* is the subcode invalid or too big?*/
        if (!new_string(decimalPoint + 1, errorcode->getLength() - count -1)->numberValue(secondary) || secondary < 0  || secondary >= 1000)
        {
            /* Yes, raise an error.              */
            reportException(Error_Expression_result_raise);
        }
    }
    return primary + secondary;          /* add two portions together, return */
}
