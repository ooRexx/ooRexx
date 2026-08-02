/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2025 Rexx Language Association. All rights reserved.    */
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
#ifndef Included_ActivityManager
#define Included_ActivityManager

#include "Activity.hpp"
#include "ActivationSettings.hpp"
#include "ActivityList.hpp"
#include <deque>
#include <atomic>
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
class DispatchSection;


/**
 * The queue of activities that are waiting to be given kernel access,
 * bundled together with the lock that protects it.
 *
 * The container is private and can only be reached through a DispatchSection,
 * which is the object that holds the lock, so there is no way to touch the
 * queue without holding the lock.  This used to be a bare static member of
 * ActivityManager, with every access relying on the caller having taken the
 * lock by convention.  At least one caller did not (see bug #2072), and
 * ThreadSanitizer reported this queue as the most-raced object in the
 * interpreter.
 */
class WaitingActivityQueue
{
    // the section object is the one holding the lock, so it is the only thing
    // allowed to reach the container.
    friend class DispatchSection;

public:
    // this is a critical-time lock, which involves special processing on Windows.
    static inline void createLock() { instance().dispatchLock.create(true); }
    static inline void closeLock() { instance().dispatchLock.close(); }

protected:
    // IMPORTANT NOTE: To avoid deadlocks, never request the kernel lock while holding
    // the dispatch lock. It is permissible to request the dispatch lock while holding
    // the kernel lock, but this ordering must be strictly observed.
    struct Data
    {
        SysMutex dispatchLock;                   // guards the queue below
        std::deque<Activity *> queue;            // the activities awaiting dispatch
    };

    // These live in a function-local static, initialized on first use.  A
    // namespace-scope object is constructed in link order relative to the
    // _rexx_init() ELF constructor that calls createLock(), and if it runs
    // afterwards the SysMutex constructor resets the created flag, leaving
    // request() returning false and this section guarding nothing for the whole
    // life of the process.  That is not hypothetical: it is what the dispatch
    // lock did while it was declared in Interpreter.cpp.  See
    // Interpreter::resourceLock() and bug #2078 for the measurement.
    static Data &instance();

    static inline bool lock() { return instance().dispatchLock.request(); }
    static inline void unlock() { instance().dispatchLock.release(); }
};


/**
 * Block control for access to the dispatch queue.  Holding one of these is
 * the only way to reach the queue itself.
 */
class DispatchSection
{
public:
    inline DispatchSection()
    {
        // if the acquire fails we must NOT unlock in the destructor. An unbalanced
        // unlock on a recursive mutex decrements the count and can drop a lock an
        // outer scope still believes it holds, destroying mutual exclusion for
        // everyone. See bug #2071.
        locked = WaitingActivityQueue::lock();
    }

    inline ~DispatchSection()
    {
        release();
    }

    // a copy would release the lock twice
    DispatchSection(const DispatchSection &) = delete;
    DispatchSection &operator=(const DispatchSection &) = delete;

    inline void release()
    {
        if (locked)
        {
            locked = false;
            WaitingActivityQueue::unlock();
        }
    }

    inline void reacquire()
    {
        if (!locked)
        {
            locked = WaitingActivityQueue::lock();
        }
    }

    inline bool isLocked() { return locked; }

    // The queue operations. These are only reachable from a section object, so the
    // lock is held whenever the container is touched.  If the lock could not be
    // obtained (the locks have not been created yet, or have already been closed
    // during shutdown), then the queue cannot be touched safely at all, so these
    // do nothing rather than race on the container.

    inline bool isEmpty() { return !locked || WaitingActivityQueue::instance().queue.empty(); }

    /**
     * Add an activity to the end of the dispatch queue.
     *
     * @param activity The activity to queue up.
     */
    inline void add(Activity *activity)
    {
        if (locked)
        {
            WaitingActivityQueue::instance().queue.push_back(activity);
        }
    }

    /**
     * Remove and return the activity at the front of the queue.
     *
     * @return The first queued activity, or OREF_NULL if the queue is empty.
     */
    inline Activity *removeFirst()
    {
        if (isEmpty())
        {
            return OREF_NULL;
        }
        Activity *activity = WaitingActivityQueue::instance().queue.front();
        WaitingActivityQueue::instance().queue.pop_front();
        return activity;
    }

    /**
     * Return the activity at the front of the queue without removing it.
     *
     * @return The first queued activity, or OREF_NULL if the queue is empty.
     */
    inline Activity *peekFirst()
    {
        return isEmpty() ? OREF_NULL : WaitingActivityQueue::instance().queue.front();
    }

    /**
     * Remove a specific activity from anywhere in the queue.
     *
     * @param activity The activity to remove.  Not being queued is not an error.
     */
    inline void remove(Activity *activity)
    {
        if (!locked)
        {
            return;
        }

        std::deque<Activity *> &queue = WaitingActivityQueue::instance().queue;
        for (std::deque<Activity *>::iterator it = queue.begin(); it != queue.end(); ++it)
        {
            if (*it == activity)
            {
                queue.erase(it);
                return;
            }
        }
        // ignore this if not found.
    }

private:

    bool locked;           // true if we actually hold the lock
};


class ActivityManager
{
public:
    static void live(size_t);
    static void liveGeneral(MarkReason reason);

    static void addWaitingActivity(Activity *a, bool release);
    static void addWaitingApiActivity(Activity *a);
    // the caller must be holding the dispatch lock, which the section argument proves.
    static bool dispatchNext(DispatchSection &lock);
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
        kernelLock().request();
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
        kernelLock().release();
    }

    static void releaseAccess(bool dispatch = false);
    static bool lockKernelImmediate();
    static void createLocks();
    static void closeLocks();
    static void init();
    static RexxActivation *newActivation(Activity *activity, RexxActivation *parent, RoutineClass *routine, RexxCode *code, RexxString *calltype, RexxString *environment, ActivationContext context);
    static RexxActivation *newActivation(Activity *activity, RexxActivation *parent, RexxCode *code, ActivationContext context);
    static RexxActivation *newActivation(Activity *activity, RexxActivation *parent, MethodClass *method, RexxCode *code);
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
    static void returnWaitingActivity(Activity *waitingAct);
    static void handleNestedActivity(Activity *newActivity, Activity *oldActivity);

    // non-static method that is attached to the environment directory
    DirectoryClass *getLocalRexx()
    {
        return getLocal();
    }

    // was "Activity * volatile" -- volatile provides no ordering; see bug #2074
    static std::atomic<Activity *> currentActivity;   // the currently active thread

    static inline void postTermination()
    {
        terminationLock().post();              /* let anyone who cares know we're done*/
    }

    static inline void waitForTermination()
    {
        terminationLock().wait();              // wait until this is posted
    }

protected:

    // maximum number of activities we'll pool
    static const size_t MAX_THREAD_POOL_SIZE = 5;
    static const uint64_t timeSliceLength = 24;            // how long we'll run before checking for a control yield.

    static QueueClass       *availableActivities;     // table of available activities
    // NOT a Rexx object: see ActivityList.hpp. activityEnded() maintains this
    // after runThread() has released kernel access, so it must not be walked by
    // the collector.
    // Every touch of this list needs the resource lock, with no
    // exceptions. Kernel access is NOT a substitute: it excludes the collector,
    // but not a thread holding only the resource lock, and Interpreter::
    // haltAllActivities() and traceAllActivities() are exactly that -- reachable
    // from a signal handler and from the RexxHaltInstance API on any thread.
    // Unlike the QueueClass this replaced, whose backing store was a Rexx object
    // and so stayed live for a stale reader, a vector reallocation hands the old
    // buffer straight back to operator delete.
    //
    // A function-local static that is never destroyed. As a namespace-scope
    // object its destructor would be registered with __cxa_atexit, and its order
    // relative to _rexx_fini() -- an ELF destructor -- is unspecified, so a
    // thread reaching activityEnded() or returnRootActivity() during shutdown
    // could search and erase freed storage. Same reasoning as the locks above.
    static ActivityList &allActivities();             // table of all activities
    static bool              processTerminating;      // shutdown processing started
    static size_t            interpreterInstances;    // number of times an interpreter has been created.

     // IMPORTANT NOTE: To avoid deadlocks, never request the kernel lock while holding the resourceLock,
     // otherwise deadlocks are possible. It is permissible to request the resource lock while holding the
     // kernel lock, but this ordering must be strictly observed.
    // function-local statics, for the reason given on WaitingActivityQueue::instance()
    static SysMutex &kernelLock();                    // global kernel semaphore lock
    static SysSemaphore &terminationLock();           // used to signal that everything has shutdown
    // NOTE: was "volatile bool".  volatile does NOT order surrounding non-volatile
    // accesses and emits no fence, so the barrier the comments below claim never
    // existed.  ThreadSanitizer reports this as the single most-raced object in the
    // interpreter (94 races in one targeted run).  std::atomic gives the intended
    // sequentially-consistent ordering.  See bug #2074.
    static std::atomic<bool> sentinel;                // used to ensure proper ordering of updates
    // NOTE: the queue of waiting activities now lives in WaitingActivityQueue above,
    // where it is reachable only while holding the lock that protects it.
    static std::atomic<size_t> waitingAttaches;                  // the count of attaches waiting for access
    static std::atomic<size_t> waitingAccess;                      // the count of activities waiting for access
    static std::atomic<size_t> waitingApiAccess;                   // the count of activities waiting for access for API callbacks.
    static uint64_t          lastLockTime;            // the last time we granted the kernel lock.
};


// various exception/condition reporting routines
inline void reportCondition(RexxString *condition, RexxObject *description) { ActivityManager::currentActivity.load()->raiseCondition(condition, OREF_NULL, description, OREF_NULL, OREF_NULL); }
inline void reportNovalue(RexxString *description) { reportCondition(GlobalNames::NOVALUE, description); }
inline void reportNostring(RexxString *description) { reportCondition(GlobalNames::NOSTRING, description); }

inline void reportException(RexxErrorCodes error)
{
    ActivityManager::currentActivity.load()->reportAnException(error);
}

inline void reportException(RexxErrorCodes error, ArrayClass *args)
{
    ActivityManager::currentActivity.load()->raiseException(error, OREF_NULL, args, OREF_NULL);
}

inline void reportException(RexxErrorCodes error, RexxObject *a1)
{
    ActivityManager::currentActivity.load()->reportAnException(error, a1);
}

inline void reportException(RexxErrorCodes error, wholenumber_t a1)
{
    ActivityManager::currentActivity.load()->reportAnException(error, a1);
}

inline void reportException(RexxErrorCodes error, wholenumber_t a1, wholenumber_t a2)
{
    ActivityManager::currentActivity.load()->reportAnException(error, a1, a2);
}

inline void reportException(RexxErrorCodes error, wholenumber_t a1, RexxObject *a2)
{
    ActivityManager::currentActivity.load()->reportAnException(error, a1, a2);
}

inline void reportException(RexxErrorCodes error, RexxObject *a1, wholenumber_t a2)
{
    ActivityManager::currentActivity.load()->reportAnException(error, a1, a2);
}

inline void reportException(RexxErrorCodes error, const char *a1, RexxObject *a2)
{
    ActivityManager::currentActivity.load()->reportAnException(error, a1, a2);
}

inline void reportException(RexxErrorCodes error, RexxObject *a1, const char *a2)
{
    ActivityManager::currentActivity.load()->reportAnException(error, a1, a2);
}

inline void reportException(RexxErrorCodes error, const char *a1)
{
    ActivityManager::currentActivity.load()->reportAnException(error, a1);
}

inline void reportException(RexxErrorCodes error, const char *a1, const char *a2)
{
    ActivityManager::currentActivity.load()->reportAnException(error, a1, a2);
}

inline void reportException(RexxErrorCodes error, const char *a1, wholenumber_t a2)
{
    ActivityManager::currentActivity.load()->reportAnException(error, a1, a2);
}

inline void reportException(RexxErrorCodes error, const char *a1, wholenumber_t a2, RexxObject *a3)
{
    ActivityManager::currentActivity.load()->reportAnException(error, a1, a2, a3);
}

inline void reportException(RexxErrorCodes error, const char *a1, RexxObject *a2, wholenumber_t a3)
{
    ActivityManager::currentActivity.load()->reportAnException(error, a1, a2, a3);
}

inline void reportException(RexxErrorCodes error, RexxObject *a1, RexxObject *a2)
{
    ActivityManager::currentActivity.load()->reportAnException(error, a1, a2);
}

inline void reportException(RexxErrorCodes error, RexxObject *a1, RexxObject *a2, RexxObject *a3)
{
    ActivityManager::currentActivity.load()->reportAnException(error, a1, a2, a3);
}

inline void reportException(RexxErrorCodes error, RexxObject *a1, RexxObject *a2, RexxObject *a3, RexxObject *a4)
{
    ActivityManager::currentActivity.load()->reportAnException(error, a1, a2, a3, a4);
}

inline void reportException(RexxErrorCodes error, const char *a1, RexxObject *a2, const char *a3, RexxObject *a4)
{
    ActivityManager::currentActivity.load()->reportAnException(error, a1, a2, a3, a4);
}

inline void reportException(RexxErrorCodes error, const char *a1, RexxObject *a2, RexxObject *a3, RexxObject *a4)
{
    ActivityManager::currentActivity.load()->reportAnException(error, new_string(a1), a2, a3, a4);
}

inline void reportException(RexxErrorCodes error, const char *a1, RexxObject *a2, RexxObject *a3)
{
    ActivityManager::currentActivity.load()->reportAnException(error, new_string(a1), a2, a3);
}

inline void reportNomethod(RexxErrorCodes error, RexxString *message, RexxObject *receiver)
{
    if (!ActivityManager::currentActivity.load()->raiseCondition(GlobalNames::NOMETHOD, OREF_NULL, message, receiver, OREF_NULL))
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
  return ActivityManager::currentActivity.load()->getLastMessageName();
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

