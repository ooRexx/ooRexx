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
#ifndef Included_ActivityManager
#define Included_ActivityManager

#include "Activity.hpp"
#include "ActivationSettings.hpp"
#include <deque>
#include "GlobalNames.hpp"
#include "SystemInterpreter.hpp"

// the error code definitions are need for any code that
// issues error messages, so place these here.
#include "RexxErrorCodes.h"

class IdentityTable;
class LiveStack;
class RexxCode;
class RoutineClass;
class NativeActivation;
class QueueClass;

class ActivityManager
{
public:
    static void live(size_t);
    static void liveGeneral(MarkReason reason);

    static void addWaitingActivity(Activity *a, bool release);
    static void addWaitingApiActivity(Activity *a);
    static bool dispatchNext();
    static inline bool hasWaiters() { return waitingAccess != 0 || waitingAttaches != 0; }
    static inline bool hasApiWaiters() { return waitingApiAccess != 0; }

    static Activity *findActivity();
    static Activity *findActivity(thread_id_t);
    static Activity *getActivity();
    static void returnActivity(Activity *);
    static void activityEnded(Activity *);
    static void shutdown();
    static void checkShutdown();
    static void createInterpreter();
    static void terminateInterpreter();

    inline static void lockKernel()
    {
        kernelSemaphore.request();
        // keep track of the last time this was granted.
        lastLockTime = SysThread::getMillisecondTicks();
    }

    inline static void unlockKernel()
    {
        // the use of the sentinel variables will ensure that the assignment of
        // current activity occurs BEFORE the kernel semaphore is released.
        sentinel = false;
        currentActivity = OREF_NULL;
        sentinel = true;
        // now release the semaphore
        kernelSemaphore.release();
    }

    static void releaseAccess();
    static bool lockKernelImmediate();
    static void createLocks();
    static void closeLocks();
    static void init();
    static RexxActivation *newActivation(Activity *activity, RoutineClass *routine, RexxCode *code, RexxString *calltype, RexxString *environment, ActivationContext context);
    static RexxActivation *newActivation(Activity *activity, RexxActivation *parent, RexxCode *code, ActivationContext context);
    static RexxActivation *newActivation(Activity *activity, MethodClass *method, RexxCode *code);
    static NativeActivation *newNativeActivation(Activity *activity, RexxActivation *parent);
    static NativeActivation *newNativeActivation(Activity *activity);
    static Activity *createNewActivity();
    static Activity *createCurrentActivity();
    static Activity *createNewActivity(Activity *);
    static void haltAllActivities(RexxString *);
    static void traceAllActivities(bool on);
    static bool setActivityTrace(thread_id_t thread_id, bool on_or_off);
    static void clearActivityPool();
    static bool poolActivity(Activity *activity);
    static bool haltActivity(thread_id_t thread_id, RexxString * description);
    static void yieldCurrentActivity();
    static void exit(int retcode);
    static inline void relinquish(Activity *activity)
    {
        // if we have waiting activities, then let one of them
        // in next.
        if (hasWaiters())
        {
            addWaitingActivity(activity, true);
        }
    }
    // give up control, but only if the time slice requires it.
    static inline void relinquishIfNeeded(Activity *activity)
    {
        // if we have waiting activities, then let one of them
        // in next.
        if (hasWaiters())
        {
            // if we have API calls trying to get in, then don't bother checking the time stamp.
            if (hasApiWaiters())
            {
                addWaitingActivity(activity, true);
            }
            else
            {
                // check the time that we've have been holding the lock and release it
                // if we've crossed the threshold,
                uint64_t timeNow = SysThread::getMillisecondTicks();
                if (timeNow - lastLockTime > timeSliceLength)
                {
                    addWaitingActivity(activity, true);
                }
            }
        }
    }

    static Activity *getRootActivity();
    static void returnRootActivity(Activity *activity);
    static Activity *attachThread();
    static RexxObject *getLocalEnvironment(RexxString *name);
    static DirectoryClass *getLocal();
    static void suspendDispatch(Activity *activity);
    static void removeWaitingActivity(Activity *waitingAct);
    static void returnWaitingActivity(Activity *waitingAct);
    static void handleNestedActivity(Activity *newActivity, Activity *oldActivity);

    // non-static method that is attached to the environment directory
    DirectoryClass *getLocalRexx()
    {
        return getLocal();
    }

    static Activity * volatile currentActivity;   // the currently active thread

    static inline void postTermination()
    {
        terminationSem.post();              /* let anyone who cares know we're done*/
    }

    static inline void waitForTermination()
    {
        terminationSem.wait();              // wait until this is posted
    }

protected:

    // maximum number of activities we'll pool
    static const size_t MAX_THREAD_POOL_SIZE = 5;
    static const uint64_t timeSliceLength = 24;            // how long we'll run before checking for a control yield.

    static QueueClass       *availableActivities;     // table of available activities
    static QueueClass       *allActivities;           // table of all activities
    static bool              processTerminating;      // shutdown processing started
    static size_t            interpreterInstances;    // number of times an interpreter has been created.

    static SysMutex          kernelSemaphore;         // global kernel semaphore lock
    static SysSemaphore      terminationSem;          // used to signal that everything has shutdown
    static volatile bool sentinel;                    // used to ensure proper ordering of updates
    static std::deque<Activity *>waitingActivities;   // queue of waiting activities
    static size_t waitingAttaches;                    // the count of attaches waiting for access
    static size_t waitingAccess;                      // the count of activities waiting for access
    static size_t waitingApiAccess;                   // the count of activities waiting for access for API callbacks.
    static uint64_t          lastLockTime;            // the last time we granted the kernel lock.
};


// various exception/condition reporting routines
inline void reportCondition(RexxString *condition, RexxObject *description) { ActivityManager::currentActivity->raiseCondition(condition, OREF_NULL, description, OREF_NULL, OREF_NULL); }
inline void reportNovalue(RexxString *description) { reportCondition(GlobalNames::NOVALUE, description); }
inline void reportNostring(RexxString *description) { reportCondition(GlobalNames::NOSTRING, description); }

inline void reportException(RexxErrorCodes error)
{
    ActivityManager::currentActivity->reportAnException(error);
}

inline void reportException(RexxErrorCodes error, ArrayClass *args)
{
    ActivityManager::currentActivity->raiseException(error, OREF_NULL, args, OREF_NULL);
}

inline void reportException(RexxErrorCodes error, RexxObject *a1)
{
    ActivityManager::currentActivity->reportAnException(error, a1);
}

inline void reportException(RexxErrorCodes error, wholenumber_t a1)
{
    ActivityManager::currentActivity->reportAnException(error, a1);
}

inline void reportException(RexxErrorCodes error, wholenumber_t a1, wholenumber_t a2)
{
    ActivityManager::currentActivity->reportAnException(error, a1, a2);
}

inline void reportException(RexxErrorCodes error, wholenumber_t a1, RexxObject *a2)
{
    ActivityManager::currentActivity->reportAnException(error, a1, a2);
}

inline void reportException(RexxErrorCodes error, RexxObject *a1, wholenumber_t a2)
{
    ActivityManager::currentActivity->reportAnException(error, a1, a2);
}

inline void reportException(RexxErrorCodes error, const char *a1, RexxObject *a2)
{
    ActivityManager::currentActivity->reportAnException(error, a1, a2);
}

inline void reportException(RexxErrorCodes error, RexxObject *a1, const char *a2)
{
    ActivityManager::currentActivity->reportAnException(error, a1, a2);
}

inline void reportException(RexxErrorCodes error, const char *a1)
{
    ActivityManager::currentActivity->reportAnException(error, a1);
}

inline void reportException(RexxErrorCodes error, const char *a1, const char *a2)
{
    ActivityManager::currentActivity->reportAnException(error, a1, a2);
}

inline void reportException(RexxErrorCodes error, const char *a1, wholenumber_t a2)
{
    ActivityManager::currentActivity->reportAnException(error, a1, a2);
}

inline void reportException(RexxErrorCodes error, const char *a1, wholenumber_t a2, RexxObject *a3)
{
    ActivityManager::currentActivity->reportAnException(error, a1, a2, a3);
}

inline void reportException(RexxErrorCodes error, const char *a1, RexxObject *a2, wholenumber_t a3)
{
    ActivityManager::currentActivity->reportAnException(error, a1, a2, a3);
}

inline void reportException(RexxErrorCodes error, RexxObject *a1, RexxObject *a2)
{
    ActivityManager::currentActivity->reportAnException(error, a1, a2);
}

inline void reportException(RexxErrorCodes error, RexxObject *a1, RexxObject *a2, RexxObject *a3)
{
    ActivityManager::currentActivity->reportAnException(error, a1, a2, a3);
}

inline void reportException(RexxErrorCodes error, RexxObject *a1, RexxObject *a2, RexxObject *a3, RexxObject *a4)
{
    ActivityManager::currentActivity->reportAnException(error, a1, a2, a3, a4);
}

inline void reportException(RexxErrorCodes error, const char *a1, RexxObject *a2, const char *a3, RexxObject *a4)
{
    ActivityManager::currentActivity->reportAnException(error, a1, a2, a3, a4);
}

inline void reportException(RexxErrorCodes error, const char *a1, RexxObject *a2, RexxObject *a3, RexxObject *a4)
{
    ActivityManager::currentActivity->reportAnException(error, new_string(a1), a2, a3, a4);
}

inline void reportException(RexxErrorCodes error, const char *a1, RexxObject *a2, RexxObject *a3)
{
    ActivityManager::currentActivity->reportAnException(error, new_string(a1), a2, a3);
}

inline void reportNomethod(RexxErrorCodes error, RexxString *message, RexxObject *receiver)
{
    if (!ActivityManager::currentActivity->raiseCondition(GlobalNames::NOMETHOD, OREF_NULL, message, receiver, OREF_NULL))
    {
        reportException(error, receiver, message);
    }
}


/**
 * Return name of last message sent via messageSend()
 *
 * @return
 */
inline RexxString *lastMessageName()
{
  return ActivityManager::currentActivity->getLastMessageName();
}


/**
 * A class that can be used to release kernel exclusive access inside
 * a block and have the kernel access automatically reobtained
 * once the UnsafeBlock object goes out of scope.
 */
class UnsafeBlock
{
public:
    UnsafeBlock()
    {
        activity = ActivityManager::currentActivity;
        activity->releaseAccess();
    }

    ~UnsafeBlock()
    {
        activity->requestAccess();
    }
protected:
    Activity *activity;
};


/**
 * Obtain a lock on a semaphore in "safe" fashion.  This will
 * release the kernel lock if it needs to wait on the
 * target semaphore to keep from locking out other threads.
 */
class SafeLock
{
public:
    inline SafeLock(SysMutex &l) : lock(l)
    {
        // make sure we grab the target semaphore first, then
        // the kernel semaphore.
        UnsafeBlock releaser;
        lock.request();
    }


    inline ~SafeLock()
    {
        lock.release();
    }

protected:
     SysMutex &lock;
};


/**
 * A class that can be used to release kernel exclusive access inside
 * a block and have the kernel access automatically reobtained
 * once the UnsafeBlock object goes out of scope.
 */
class CalloutBlock
{
public:
    CalloutBlock()
    {
        activity = ActivityManager::currentActivity;
        activity->exitKernel();
    }

    ~CalloutBlock()
    {
        activity->enterKernel();
    }
protected:
    Activity *activity;
};



class NativeContextBlock
{
public:
    NativeContextBlock();
    ~NativeContextBlock();
    RexxObject *protect(RexxObject *o);

    NativeActivation *self;        // the native activation we operate under
    Activity         *activity;    // our current activity
    InterpreterInstance  *instance;    // potential interpreter instance
};

#endif

