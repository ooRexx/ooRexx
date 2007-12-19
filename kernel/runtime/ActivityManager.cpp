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

#include "RexxCore.h"
#include "RexxActivity.hpp"
#include "RexxActivation.hpp"
#include "DirectoryClass.hpp"
#include "ActivityManager.hpp"
#include "Interpreter.hpp"

// The currently active activity.
RexxActivity *ActivityManager::currentActivity = OREF_NULL;

// activities in use
RexxList *ActivityManager::activeActivities = OREF_NULL;

// available activities we can reuse
RexxList *ActivityManager::availableActivities = OREF_NULL;

// table of all activities
RexxList *ActivityManager::allActivities = OREF_NULL;

// the activation Cache
RexxStack *ActivityManager::activations = OREF_NULL;

// size of the activation cache
size_t ActivityManager::activationCacheSize = 0;

// this is the head of the waiting activity chain
RexxActivity *ActivityManager::firstWaitingActivity = OREF_NULL;

// tail of the waiting activity chain
RexxActivity *ActivityManager::lastWaitingActivity = OREF_NULL;

// process shutting down flag
bool ActivityManager::processTerminating = false;

// number of active interpreter instances in this process
size_t ActivityManager::interpreterInstances = 0;

// the local environment
RexxDirectory *ActivityManager::localEnvironment = OREF_NULL;

// the local server object
RexxObject *ActivityManager::localServer = OREF_NULL;

// global lock for the interpreter
SMTX ActivityManager::kernelSemaphore = 0;

const size_t ACTIVATION_CACHE_SIZE = 5;

// TODO:  This needs to be moved into a manager class of some type....potentially here.
void kernelShutdown (void);

/**
 * Initialize the activity manager when the interpreter starts up.
 */
void ActivityManager::init()
{
    activeActivities = new_list();
    availableActivities = new_list();
    allActivities = new_list();
    activations = new_stack(ACTIVATION_CACHE_SIZE);
    currentActivity = OREF_NULL;
    localEnvironment = new_directory();
}

void ActivityManager::live(size_t liveMark)
/******************************************************************************/
/* NOTE: we do not mark the UninitTables.  MEMORY will request the table      */
/*  and mark it for us.  This is so that it can determine if there are        */
/*  any objects that a "dead" and need uninit run.  Activity will run the     */
/*  UNINIT, but we let Garbage Collection, handle detection/etc.              */
/* NOTE: we also do not mark the subClasses table.  This will be managed      */
/*  by memory so that we can reclaim classes once all of the instances have   */
/*  also been reclaimed.                                                      */
/******************************************************************************/
{
  memory_mark(activeActivities);
  memory_mark(availableActivities);
  memory_mark(allActivities);
  memory_mark(activations);
  memory_mark(firstWaitingActivity);
  memory_mark(lastWaitingActivity);
  memory_mark(localEnvironment);
  memory_mark(localServer);
}

void ActivityManager::liveGeneral(int reason)
/******************************************************************************/
/* NOTE: we do not mark the UninitTables.  MEMORY will request the table      */
/*  and mark it for us.  This is so that it can determine if there are        */
/*  any objects that a "dead" and need uninit run.  Activity will run the     */
/*  UNINIT, but we let Garbage Collection, handle detection/etc.              */
/*  The subClasses table is only marked during a save image, so that the      */
/*  classes will still have the proper subclass definitions.                  */
/******************************************************************************/
{
  if (!memoryObject.savingImage())
  {
      memory_mark_general(activeActivities);
      memory_mark_general(availableActivities);
      memory_mark_general(allActivities);
      memory_mark_general(activations);
      memory_mark_general(firstWaitingActivity);
      memory_mark_general(lastWaitingActivity);
      memory_mark_general(localEnvironment);
      memory_mark_general(localServer);
  }
}


void ActivityManager::addWaitingActivity(
    RexxActivity *waitingAct,          /* new activity to add to the queue  */
    bool          release )            /* need to release the run semaphore */
/******************************************************************************/
/* Function:  Add an activity to the round robin wait queue                   */
/******************************************************************************/
{
    ResourceSection lock;                // need the control block locks

                                         /* NOTE:  The following assignments  */
                                         /* do not use OrefSet intentionally. */
                                         /* because we do have yet have kernel*/
                                         /* access, we can't allow memory to  */
                                         /* allocate a new counter object for */
                                         /* this.  This leads to memory       */
                                         /* corruption and unpredictable traps*/
                                         /* nobody waiting yet?               */
    if (firstWaitingActivity == OREF_NULL)
    {
        /* this is the head of the chain     */
        firstWaitingActivity = waitingAct;
        /* and the tail                      */
        lastWaitingActivity = waitingAct;
        lock.release();                  // release the lock now
    }
    else
    {                                    /* move to the end of the line       */
                                         /* chain off of the existing one     */
        lastWaitingActivity->setNextWaitingActivity(waitingAct);
        /* this is the new last one          */
        lastWaitingActivity = waitingAct;
        waitingAct->clearWait();           /* clear the run semaphore           */
        lock.release();                    // release the lock now
        if (release)                       /* current semaphore owner?          */
        {
            unlockKernel();
        }
        SysThreadYield();                  /* yield the thread                  */
        waitingAct->waitKernel();          /* and wait for permission           */
    }
    lockKernel();                        // get the kernel lock now
    lock.reacquire();                    // get the resource lock back
                                         /* NOTE:  The following assignments  */
                                         /* do not use OrefSet intentionally. */
                                         /* because we do have yet have kernel*/
                                         /* access, we can't allow memory to  */
                                         /* allocate a new counter object for */
                                         /* this.  This leads to memory       */
                                         /* corruption and unpredictable traps*/
                                         /* dechain the activity              */

    /* firstWaitingActivity will be released, so set first to next of first
       The original outcommented code was setting the first to the next of the
       activity that got the semaphore. This could corrupt the list if threads
       are not released in fifo */

    if (firstWaitingActivity != OREF_NULL)
    {
        firstWaitingActivity = firstWaitingActivity->getNextWaitingActivity();
    }
    /* clear out the chain               */
    /* if we are here, newActivity must have been firstWaitingActivity sometime
       before and therefore we can set next pointer to NULL without disturbing
       the linked list */

    waitingAct->setNextWaitingActivity(OREF_NULL);
    /* was this the only one?            */
    if (firstWaitingActivity == OREF_NULL)
    {
        /* clear out the last one            */
        lastWaitingActivity = OREF_NULL;
    }
    else                                 /* release the next one to run       */
    {
        firstWaitingActivity->postRelease();
    }
    currentActivity = waitingAct;        /* set new current activity          */
    /* and new active settings           */
    Numerics::setCurrentSettings(waitingAct->getNumericSettings());
}


/**
 * Terminate an interpreter instance.  This starts process
 * shutdown if the last instance goes away.
 */
void ActivityManager::createInterpreter()
{
    //TODO:  more stuff should be moved into here.
    interpreterInstances++;
}

/**
 * Terminate an interpreter instance.  This starts process
 * shutdown if the last instance goes away.
 */
void ActivityManager::terminateInterpreter()
{
    ResourceSection lock;
    interpreterInstances--;              /* reduce the active count           */
    if (interpreterInstances == 0)       /* down to nothing?                  */
    {
                                         /* force termination                 */
        shutdown();
    }
}


/**
 * Shutdown the activity manager and initiate interpreter termination.
 */
void ActivityManager::shutdown()
{
    processTerminating = true;
    lockKernel();
                                       /* Make sure we wake up server       */
                                       /* Make sure all free Activities     */
                                       /*  get the terminate message        */
                                       /* done after uninit calls. incas    */
                                       /*  uninits needed some.             */
    clearActivityPool();
    // if there are no activities yet to shutdown, we can terminate immediately.
    // Otherwise, we need to wait for the rest of the threads to finish cleaning
    // up.  Once the last thread terminates, we can shutdown.
    checkShutdown();
    localEnvironment = OREF_NULL;      /* no local environment              */
    unlockKernel();                    /* Done with Kernel stuff            */
}


/**
 * Check to see it it's time to shutdown the entire kernel.
 * This only occurs once all of the activities have shut down.
 */
void ActivityManager::checkShutdown()
{
    // if this is the last thread, time to shutdown
    if (allActivities->items() == 0)
    {
        kernelShutdown();                /* time to shut things down          */
    }
}


RexxActivation *ActivityManager::newActivation(
     RexxObject     *receiver,         /* message receiver                  */
     RexxMethod     *runMethod,        /* method to run                     */
     RexxActivity   *activity,         /* current activity                  */
     RexxString     *msgname,          /* message name processed            */
     RexxActivation *activation,       /* parent activation                 */
     int             context )         /* execution context                 */
/******************************************************************************/
/* Function:  Get an activation from cache or create new one                  */
/******************************************************************************/
{

    if (activationCacheSize != 0)  /* have a cached entry?              */
    {
        activationCacheSize--;       /* remove an entry from the count    */
                                           /* get the top cached entry          */
        RexxActivation *resultActivation = (RexxActivation *)activations->stackTop();
        /* reactivate this                   */
        resultActivation->setHasReferences();
        resultActivation = new (resultActivation) RexxActivation (receiver, runMethod, activity, msgname, activation, context);
        activations->pop();          /* Remove reused activation from stac*/
        return resultActivation;

    }
    else                                 /* need to create a new one          */
    {
        /* Create new Activation.            */
        return new RexxActivation (receiver, runMethod, activity, msgname, activation, context);

    }
}


void ActivityManager::cacheActivation(
  RexxActivation *activation )         /* activation to process             */
/******************************************************************************/
/* Function:  Save an activation to the cache.                                */
/******************************************************************************/
{
    /* still room in the cache?          */
    if (activationCacheSize < ACTIVATION_CACHE_SIZE)
    {
        /* free everything for reclamation   */
        activation->setHasNoReferences();
        activationCacheSize++;       /* add the this to the count         */
                                           /* and add the activation            */
        activations->push((RexxObject *)activation);
    }
}


RexxActivity *ActivityManager::newActivity(int priority)
/******************************************************************************/
/* Function:  Create or reuse an activity object                              */
/******************************************************************************/
{
  ResourceSection lock;                // lock the control information

  RexxActivity *activity = OREF_NULL;  /* no activity yet                   */
  if (priority != NO_THREAD)           /* can we reuse one?                 */
  {
                                       /* try to get one from the free table*/
      activity =  (RexxActivity *)availableActivities->removeFirstItem();
  }

  if (activity == OREF_NULL)
  {
    lock.release();                    // release lock while creating new activity
                                       /* Create a new activity object      */
    activity = new RexxActivity(false, priority);
    lock.reacquire();                  // need this back again
                                       /* Add this activity to the table of */
                                       /* in use activities and the global  */
                                       /* table                             */
    activeActivities->append((RexxObject *)activity);
    allActivities->append((RexxObject *)activity);
  }
  else
  {
                                       /* We are able to reuse an activity, */
                                       /*  so just re-initialize it.        */
    new (activity) RexxActivity(true, priority);
    // this one is in use now
    activeActivities->append((RexxObject *)activity);
  }
  return activity;                     /* return the activity               */
}


/**
 * Raise a halt condition on all running activities.
 */
void ActivityManager::haltAllActivities()
{
    for (size_t listIndex = activeActivities->firstIndex() ;
         listIndex != LIST_END;
         listIndex = activeActivities->nextIndex(listIndex) )
    {
                                         /* Get the next message object to    */
                                         /*process                            */
        RexxActivity *activity = (RexxActivity *)allActivities->getValue(listIndex);
        RexxActivation *currentActivation = activity->getCurrentActivation();

        if (currentActivation != (RexxActivationBase *)TheNilObject)
        {
                                         /* Yes, issue the halt to it.        */
            ((RexxActivation *)currentActivation)->halt(OREF_NULL);
        }
    }
}


/**
 * Raise a trace condition on all running activities.
 */
void ActivityManager::traceAllActivities(bool on)
{
    for (size_t listIndex = activeActivities->firstIndex() ;
         listIndex != LIST_END;
         listIndex = activeActivities->nextIndex(listIndex) )
    {
                                         /* Get the next message object to    */
                                         /*process                            */
        RexxActivity *activity = (RexxActivity *)allActivities->getValue(listIndex);
        RexxActivation *currentActivation = activity->getCurrentActivation();

        if (currentActivation != (RexxActivationBase *)TheNilObject)
        {
            if (on)               /* turning this on?                  */
            {
                                         /* turn tracing on                   */
                currentActivation->externalTraceOn();
            }
            else
            {
                                         /* turn tracing off                  */
                currentActivation->externalTraceOff();
            }
        }
    }
}


void ActivityManager::clearActivityPool()
/******************************************************************************/
/* Function:   see if there are any Uninit messages need to be send before    */
/*             the process goes away.                                         */
/******************************************************************************/
{
    RexxActivity *activity = (RexxActivity *)availableActivities->removeFirstItem();
    while (activity != OREF_NULL)
    {
        // terminate this thread
        activity->terminateMethod();
        activity = (RexxActivity *)availableActivities->removeFirstItem();
    }
}


/**
 * Return an activity to the available pool.  If we're in the
 * process of shutting down, or have too many pool activities,
 * we'll return false to tell the activity to completely
 * shut things down.
 *
 * @param activity The activity we're adding back to the available pool.
 *
 * @return true if this was pooled, false if the thread should not wait for
 *         more work.
 */
bool ActivityManager::poolActivity(RexxActivity *activity)
{
    // remove this from the active list
    activeActivities->removeItem((RexxObject *)activity);

    // are we shutting down?
    if (processTerminating)
    {
        // have the activity clean up its resources.
        activity->terminateActivity();

        // remove this from the activity list
        allActivities->removeItem((RexxObject *)activity);
        // if this is the last thread, time to shutdown
        checkShutdown();
        return false;
    }
    else if (availableActivities->items() > MAX_THREAD_POOL_SIZE)
    {
        // have the activity clean up its resources.
        activity->terminateActivity();

        // remove this from the activity list
        allActivities->removeItem((RexxObject *)activity);
        return false;
    }
    else
    {
        // just add this to the available list
        availableActivities->append((RexxObject *)activity);
        return true;   // this was successfully pooled
    }
}


bool ActivityManager::haltActivity(
     thread_id_t  thread_id,           /* target thread id                  */
     RexxString * description )        /* description to use                */
/******************************************************************************/
/* Function:   Flip on a bit in a target activities top activation            */
/******************************************************************************/
{
    ResourceSection lock;
    // locate the activity associated with this thread_id.  If not found, return
    // a failure.
    RexxActivity *activity = findActivity(thread_id);
    if (activity != OREF_NULL)
    {
        return activity->halt(description);
    }
    return false;                        // this was a failure
}

bool ActivityManager::yieldActivity(
     thread_id_t  thread_id)           /* target thread id                  */
/****************************************************************************/
/* Function:   Flip on a bit in a target activities top activation          */
/*             called from rexxsetyield                                     */
/****************************************************************************/
{
    ResourceSection lock;
    // locate the activity associated with this thread_id.  If not found, return
    // a failure.
    RexxActivity *activity = findActivity(thread_id);
    if (activity != OREF_NULL)
    {
        activity->yield();
        return true;                     /* this actually worked              */
    }
    return false;                        // this was a failure
}


bool ActivityManager::setActivityTrace(
     thread_id_t thread_id,            /* target thread id                  */
     bool  on_or_off )                 /* trace on/off flag                 */
/******************************************************************************/
/* Function:   Flip on a bit in a target activities top activation            */
/******************************************************************************/
{
    ResourceSection lock;
    // locate the activity associated with this thread_id.  If not found, return
    // a failure.
    RexxActivity *activity = findActivity(thread_id);
    if (activity != OREF_NULL)
    {
        return activity->setTrace(on_or_off);
    }
    return false;                        // this was a failure
}


void ActivityManager::yieldCurrentActivity()
/******************************************************************************/
/* Function:   Signal an activation to yield control                          */
/******************************************************************************/
{
    ResourceSection lock;

    RexxActivity *activity = ActivityManager::currentActivity;
    if (activity != OREF_NULL)
    {
        activity->yield();
    }
}


RexxActivity *ActivityManager::findActivity(thread_id_t threadId)
/******************************************************************************/
/* Function:  Locate the activity associated with a thread                    */
/******************************************************************************/
{
    for (size_t listIndex = allActivities->firstIndex() ;
         listIndex != LIST_END;
         listIndex = allActivities->nextIndex(listIndex) )
    {
        RexxActivity *activity = (RexxActivity *)allActivities->getValue(listIndex);
        if (activity->isThread(threadId))
        {
            return activity;
        }
    }
    return OREF_NULL;
}


RexxActivity *ActivityManager::findActivity()
/******************************************************************************/
/* Function:  Locate the activity associated with a thread                    */
/******************************************************************************/
{
    return findActivity(SysQueryThreadID());
}


void ActivityManager::exit(int retcode)
/******************************************************************************/
/* Function:  Really shut down--this exits the process                        */
/******************************************************************************/
{
   ::exit(retcode);
}

void ActivityManager::lockKernel()
/******************************************************************************/
/* Function:  Request access to the kernel                                    */
/******************************************************************************/
{
    MTXRQ(kernelSemaphore);            /* just request the semaphore        */
}

void ActivityManager::unlockKernel()
/******************************************************************************/
/* Function:  Release the kernel access                                       */
/******************************************************************************/
{
    currentActivity = OREF_NULL;         /* no current activation             */
    MTXRL(kernelSemaphore);             /* release the kernel semaphore      */
}

/**
 * Create the global kernel lock for the ActivityManager.
 */
void ActivityManager::createKernelLock()
{
    MTXCROPEN(kernelSemaphore, "OBJREXXKERNELSEM");
}

/**
 * Create the global kernel lock for the ActivityManager.
 */
void ActivityManager::closeKernelLock()
{
    MTXCL(kernelSemaphore);
}


/**
 * Try a fast request for the kernel.  This requires A) there
 * be no waiting activities and B) that it be possible to request
 * the semaphore without waiting.
 *
 * @return true if the semaphore was obtained, false if the kernel is
 *         not locked by this activity.
 */
bool ActivityManager::lockKernelImmediate()
{
    if (firstWaitingActivity == OREF_NULL)
    {
        return MTXRI(kernelSemaphore) == 0;
    }
    // don't give this up if somebody is waiting
    return false;
}


/**
 * Return an activity to the activity pool.
 *
 * @param activityObject
 *               The released activity.
 */
void ActivityManager::returnActivity(RexxActivity *activityObject)
/******************************************************************************/
/* Function:  Return access to an activity previously obtained from           */
/*            getActivity().  This will handle activity nesting and also      */
/*            release the kernel semaphore.                                   */
/******************************************************************************/
{
    // tell the activity he's done working at this level
    activityObject->deactivate();

    // is this thread really terminating?
    if (activityObject->isInactive())
    {
        // was that our last working activity for this interpreter invocation?
        if (activeActivities->items() == 1)
        {
            // This activity is currently the current activity.  We're going to run the
            // uninits on this one, so reactivate it until we're done running
            activityObject->activate();
            // before we update of the data structures, make sure we process any
            // pending uninit activity.
            memoryObject.forceUninits();
            // ok, deactivate this again.
            activityObject->deactivate();
        }

        // START OF CRITICAL SECTION
        {
            ResourceSection lock;
            // remove this from the active list
            activeActivities->removeItem((RexxObject *)activityObject);
            // and also remove from the global list
            allActivities->removeItem((RexxObject *)activityObject);
            // cleanup any system resources this activity might own
            activityObject->terminateActivity();
        }
        // END OF CRITICAL SECTION
                                         /* Are we terminating?               */
        if (processTerminating)
        {
            checkShutdown();                /* time to shut things down          */
        }
    }

    // this activity owned the kernel semaphore before entering here...release it
    // now.
    activityObject->releaseAccess();
}


/**
 * Return an activity to the activity pool.
 *
 * @param activityObject
 *               The released activity.
 */
void ActivityManager::activityEnded(RexxActivity *activityObject)
{
    // was that our last working activity for this interpreter invocation?
    if (activeActivities->items() == 1)
    {
        // This activity is currently the current activity.  We're going to run the
        // uninits on this one, so reactivate it until we're done running
        activityObject->activate();
        // before we update of the data structures, make sure we process any
        // pending uninit activity.
        memoryObject.forceUninits();
        // ok, deactivate this again.
        activityObject->deactivate();
    }

    // START OF CRITICAL SECTION
    {
        ResourceSection lock;       // this is a critical section
        // remove this from the active list
        activeActivities->removeItem((RexxObject *)activityObject);
        // and also remove from the global list
        allActivities->removeItem((RexxObject *)activityObject);
        // cleanup any system resources this activity might own
        activityObject->terminateActivity();
    }

    // END OF CRITICAL SECTION
                                     /* Are we terminating?               */
    if (processTerminating)
    {
        checkShutdown();                /* time to shut things down          */
    }

    // this activity owned the kernel semaphore before entering here...release it
    // now.
    activityObject->releaseAccess();
}


RexxActivity *ActivityManager::getActivity()
/******************************************************************************/
/* Function:  Determine the activity (or create a new one) for the thread     */
/*            we are about to enter kernel to send a message.  We return      */
/*            the activity object to be used to send the message.             */
/*            The routine requests kernel access on the new activity before   */
/*            returning.                                                      */
/******************************************************************************/
{
    // it's possible we already have an activity active for this thread.  That
    // most likely occurs in nested RexxStart() calls.
    RexxActivity *activityObject = findActivity();
    if (activityObject == OREF_NULL)     /* Nope, 1st time through here.      */
    {

        // we need to lock the kernel to have access to the memory manager to
        // create this activity.
        lockKernel();
                                       /* Get a new activity object.        */
        activityObject = newActivity(NO_THREAD);
        unlockKernel();                /* release kernel semaphore          */

        // now we need to have this activity become the kernel owner.
        activityObject->requestAccess();
    }
    else
    {
                                       /* Activity already existed for this */
                                       /* get kernel semophore in activity  */
        activityObject->requestAccess();

        ResourceSection lock;          // lock the resources from this point

        // this might be a recursive reentry on the same thread...if not, we
        // need to reactivate this thread.
        if (!activeActivities->hasItem((RexxObject *)activityObject))
        {
            // add this to the active list
            activeActivities->append((RexxObject *)activityObject);
        }
    }
    activityObject->activate();        // let the activity know it's in use, potentially nested
    // belt-and-braces.  Make sure the current activity is explicitly set to
    // this activity before leaving.
    currentActivity = activityObject;
    return activityObject;             // Return the activity for thread
}


/**
 * Switch the active activity if there are other activities
 * waiting to run.
 *
 * @param activity The current active activity.
 */
void ActivityManager::relinquish(RexxActivity *activity)
{
    if (firstWaitingActivity != OREF_NULL)
    {
                                         /* now join the line                 */
        addWaitingActivity(activity, true);
    }
    SysRelinquish();                     /* now allow system stuff to run     */
}


/**
 * Perform activity manager starupt processing.
 */
void ActivityManager::startup()
{
    // if we have a local server created already, don't recurse.
    if (localServer != OREF_NULL)
    {
        return;
    }

    getActivity();                       /* get an activity set up            */
                                         /* get the local environment         */
                                         /* get the server class              */
    RexxObject *server_class = env_find(new_string("!SERVER"));
                                         /* create a new server object        */
    currentActivity->messageSend(server_class, OREF_NEW, 0, OREF_NULL, &localServer);
                                         /* now release this activity         */
    returnActivity();
}
