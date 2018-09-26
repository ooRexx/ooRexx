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

#include "RexxCore.h"
#include "Activity.hpp"
#include "RexxActivation.hpp"
#include "DirectoryClass.hpp"
#include "ActivityManager.hpp"
#include "Interpreter.hpp"
#include "ProtectedObject.hpp"
#include "InterpreterInstance.hpp"
#include "NativeActivation.hpp"
#include "SysActivity.hpp"
#include "QueueClass.hpp"

// The currently active activity.
Activity *volatile ActivityManager::currentActivity = OREF_NULL;

// this is a volatile variable used to ensure instruction ordering
volatile bool ActivityManager::sentinel = false;

// available activities we can reuse
QueueClass *ActivityManager::availableActivities = OREF_NULL;

// table of all activities
QueueClass *ActivityManager::allActivities = OREF_NULL;

std::deque<Activity *>ActivityManager::waitingActivities;   // queue of waiting activities

size_t ActivityManager::waitingAttaches = 0;                // count of waiting external attaches

uint64_t ActivityManager::lastLockTime = 0;                 // the last time we granted the kernel lock.

// process shutting down flag
bool ActivityManager::processTerminating = false;

// number of active interpreter instances in this process
size_t ActivityManager::interpreterInstances = 0;

// global lock for the interpreter
SysMutex ActivityManager::kernelSemaphore;

// the termination complete semaphore
SysSemaphore ActivityManager::terminationSem;


/**
 * Initialize the activity manager when the interpreter starts up.
 */
void ActivityManager::init()
{
    availableActivities = new_queue();
    allActivities = new_queue();
    currentActivity = OREF_NULL;
}


/**
 * Live marking of the objects owned by the activity manager.
 *
 * @param liveMark The current live mark value.
 */
void ActivityManager::live(size_t liveMark)
{
    memory_mark(availableActivities);
    memory_mark(allActivities);
}

/**
 * Generalized marking of activity manager owned objects.
 *
 * @param reason The marking reason.
 */
void ActivityManager::liveGeneral(MarkReason reason)
{
    // none of these get included in the saved image.  The activity
    // manager shouldn't even be getting marked durin the save, but
    // we make sure for safety.
    if (reason != SAVINGIMAGE)
    {
        memory_mark_general(availableActivities);
        memory_mark_general(allActivities);
    }
}


/**
 * Add a waiting activity to the waiting queue.
 *
 * @param waitingAct The activity to queue up.
 * @param release    If true, the kernel lock should be released on completion.
 */
void ActivityManager::addWaitingActivity(Activity *waitingAct, bool release )
{
    DispatchSection lock;                // need the dispatch queue lock

    bool inWaitQueue = false;            // used to see if we're waiting for permission to request the kernel lock

    // nobody waiting yet?  If the release flag is true, we already have the
    // kernel lock, but nobody is waiting.
    if (!hasWaiters())
    {
        // we're done if we already have the lock and the queue is empty.
        if (release)
        {
            return;
        }

        // we to add ourselves to the queue so that the current lock holder will
        // know that there is an activity waiting on the lock, but we will go
        // immediately to requesting the semaphore to avoid a race condition that
        // can cause the dispatch semaphore to not get dispatched and leave the
        // thread hanging.
        waitingActivities.push_front(waitingAct);

        // belt-and-braces...this tells us now to wait for a dispatch event.
        inWaitQueue = false;
    }
    // if there is something in the queue, remove it and give it permission to run before
    // we do our stuff.
    else
    {
        // if there are pending attaches, they are already waiting on the
        // kernel lock but don't have an activity yet. We only wake up
        // other activies that are waiting if there are no attaches pending.

        // make sure we do this test BEFORE we add ourselves to the list
        if (waitingAttaches == 0 && !waitingActivities.empty())
        {
            // if there's something else in the queue, then post the run semaphore of
            // the head element so that it wakes up next and starts waiting on the
            // run semaphore
            waitingActivities.front()->postDispatch();
        }

        // we got here because a) we could not obtain the kernel lock immediately, so
        // we need to wait for the current owning thread to give it up, and b) there was already
        // something in the queue so we have to go stand in line.

        // add to the end
        waitingActivities.push_back(waitingAct);

        // set the flag to indicate a dispatch wait is necessary
        inWaitQueue = true;

        // we're going to wait until posted, so make sure the run semaphore is cleared
        waitingAct->clearRunWait();
    }
    lock.release();                    // release the lock now
    // if we are the current kernel semaphore owner, time to release this
    // so other waiters can
    if (release)
    {
        unlockKernel();
    }
    SysActivity::yield();            // yield the thread

    // if we're in line behind another activity, then wait to get permission to
    // request the kernel lock.
    if (inWaitQueue)
    {
        waitingAct->waitForDispatch();   // wait for this thread to get dispatched again
    }

    sentinel = true;
    // in theory, we should now be the only activity waiting on the kernel lock
    waitingAct->waitForKernel();         // perform the kernel lock
    // belt and braces.  it is possible the dispatcher was
    // reentered on the same thread, in which case we have an
    // earlier stack frame waiting on the same semaphore.  Clear it so it
    // get get reposted later.
    waitingAct->clearRunWait();
    sentinel = false;
    lock.reacquire();                    // get the resource lock back
    sentinel = false;                    // another memory barrier

    // ok, we have the kernel lock and the resource lock. In theory, we should be
    // at the front of the waitingActivities list, but there are situations where
    // entries might be out of order. Search to find our entry and remove it.
    if (!waitingActivities.empty())
    {
        // search for the activity position and remove it.
        for (std::deque<Activity *>::iterator it = waitingActivities.begin(); it != waitingActivities.end(); ++it)
        {
            // if this is found, remove it from the queue
            if (*it == waitingAct)
            {
                waitingActivities.erase(it);
                break;
            }
        }
        // ignore this if not found.
    }

    sentinel = true;
    // the setting of the sentinel variables acts as a memory barrier to
    // ensure that the assignment of currentActivitiy occurs precisely at this point.
    sentinel = false;
    currentActivity = waitingAct;
    sentinel = true;
    // set the new active numeric settings
    Numerics::setCurrentSettings(waitingAct->getNumericSettings());
}


/**
 * return a nested waiting activity to the waiting queue.
 *
 * @param waitingAct The activity to queue up.
 */
void ActivityManager::returnWaitingActivity(Activity *waitingAct)
{
    DispatchSection lock;                // need the dispatch queue lock

    // nobody waiting yet?  Unusual, but we might be the only one waiting
    // dispatch at this time. Push it back on the queue and post the semaphore
    // so that this will wakup up when we rewind back to it.
    if (waitingActivities.empty())
    {
        // add to the front of the queue so others know someone is waiting.
        waitingActivities.push_front(waitingAct);
        // since this is at the front of the queue now, we want to ensure it
        // wakes up once we get back to the semaphore wait call.
        waitingAct->postDispatch();
    }
    else
    {
        // This is sort of tricky. At the time the activity was removed from
        // the queue, it could have been in one of three states.
        //
        // 1) In the queue, but not waiting on for run permission.
        // 2) In the queue and waiting for run permission.
        // 3) In the queue and waiting for the kernel lock
        //
        // For situations 2) & 3), once the thread wakes up, it removes the front
        // entry when it wakes up (if it is the front entry). Since the thread is
        // already waiting, it might jump ahead in the line and continue without
        // removing the entry. This might cause thread stalls if its run semaphore
        // is posted while it is running. We'll add it to the end, clear the semaphore
        // and let normal dispatch take over. If this thread is already past the run permission
        // stage, it will need to search the queue to remove its entry.
        waitingActivities.push_back(waitingAct);
        // others are waiting, but post our run sem now so that we'll block on the
        // kernel lock, not waiting for dispatch.
        waitingAct->clearRunWait();
    }
}


/**
 * Terminate an interpreter instance.  This starts process
 * shutdown if the last instance goes away.
 */
void ActivityManager::createInterpreter()
{
    interpreterInstances++;
}

/**
 * Terminate an interpreter instance.  This starts process
 * shutdown if the last instance goes away.
 */
void ActivityManager::terminateInterpreter()
{
    ResourceSection lock;

    // if this is the last interpreter instance, then shutdown
    // the entire environment.
    interpreterInstances--;
    if (interpreterInstances == 0)
    {
        shutdown();
    }
}


/**
 * Shutdown the activity manager and initiate interpreter termination.
 */
void ActivityManager::shutdown()
{
    processTerminating = true;

    // Go clean up all of the pooled activities.
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
RexxActivation *ActivityManager::newActivation(Activity *activity, RoutineClass *routine, RexxCode *code, RexxString *calltype, RexxString *environment, ActivationContext context)
{
    // in heavily multithreaded environments, the activation cache is a source for race conditions
    // that can lead to crashes.  Just unconditionally create a new actvation
    return new RexxActivation(activity, routine, code, calltype, environment, context);
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
RexxActivation *ActivityManager::newActivation(Activity *activity, RexxActivation *parent, RexxCode *code, ActivationContext context)
{
    // in heavily multithreaded environments, the activation cache is a source for race conditions
    // that can lead to crashes.  Just unconditionally create a new actvation
    return new RexxActivation(activity, parent, code, context);
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
RexxActivation *ActivityManager::newActivation(Activity *activity, MethodClass *method, RexxCode *code)
{
    // in heavily multithreaded environments, the activation cache is a source for race conditions
    // that can lead to crashes.  Just unconditionally create a new actvation
    return new RexxActivation(activity, method, code);
}


/**
 * Create a new activation for a a native external call stack
 *
 * @param activity The activity we're running on.
 * @param parent   The parent activation.
 *
 * @return The newly created activation.
 */
NativeActivation *ActivityManager::newNativeActivation(Activity *activity, RexxActivation *parent)
{
    // in heavily multithreaded environments, the activation cache is a source for race conditions
    // that can lead to crashes.  Just unconditionally create a new actvation
    return new NativeActivation(activity, parent);
}


/**
 * Create a new activation for a a native external call stack
 *
 * @param activity The activity we're running on.
 *
 * @return The newly created activation.
 */
NativeActivation *ActivityManager::newNativeActivation(Activity *activity)
{
    // in heavily multithreaded environments, the activation cache is a source for race conditions
    // that can lead to crashes.  Just unconditionally create a new actvation
    return new NativeActivation(activity);
}


/**
 * Obtain a new activity for running on a separate thread.
 *
 * @return The created (or pooled) activity object.
 */
Activity *ActivityManager::createNewActivity()
{
    ResourceSection lock;                // lock the control information
    // try to get an activity from the cache
    Activity *activity =  (Activity *)availableActivities->pull();
    if (activity == OREF_NULL)
    {
        // we release the resource lock around creating
        // a new activation
        lock.release();
        activity = new Activity(true);
        lock.reacquire();
        // add this to our table of all activities
        allActivities->append(activity);
    }
    else
    {
        // We are able to reuse an activity, so just re-initialize it.
        activity->reset();
    }
    return activity;
}


/**
 * Create an activity object for the current thread.
 *
 * @return
 */
Activity *ActivityManager::createCurrentActivity()
{
    // create an activity object without creating a new thread
    Activity *activity = new Activity(false);
    // we need the resource lock while doing this.
    ResourceSection lock;
    // add this to the activity table and return
    allActivities->append(activity);
    return activity;
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
Activity *ActivityManager::createNewActivity(Activity *parent)
{
    // create a new activity with the same priority as the parent
    Activity *activity = createNewActivity();
    // copy any needed settings from the parent
    activity->inheritSettings(parent);
    return activity;
}


/**
 * Clear the activty pool of pooled activities.
 */
void ActivityManager::clearActivityPool()
{
    Activity *activity = (Activity *)availableActivities->pull();
    while (activity != OREF_NULL)
    {
        // terminate this thread
        activity->terminatePoolActivity();
        activity = (Activity *)availableActivities->pull();
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
bool ActivityManager::poolActivity(Activity *activity)
{
    // are we shutting down or have too many threads in the pool?
    if (processTerminating || availableActivities->items() > MAX_THREAD_POOL_SIZE)
    {
        // have the activity clean up its resources.
        activity->cleanupActivityResources();

        // remove this from the activity list
        allActivities->removeItem(activity);
        return false;
    }
    else
    {
        // just add this to the available list
        availableActivities->append(activity);
        return true;   // this was successfully pooled
    }
}


/**
 * Raise a halt condition on an activity.
 *
 * @param thread_id The target thread identifier.
 * @param description
 *                  The description of the halt.
 *
 * @return Returns the halt result.  Returns false if a halt
 *         condition is already pending or the target activity
 *         is not found.
 */
bool ActivityManager::haltActivity(thread_id_t  thread_id, RexxString * description )
{
    ResourceSection lock;
    // locate the activity associated with this thread_id.  If not found, return
    // a failure.
    Activity *activity = findActivity(thread_id);
    if (activity != OREF_NULL)
    {
        return activity->halt(description);
    }
    return false;                        // this was a failure
}


/**
 * Flip on external trace for a thread.
 *
 * @param thread_id The target thread id.
 * @param on_or_off The trace setting.
 *
 * @return true if this worked, false otherwise.
 */
bool ActivityManager::setActivityTrace(thread_id_t thread_id, bool  on_or_off )
{
    ResourceSection lock;
    // locate the activity associated with this thread_id.  If not found, return
    // a failure.
    Activity *activity = findActivity(thread_id);
    if (activity != OREF_NULL)
    {
        return activity->setTrace(on_or_off);
    }
    return false;                        // this was a failure
}


/**
 * Signal an activation to yield control
 */
void ActivityManager::yieldCurrentActivity()
{
    ResourceSection lock;

    Activity *activity = ActivityManager::currentActivity;
    if (activity != OREF_NULL)
    {
        activity->yield();
    }
}


/**
 * Locate the activity associated with a thread
 *
 * @param threadId The target thread id
 *
 * @return The activity, or OREF_NULL if this is not currently in use.
 */
Activity *ActivityManager::findActivity(thread_id_t threadId)
{
    // this is a critical section
    ResourceSection lock;

    // NB:  New activities are pushed on to the end, so it's prudent to search
    // from the list end toward the front of the list.  Also, this ensures we
    // will find the toplevel activity nested on a given thread first.
    for (size_t listIndex = allActivities->lastIndex(); listIndex > 0; listIndex--)
    {
        Activity *activity = (Activity *)allActivities->get(listIndex);
        // this should never happen, but we never return suspended threads
        if (activity->isThread(threadId) && !activity->isSuspended())
        {
            return activity;
        }
    }
    return OREF_NULL;
}


/**
 * Locate the activity associated with the current thread
 *
 * @return The Activity for this thread, if it exists.
 */
Activity *ActivityManager::findActivity()
{
    return findActivity(SysActivity::queryThreadID());
}


/**
 * Really shut down--this exits the process
 *
 * @param retcode The exit return code.
 */
void ActivityManager::exit(int retcode)
{
   ::exit(retcode);
}


/**
 * release the kernel access and make sure waiting activites are woken up.
 */
void ActivityManager::releaseAccess()
{
    DispatchSection lock;                // need the dispatch queue lock

    // we're releasing the kernel, but we might need to nudge another
    // activity if someone is waiting for the lock, but only if we don't
    // have an pending attach trying to get the lock.
    if (!waitingActivities.empty() && waitingAttaches == 0)
    {
        // if there's something else in the queue, then post the run semaphore of
        // the head element so that it wakes up next and starts waiting on the
        // kernel semaphore. It might already be doing that, but posting this
        // does no harm. We do not pop the element off the list... that is done
        // once the thread wakes up.
        waitingActivities.front()->postDispatch();
    }

    // now release the kernel lock
    unlockKernel();
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
 * Cleanup the global locks for the ActivityManager.
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
    // don't give this up if we have activities in the
    // dispatch queue
    if (!hasWaiters())
    {
        return kernelSemaphore.requestImmediate();
    }
    return false;
}


/**
 * Return an activity to the activity pool.
 *
 * @param activityObject
 *               The released activity.
 */
void ActivityManager::returnActivity(Activity *activityObject)
{
    // START OF CRITICAL SECTION
    {
        ResourceSection lock;
        // remove this from the activte list
        allActivities->removeItem(activityObject);
        // if we ended up pushing an old activity down when we attached this
        // thread, then we need to restore the old thread to active state.
        Activity *oldActivity = activityObject->getNestedActivity();

        // there are two situations for a nested activity. The first is simple,
        // which is just a normal exit interpreter/reenter interpreter.
        // The second is more complicated and can (as far as we know) only
        // occur on Windows).
        if (oldActivity != OREF_NULL)
        {
            // this is the dispatch queue situation. We just return this
            // to the dispatch queue without waiting to obtain the kernel
            // lock.
            if (oldActivity->isWaitingForDispatch())
            {
                returnWaitingActivity(oldActivity);
            }
            // just remove the suspended state
            else
            {
                oldActivity->setSuspended(false);
            }
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
void ActivityManager::activityEnded(Activity *activityObject)
{
    // START OF CRITICAL SECTION
    {
        ResourceSection lock;       // this is a critical section
        // and also remove from the global list
        allActivities->removeItem(activityObject);
        // cleanup any system resources this activity might own
        activityObject->cleanupActivityResources();

        // did we just release the last activity during a shutdown?  The shutdown
        // can now complete.
        if (processTerminating && allActivities->isEmpty())
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
Activity *ActivityManager::getRootActivity()
{
    // it's possible we already have an activity active for this thread.  That
    // most likely occurs in nested RexxStart() calls.  Get that activity first,
    // and if we have one, we'll need to push this down.
    Activity *oldActivity = findActivity();

    // we need to lock the kernel to have access to the memory manager to
    // create this activity.
    lockKernel();

    // get a new activity object
    Activity *activityObject = createCurrentActivity();
    unlockKernel();
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
void ActivityManager::returnRootActivity(Activity *activity)
{
    // detach this from the instance.  This will also reactivate
    // and nested activity that's been pushed down.
    activity->detachInstance();
    // make sure we release any system resources used by this activity, such as the semaphores
    activity->cleanupActivityResources();

    ResourceSection lock;                // need the control block locks
    // remove this from the activity list so it will never get
    // picked up again.
    allActivities->removeItem(activity);
}


/**
 * Attach an activity to an interpreter instance
 *
 * @param instance The interpreter instance involved.
 *
 * @return A new activity instance created for this thread. This
 *         activity will own the kernel lock on return.
 */
Activity *ActivityManager::attachThread()
{
    // it's possible we already have an activity active for this thread.  That
    // most likely occurs in nested RexxStart() calls.
    Activity *oldActivity = findActivity();
    // we have an activity created for this thread already.  The interpreter instance
    // should already have handled the case of an attach for an already attached thread.
    // so we're going to have a new activity to create, and potentially an existing one to
    // suspend. We may also need to remove the activity from the dispatch queue if
    // it is there.

    // we need to lock the kernel to have access to the memory manager to
    // create this activity. This is an "unowned" lock, since there will not be
    // an activity connected with it.

    // try the fast version first
    if (!lockKernelImmediate())
    {
        DispatchSection lock;                // need the dispatch queue lock

        waitingAttaches++;                   // make sure we indicate that someone is waiting for this
        sentinel = true;
        lock.release();                      // release the lock now
        sentinel = false;

        lockKernel();                        // now wait for the semaphore to come free

        sentinel = true;
        lock.reacquire();                    // get the resource lock back
        sentinel = false;
        waitingAttaches--;                   // remove our request to the lock
    }

    // now that we have the lock, we can allocate an activity object
    // from the heap.
    Activity *activityObject = createCurrentActivity();
    // Do we have a nested interpreter call occurring on the same thread?  We need to
    // mark the old activity as suspended, and chain this to the new activity.
    if (oldActivity != OREF_NULL)
    {
        // if we have an activity on this thread that is waiting for dispatch,
        // then we need to make sure the old activity is removed from the
        // dispatch queue and disabled until this new activity is detached.
        if (oldActivity->isWaitingForDispatch())
        {
            // make sure this is never dispatched.
            suspendDispatch(oldActivity);
        }
        // this is a normal nested reentry, so just make it as suspended.
        else
        {
            oldActivity->setSuspended(true);
        }
        // The third state is an activity that's waiting on kernel access. We don't
        // need to do anything special here.

        // this pushes this down the stack.
        activityObject->setNestedActivity(oldActivity);
    }

    // this will help ensure that the following code is only executed after
    // everything above is executed.
    sentinel = true;

    // We own the kernel lock, but there is no current activity set. While still
    // locked, make this the current activity and return. We don't want to release
    // the lock yet because the garbage collector could get triggered on another
    // thread.
    activityObject->setupCurrentActivity();
    return activityObject;
}


/**
 * Remove an activity from the dispatch queue when a
 * an attach is superceding an activity that is in the
 * dispatch queue.
 *
 * @param activity The activity to suspend.
 */
void ActivityManager::suspendDispatch(Activity *activity)
{
    DispatchSection lock;                // need the dispatch queue lock

    // if this activity is at the front of the queue, then we need to remove
    // this and dispatch the new front element.
    if (waitingActivities.front() == activity)
    {
        // We can generally only get here if the activity is waiting on either the
        // run semaphore or the kernel semaphore already. This will not wake up again
        // until the current activity stack unwinds to the semaphore call, so
        // we remove this from the queue so it doesn't keep getting dispatched and
        // them poke the next activity in line if there is one. This will get
        // returned to the queue once the re-entry is complete.
        waitingActivities.pop_front();
        // If we have another activity in the queue and there are no
        // attaches pending, wake up the front activity
        if (!waitingActivities.empty() && waitingAttaches == 0)
        {
            // we just dispatch this. The activity will remove itself once it
            // has access
            waitingActivities.front()->postDispatch();
        }
    }
    // we're in line behind some other activity, so just remove this one
    // from the queue. We don't need to repost anything at this point.
    else
    {
        removeWaitingActivity(activity);
    }
}


/**
 * In certain situations (mostly confined to Windows),
 * we can re-enter the interpreter on a thread that is
 * currently on the dispatch queue. If that activity gets
 * dispatched, then everything blocks because the
 * activity never wakes up until the nested thread returns.
 * In that situation, we need to ensure that the original
 * waiting activity is removed from the dispatch queue
 * until it is the top activity on the stack.
 *
 * @param waitingAct The activity to remove from the queue.
 */
void ActivityManager::removeWaitingActivity(Activity *waitingAct)
{
    // iterators don't work if the collection is empty, so
    if (waitingActivities.empty())
    {
        return;
    }

    // search for the activity position and remove it.
    for (std::deque<Activity *>::iterator it = waitingActivities.begin(); it != waitingActivities.end(); ++it)
    {
        // if this is found, remove it from the queue
        if (*it == waitingAct)
        {
            waitingActivities.erase(it);
            return;
        }
    }
    // ignore this if not found.
}


/**
 * Get an already existing activity for the current thread and
 * give it kernel access before returning.  This will fail if
 * the thread has not been properly attached.
 *
 * @return The activity for this thread.
 */
Activity *ActivityManager::getActivity()
{
    // it's possible we already have an activity active for this thread.  That
    // most likely occurs in nested RexxStart() calls.
    Activity *activityObject = findActivity();
    // we generally should have something.  Somehow we have an improperly
    // attached thread.  Just return a failure indicator.
    if (activityObject == OREF_NULL)
    {
        return OREF_NULL;
    }
    // go acquire the kernel lock and take care of nesting
    activityObject->enterCurrentThread();
    return activityObject;             // Return the activity for thread
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
DirectoryClass *ActivityManager::getLocal()
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
    self = (NativeActivation *)activity->getTopStackFrame();
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

