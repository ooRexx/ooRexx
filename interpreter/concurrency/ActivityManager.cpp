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

#include "RexxCore.h"
#include "RexxActivity.hpp"
#include "RexxActivation.hpp"
#include "DirectoryClass.hpp"
#include "ActivityManager.hpp"
#include "Interpreter.hpp"
#include "ProtectedObject.hpp"
#include "InterpreterInstance.hpp"
#include "RexxNativeActivation.hpp"
#include "SysActivity.hpp"

// The currently active activity.
RexxActivity * volatile ActivityManager::currentActivity = OREF_NULL;

// this is a volatile variable used to ensure instruction ordering
volatile bool ActivityManager::sentinel = false;

// available activities we can reuse
RexxList *ActivityManager::availableActivities = OREF_NULL;

// table of all activities
RexxList *ActivityManager::allActivities = OREF_NULL;

// the activation Cache
RexxStack *ActivityManager::activations = OREF_NULL;

// size of the activation cache
size_t ActivityManager::activationCacheSize = 0;

// the native activation Cache
RexxStack *ActivityManager::nativeActivations = OREF_NULL;

// size of the activation cache
size_t ActivityManager::nativeActivationCacheSize = 0;

// this is the head of the waiting activity chain
RexxActivity * volatile ActivityManager::firstWaitingActivity = OREF_NULL;

// tail of the waiting activity chain
RexxActivity * volatile ActivityManager::lastWaitingActivity = OREF_NULL;

// process shutting down flag
bool ActivityManager::processTerminating = false;

// number of active interpreter instances in this process
size_t ActivityManager::interpreterInstances = 0;

// global lock for the interpreter
SysMutex ActivityManager::kernelSemaphore;

// the termination complete semaphore
SysSemaphore ActivityManager::terminationSem;

const size_t ACTIVATION_CACHE_SIZE = 10;

// we tend not to go as deep with nested native activations, so
// this number does not need to be the same as as the activation
// cache
const size_t NATIVE_ACTIVATION_CACHE_SIZE = 5;


/**
 * Initialize the activity manager when the interpreter starts up.
 */
void ActivityManager::init()
{
    availableActivities = new_list();
    allActivities = new_list();
    activations = new (ACTIVATION_CACHE_SIZE, false) RexxStack(ACTIVATION_CACHE_SIZE);

    // now initialize both activation caches with empty references.  This generally
    // keeps the cached items from fragmenting the object heap because they are
    // longer lived.
    size_t i;
    for (i = 0; i < ACTIVATION_CACHE_SIZE; i++)
    {
        // push a dummy activation on the stack
        activations->push((RexxObject *)new RexxActivation());
    }
    activationCacheSize = ACTIVATION_CACHE_SIZE;

    nativeActivations = new (NATIVE_ACTIVATION_CACHE_SIZE, false) RexxStack(NATIVE_ACTIVATION_CACHE_SIZE);
    for (i = 0; i < NATIVE_ACTIVATION_CACHE_SIZE; i++)
    {
        // push a dummy activation on the stack
        nativeActivations->push((RexxObject *)new RexxNativeActivation());
    }
    nativeActivationCacheSize = NATIVE_ACTIVATION_CACHE_SIZE;

    currentActivity = OREF_NULL;
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
  memory_mark(availableActivities);
  memory_mark(allActivities);
  memory_mark(activations);
  memory_mark(nativeActivations);
  memory_mark(firstWaitingActivity);
  memory_mark(lastWaitingActivity);
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
      memory_mark_general(availableActivities);
      memory_mark_general(allActivities);
      memory_mark_general(activations);
      memory_mark_general(nativeActivations);
      memory_mark_general(firstWaitingActivity);
      memory_mark_general(lastWaitingActivity);
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

    sentinel = true;
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
        // this will ensure these are set before the lock is released
        sentinel = false;
        lock.release();                  // release the lock now
    }
    else
    {                                    /* move to the end of the line       */
                                         /* chain off of the existing one     */
        lastWaitingActivity->setNextWaitingActivity(waitingAct);
        /* this is the new last one          */
        lastWaitingActivity = waitingAct;
        sentinel = false;                  // another synchronization point
        waitingAct->clearWait();           /* clear the run semaphore           */
        sentinel = true;
        lock.release();                    // release the lock now
        sentinel = false;
        if (release)                       /* current semaphore owner?          */
        {
            unlockKernel();
        }
        SysActivity::yield();            /* yield the thread                  */
        SysActivity::relinquish();       /* now allow system stuff to run     */
        waitingAct->waitKernel();        /* and wait for permission           */
    }
    sentinel = true;
    lockKernel();                        // get the kernel lock now
    sentinel = false;
    lock.reacquire();                    // get the resource lock back
                                         /* NOTE:  The following assignments  */
                                         /* do not use OrefSet intentionally. */
                                         /* because we do have yet have kernel*/
                                         /* access, we can't allow memory to  */
                                         /* allocate a new counter object for */
                                         /* this.  This leads to memory       */
                                         /* corruption and unpredictable traps*/
                                         /* dechain the activity              */

    sentinel = false;                    // another memory barrier

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
    sentinel = true;
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
    // the setting of the sentinel variables acts as a memory barrier to
    // ensure that the assignment of currentActivitiy occurs precisely at this point.
    sentinel = false;
    currentActivity = waitingAct;        /* set new current activity          */
    sentinel = true;
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
                                       /* Make sure we wake up server       */
                                       /* Make sure all free Activities     */
                                       /*  get the terminate message        */
                                       /* done after uninit calls. incas    */
                                       /*  uninits needed some.             */
    clearActivityPool();
}


/**
 * Create a new activation for a toplevel activation using a
 * routine (vs. a method invocation).
 *
 * @param activity The activity we're running on.
 * @param routine  The routine object we're calling.
 * @param code     The code object associated with the method.
 * @param calltype The type of call being made.
 * @param environment
 *                 The initial address environment.
 * @param context  The context of the invocation.
 *
 * @return The newly created activation.
 */
RexxActivation *ActivityManager::newActivation(RexxActivity *activity, RoutineClass *routine, RexxCode *code, RexxString *calltype, RexxString *environment, int context)
{

    if (activationCacheSize != 0)  /* have a cached entry?              */
    {
        activationCacheSize--;       /* remove an entry from the count    */
                                           /* get the top cached entry          */
        RexxActivation *resultActivation = (RexxActivation *)activations->stackTop();
        /* reactivate this                   */
        resultActivation->setHasReferences();
        resultActivation = new (resultActivation) RexxActivation(activity, routine, code, calltype, environment, context);
        activations->pop();          /* Remove reused activation from stac*/
        return resultActivation;

    }
    else                                 /* need to create a new one          */
    {
        /* Create new Activation.            */
        return new RexxActivation(activity, routine, code, calltype, environment, context);
    }
}


/**
 * Create a new activation for an internal level call
 * (internal call or interpreted execution).
 *
 * @param activity The activity we're running on.
 * @param parent   The parent activation.  OREF_NULL is used if this is a top-level
 *                 call.
 * @param code     The code object associated with the method.
 * @param context  The context of the invocation.
 *
 * @return The newly created activation.
 */
RexxActivation *ActivityManager::newActivation(RexxActivity *activity, RexxActivation *parent, RexxCode *code, int context)
{

    if (activationCacheSize != 0)  /* have a cached entry?              */
    {
        activationCacheSize--;       /* remove an entry from the count    */
                                           /* get the top cached entry          */
        RexxActivation *resultActivation = (RexxActivation *)activations->stackTop();
        /* reactivate this                   */
        resultActivation->setHasReferences();
        resultActivation = new (resultActivation) RexxActivation(activity, parent, code, context);
        activations->pop();          /* Remove reused activation from stac*/
        return resultActivation;

    }
    else                                 /* need to create a new one          */
    {
        /* Create new Activation.            */
        return new RexxActivation(activity, parent, code, context);
    }
}


/**
 * Create a new activation for a method invocation (vs. a
 * program or routine call)
 *
 * @param activity The activity we're running on.
 * @param method   The method object being invoked.
 * @param code     The code object associated with the method.
 *
 * @return The newly created activation.
 */
RexxActivation *ActivityManager::newActivation(RexxActivity *activity, RexxMethod *method, RexxCode *code)
{

    if (activationCacheSize != 0)  /* have a cached entry?              */
    {
        activationCacheSize--;       /* remove an entry from the count    */
                                           /* get the top cached entry          */
        RexxActivation *resultActivation = (RexxActivation *)activations->stackTop();
        /* reactivate this                   */
        resultActivation->setHasReferences();
        resultActivation = new (resultActivation) RexxActivation(activity, method, code);
        activations->pop();          /* Remove reused activation from stac*/
        return resultActivation;

    }
    else                                 /* need to create a new one          */
    {
        /* Create new Activation.            */
        return new RexxActivation(activity, method, code);
    }
}


/**
 * Create a new activation for a a native external call stack
 *
 * @param activity The activity we're running on.
 * @param parent   The parent activation.
 *
 * @return The newly created activation.
 */
RexxNativeActivation *ActivityManager::newNativeActivation(RexxActivity *activity, RexxActivation *parent)
{
    if (nativeActivationCacheSize != 0)  /* have a cached entry?              */
    {
        nativeActivationCacheSize--;       /* remove an entry from the count    */
                                           /* get the top cached entry          */
        RexxNativeActivation *resultActivation = (RexxNativeActivation *)nativeActivations->stackTop();
        /* reactivate this                   */
        resultActivation->setHasReferences();
        resultActivation = new (resultActivation) RexxNativeActivation(activity, parent);
        nativeActivations->pop();          /* Remove reused activation from stac*/
        return resultActivation;

    }
    else                                 /* need to create a new one          */
    {
        /* Create new Activation.            */
        return new RexxNativeActivation(activity, parent);
    }
}


/**
 * Create a new activation for a a native external call stack
 *
 * @param activity The activity we're running on.
 *
 * @return The newly created activation.
 */
RexxNativeActivation *ActivityManager::newNativeActivation(RexxActivity *activity)
{
    if (nativeActivationCacheSize != 0)  /* have a cached entry?              */
    {
        nativeActivationCacheSize--;       /* remove an entry from the count    */
                                           /* get the top cached entry          */
        RexxNativeActivation *resultActivation = (RexxNativeActivation *)nativeActivations->stackTop();
        /* reactivate this                   */
        resultActivation->setHasReferences();
        resultActivation = new (resultActivation) RexxNativeActivation(activity);
        nativeActivations->pop();          /* Remove reused activation from stac*/
        return resultActivation;

    }
    else                                 /* need to create a new one          */
    {
        /* Create new Activation.            */
        return new RexxNativeActivation(activity);
    }
}


/**
 * Add a popped activation back to the cache.
 *
 * @param activation The activation to add.
 */
void ActivityManager::cacheActivation(RexxActivationBase *activation)
{

    // if this is not a reply operation and the frame we just removed is
    // a Rexx activation, we can just cache this.
    if (isOfClass(Activation, activation))
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
    else
    {
        /* still room in the cache?          */
        if (nativeActivationCacheSize < NATIVE_ACTIVATION_CACHE_SIZE)
        {
            /* free everything for reclamation   */
            activation->setHasNoReferences();
            nativeActivationCacheSize++;       /* add the this to the count         */
                                               /* and add the activation            */
            nativeActivations->push((RexxObject *)activation);
        }

    }
}


/**
 * Obtain a new activity for running on a separate thread.
 *
 * @return The created (or pooled) activity object.
 */
RexxActivity *ActivityManager::createNewActivity()
{
    ResourceSection lock;                // lock the control information
        /* try to get one from the free table*/
    RexxActivity *activity =  (RexxActivity *)availableActivities->removeFirstItem();
    if (activity == OREF_NULL)
    {
        lock.release();                    // release lock while creating new activity
                                           /* Create a new activity object      */
        activity = new RexxActivity(true);
        lock.reacquire();                  // need this back again
                                           /* Add this activity to the table of */
                                           /* in use activities and the global  */
                                           /* table                             */
        allActivities->append((RexxObject *)activity);
    }
    else
    {
        /* We are able to reuse an activity, */
        /*  so just re-initialize it.        */
        activity->reset();
    }
    return activity;                     /* return the activity               */
}


/**
 * Create an activity object for the current thread.
 *
 * @return
 */
RexxActivity *ActivityManager::createCurrentActivity()
{
    // create an activity object without creating a new thread
    RexxActivity *activity = new RexxActivity(false);
    ResourceSection lock;                // lock the control information
                                       /* Add this activity to the table of */
                                       /* in use activities and the global  */
                                       /* table                             */
    allActivities->append((RexxObject *)activity);
    return activity;                     /* return the activity               */
}


/**
 * Clone off an activity from an existing activity.  Used for
 * message start() are early reply operations.
 *
 * @param parent The currently running activity.  The activity-specific settings are
 *               inherited from the parent.
 *
 * @return A new activity.
 */
RexxActivity *ActivityManager::createNewActivity(RexxActivity *parent)
{
    // create a new activity with the same priority as the parent
    RexxActivity *activity = createNewActivity();
    // copy any needed settings from the parent
    activity->inheritSettings(parent);
    return activity;
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
        activity->terminatePoolActivity();
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
    // are we shutting down or have too many threads in the pool?
    if (processTerminating || availableActivities->items() > MAX_THREAD_POOL_SIZE)
    {
        // have the activity clean up its resources.
        activity->cleanupActivityResources();

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
    // this is a critical section
    ResourceSection lock;

    // NB:  New activities are pushed on to the end, so it's prudent to search
    // from the list end toward the front of the list.  Also, this ensures we
    // will find the toplevel activity nested on a given thread first.
    for (size_t listIndex = allActivities->lastIndex();
         listIndex != LIST_END;
         listIndex = allActivities->previousIndex(listIndex) )
    {
        RexxActivity *activity = (RexxActivity *)allActivities->getValue(listIndex);
        // this should never happen, but we never return suspended threads
        if (activity->isThread(threadId) && !activity->isSuspended())
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
    return findActivity(SysActivity::queryThreadID());
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
    kernelSemaphore.request();   /* just request the semaphore        */
}

void ActivityManager::unlockKernel()
/******************************************************************************/
/* Function:  Release the kernel access                                       */
/******************************************************************************/
{
    // the use of the sentinel variables will ensure that the assignment of
    // current activity occurs BEFORE the kernel semaphore is released.
    sentinel = false;
    currentActivity = OREF_NULL;         /* no current activation             */
    sentinel = true;
    kernelSemaphore.release();           /* release the kernel semaphore      */
}

/**
 * Create the global kernel lock for the ActivityManager.
 */
void ActivityManager::createLocks()
{
    kernelSemaphore.create();
    // this needs to be created and set
    terminationSem.create();
    terminationSem.reset();
}

/**
 * Create the global kernel lock for the ActivityManager.
 */
void ActivityManager::closeLocks()
{
    kernelSemaphore.close();
    terminationSem.close();
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
        return kernelSemaphore.requestImmediate();
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
    // START OF CRITICAL SECTION
    {
        ResourceSection lock;
        // and also remove from the global list
        allActivities->removeItem((RexxObject *)activityObject);
        // if we ended up pushing an old activity down when we attached this
        // thread, then we need to restore the old thread to active state.
        RexxActivity *oldActivity = activityObject->getNestedActivity();
        if (oldActivity != OREF_NULL)
        {
            oldActivity->setSuspended(false);
        }
        // cleanup any system resources this activity might own
        activityObject->cleanupActivityResources();
    }
}


/**
 * Return an activity to the activity pool.
 *
 * @param activityObject
 *               The released activity.
 */
void ActivityManager::activityEnded(RexxActivity *activityObject)
{
    // START OF CRITICAL SECTION
    {
        ResourceSection lock;       // this is a critical section
        // and also remove from the global list
        allActivities->removeItem((RexxObject *)activityObject);
        // cleanup any system resources this activity might own
        activityObject->cleanupActivityResources();
                                         /* Are we terminating?               */
        if (processTerminating && allActivities->items() == 0)
        {
            // notify any waiters that we're clear
            postTermination();
        }
    }
}


/**
 * Get a root activity for a new interpreter instance.
 *
 * @return The newly created activity.
 */
RexxActivity *ActivityManager::getRootActivity()
{
    // it's possible we already have an activity active for this thread.  That
    // most likely occurs in nested RexxStart() calls.  Get that activity first,
    // and if we have one, we'll need to push this down.
    RexxActivity *oldActivity = findActivity();

    // we need to lock the kernel to have access to the memory manager to
    // create this activity.
    lockKernel();
                                   /* Get a new activity object.        */
    RexxActivity *activityObject = createCurrentActivity();
    unlockKernel();                /* release kernel semaphore          */
    // mark this as the root activity for an interpreter instance.  Some operations
    // are only permitted from the root threads.
    activityObject->setInterpreterRoot();

    // Do we have a nested interpreter call occurring on the same thread?  We need to
    // mark the old activity as suspended, and chain this to the new activity.
    if (oldActivity != OREF_NULL)
    {
        oldActivity->setSuspended(true);
        // this pushes this down the stack.
        activityObject->setNestedActivity(oldActivity);
    }

    // now we need to have this activity become the kernel owner.
    activityObject->requestAccess();
    // this will help ensure that the code after the request access call
    // is only executed after access acquired.
    sentinel = true;

    activityObject->activate();        // let the activity know it's in use, potentially nested
    // belt-and-braces.  Make sure the current activity is explicitly set to
    // this activity before leaving.
    currentActivity = activityObject;
    return activityObject;
}


/**
 * return a root activity when an interpreter instance
 * terminates.
 */
void ActivityManager::returnRootActivity(RexxActivity *activity)
{
    // detach this from the instance.  This will also reactivate
    // and nested activity that's been pushed down.
    activity->detachInstance();
    // make sure we release any system resources used by this activity, such as the semaphores
    activity->cleanupActivityResources();

    ResourceSection lock;                // need the control block locks
    // remove this from the activity list so it will never get
    // picked up again.
    allActivities->removeItem((RexxObject *)activity);
}


/**
 * Attach an activity to an interpreter instance
 *
 * @param instance The interpreter instance involved.
 *
 * @return Either an existing activity, or a new activity created for
 *         this thread.
 */
RexxActivity *ActivityManager::attachThread()
{
    // it's possible we already have an activity active for this thread.  That
    // most likely occurs in nested RexxStart() calls.
    RexxActivity *oldActivity = findActivity();
    // we have an activity created for this thread already.  The interpreter instance
    // should already have handled the case of an attach for an already attached thread.
    // so we're going to have a new activity to create, and potentially an existing one to
    // suspend
    // we need to lock the kernel to have access to the memory manager to
    // create this activity.
    lockKernel();
    RexxActivity *activityObject = createCurrentActivity();
    // Do we have a nested interpreter call occurring on the same thread?  We need to
    // mark the old activity as suspended, and chain this to the new activity.
    if (oldActivity != OREF_NULL)
    {
        oldActivity->setSuspended(true);
        // this pushes this down the stack.
        activityObject->setNestedActivity(oldActivity);
    }

    unlockKernel();                /* release kernel semaphore          */

    // now we need to have this activity become the kernel owner.
    activityObject->requestAccess();
    // this will help ensure that the code after the request access call
    // is only executed after access acquired.
    sentinel = true;
    // belt-and-braces.  Make sure the current activity is explicitly set to
    // this activity before leaving.
    currentActivity = activityObject;
    return activityObject;
}


/**
 * Get an already existing activity for the current thread and
 * give it kernel access before returning.  This will fail if
 * the thread has not been properly attached.
 *
 * @return The activity for this thread.
 */
RexxActivity *ActivityManager::getActivity()
{
    // it's possible we already have an activity active for this thread.  That
    // most likely occurs in nested RexxStart() calls.
    RexxActivity *activityObject = findActivity();
    if (activityObject == OREF_NULL)     /* Nope, 1st time through here.      */
    {
        // this is an error....not sure how to handle this.
        return OREF_NULL;
    }
    // go acquire the kernel lock and take care of nesting
    activityObject->enterCurrentThread();
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
        addWaitingActivity(activity, true);
    }
}


/**
 * Retrieve a variable from the current local environment
 * object.
 *
 * @param name   The name of the environment variable.
 *
 * @return The object stored in .local at the requested name.
 */
RexxObject *ActivityManager::getLocalEnvironment(RexxString *name)
{
    if (currentActivity == OREF_NULL)
    {
        return TheNilObject;
    }
    return currentActivity->getLocalEnvironment(name);
}


/**
 * Retrieve the current .local directory instance.
 *
 * @return The .local directory for the current activity.
 */
RexxDirectory *ActivityManager::getLocal()
{
    if (currentActivity == OREF_NULL)
    {
        return OREF_NULL;
    }
    return currentActivity->getLocal();
}


/**
 * Enter a native context block.  This will locate the appropriate
 * activity for this callback and acquire kernel access on that
 * activity.  If this thread has never been used, then a new
 * interpreter instance will be created and the thread attached
 * to that instance.
 */
NativeContextBlock::NativeContextBlock()
{
    // default to no instance
    instance = OREF_NULL;
    activity = ActivityManager::getActivity();
    // if not reentering on an existing thread, we create a new instance
    // temporarily to service this request.  Many functions will
    // not make sense called this way.
    if (activity == OREF_NULL)
    {
        // Get an instance.  This also gives the root activity of the instance
        // the kernel lock.
        instance = Interpreter::createInterpreterInstance();
        activity = instance->getRootActivity();

    }
    self = (RexxNativeActivation *)activity->getTopStackFrame();
}


/**
 * Release the kernal access and cleanup when the context block
 * goes out of scope.
 */
NativeContextBlock::~NativeContextBlock()
{
    activity->exitCurrentThread();
    if (instance != OREF_NULL)
    {
        // terminate the instance
        instance->terminate();
    }
}


/**
 * Protect an object that associated with the current native
 * context.  This creates a local reference that will lock
 * the object into memory until the native activation is
 * popped off the stack.
 *
 * @param o      The object to protect (can be null).
 *
 * @return The protected object.
 */
RexxObject *NativeContextBlock::protect(RexxObject *o)
{
    self->createLocalReference(o);
    return o;
}

