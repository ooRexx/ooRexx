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
#ifndef Included_ActivityManager
#define Included_ActivityManager

#include <deque>

class RexxIdentityTable;
class RexxStack;
class RexxCode;
class RoutineClass;
class RexxNativeActivation;

class ActivityManager
{
public:
    static void live(size_t);
    static void liveGeneral(int reason);

    static void addWaitingActivity(RexxActivity *a, bool release);
    static inline bool hasWaiters() { return !waitingActivities.empty(); }
    static RexxActivity *findActivity();
    static RexxActivity *findActivity(thread_id_t);
    static RexxActivity *getActivity();
    static void returnActivity(RexxActivity *);
    static void activityEnded(RexxActivity *);
    static void shutdown();
    static void checkShutdown();
    static void createInterpreter();
    static void terminateInterpreter();
    static void lockKernel();
    static void unlockKernel();
    static bool lockKernelImmediate();
    static void createLocks();
    static void closeLocks();
    static void init();
    static RexxActivation *newActivation(RexxActivity *activity, RoutineClass *routine, RexxCode *code, RexxString *calltype, RexxString *environment, int context);
    static RexxActivation *newActivation(RexxActivity *activity, RexxActivation *parent, RexxCode *code, int context);
    static RexxActivation *newActivation(RexxActivity *activity, RexxMethod *method, RexxCode *code);
    static RexxNativeActivation *newNativeActivation(RexxActivity *activity, RexxActivation *parent);
    static RexxNativeActivation *newNativeActivation(RexxActivity *activity);
    static RexxActivity *createNewActivity();
    static RexxActivity *createCurrentActivity();
    static RexxActivity *createNewActivity(RexxActivity *);
    static void haltAllActivities();
    static void traceAllActivities(bool on);
    static bool setActivityTrace(thread_id_t thread_id, bool on_or_off);
    static void clearActivityPool();
    static bool poolActivity(RexxActivity *activity);
    static bool haltActivity(thread_id_t thread_id, RexxString * description);
    static void yieldCurrentActivity();
    static void exit(int retcode);
    static void relinquish(RexxActivity *activity);
    static RexxActivity *getRootActivity();
    static void returnRootActivity(RexxActivity *activity);
    static RexxActivity *attachThread();
    static RexxObject *getLocalEnvironment(RexxString *name);
    static RexxDirectory *getLocal();

    static RexxActivity * volatile currentActivity;   // the currently active thread

    static inline void postTermination()
    {
        terminationSem.post();              /* let anyone who cares know we're done*/
    }

    static inline void waitForTermination()
    {
        terminationSem.wait();              // wait until this is posted
    }

protected:
    enum
    {
        MAX_THREAD_POOL_SIZE = 5,       // maximum number of activities we'll pool
    };

                                        /* activities in use                 */
    static RexxList         *activeActivities;
                                        /* free activities                   */
    static RexxList         *availableActivities;
                                        /* table of all localact             */
    static RexxList         *allActivities;
    static RexxIdentityTable  *subClasses;   /* SubClasses...one per system       */
    static bool              processTerminating;  // shutdown processing started
    static size_t            interpreterInstances;  // number of times an interpreter has been created.

    static SysMutex          kernelSemaphore;       // global kernel semaphore lock
    static SysSemaphore      terminationSem;    // used to signal that everything has shutdown
    static volatile bool sentinel;  // used to ensure proper ordering of updates
    static std::deque<RexxActivity *>waitingActivities;   // queue of waiting activities
};


                                       /* various exception/condition       */
                                       /* reporting routines                */
inline void reportCondition(RexxString *condition, RexxString *description) { ActivityManager::currentActivity->raiseCondition(condition, OREF_NULL, description, OREF_NULL, OREF_NULL); }
inline void reportNovalue(RexxString *description) { reportCondition(OREF_NOVALUE, description); }
inline void reportNostring(RexxString *description) { reportCondition(OREF_NOSTRING, description); }

inline void reportException(wholenumber_t error)
{
    ActivityManager::currentActivity->reportAnException(error);
}

inline void reportException(wholenumber_t error, RexxArray *args)
{
    ActivityManager::currentActivity->raiseException(error, OREF_NULL, args, OREF_NULL);
}

inline void reportException(wholenumber_t error, RexxObject *a1)
{
    ActivityManager::currentActivity->reportAnException(error, a1);
}

inline void reportException(wholenumber_t error, wholenumber_t a1)
{
    ActivityManager::currentActivity->reportAnException(error, a1);
}

inline void reportException(wholenumber_t error, wholenumber_t a1, wholenumber_t a2)
{
    ActivityManager::currentActivity->reportAnException(error, a1, a2);
}

inline void reportException(wholenumber_t error, wholenumber_t a1, RexxObject *a2)
{
    ActivityManager::currentActivity->reportAnException(error, a1, a2);
}

inline void reportException(wholenumber_t error, RexxObject *a1, wholenumber_t a2)
{
    ActivityManager::currentActivity->reportAnException(error, a1, a2);
}

inline void reportException(wholenumber_t error, const char *a1, RexxObject *a2)
{
    ActivityManager::currentActivity->reportAnException(error, a1, a2);
}

inline void reportException(wholenumber_t error, RexxObject *a1, const char *a2)
{
    ActivityManager::currentActivity->reportAnException(error, a1, a2);
}

inline void reportException(wholenumber_t error, const char *a1)
{
    ActivityManager::currentActivity->reportAnException(error, a1);
}

inline void reportException(wholenumber_t error, const char *a1, const char *a2)
{
    ActivityManager::currentActivity->reportAnException(error, a1, a2);
}

inline void reportException(wholenumber_t error, const char *a1, wholenumber_t a2)
{
    ActivityManager::currentActivity->reportAnException(error, a1, a2);
}

inline void reportException(wholenumber_t error, const char *a1, wholenumber_t a2, RexxObject *a3)
{
    ActivityManager::currentActivity->reportAnException(error, a1, a2, a3);
}

inline void reportException(wholenumber_t error, const char *a1, RexxObject *a2, wholenumber_t a3)
{
    ActivityManager::currentActivity->reportAnException(error, a1, a2, a3);
}

inline void reportException(wholenumber_t error, RexxObject *a1, RexxObject *a2)
{
    ActivityManager::currentActivity->reportAnException(error, a1, a2);
}

inline void reportException(wholenumber_t error, RexxObject *a1, RexxObject *a2, RexxObject *a3)
{
    ActivityManager::currentActivity->reportAnException(error, a1, a2, a3);
}

inline void reportException(wholenumber_t error, RexxObject *a1, RexxObject *a2, RexxObject *a3, RexxObject *a4)
{
    ActivityManager::currentActivity->reportAnException(error, a1, a2, a3, a4);
}

inline void reportException(wholenumber_t error, const char *a1, RexxObject *a2, const char *a3, RexxObject *a4)
{
    ActivityManager::currentActivity->reportAnException(error, a1, a2, a3, a4);
}

inline void reportException(wholenumber_t error, const char *a1, RexxObject *a2, RexxObject *a3, RexxObject *a4)
{
    ActivityManager::currentActivity->reportAnException(error, new_string(a1), a2, a3, a4);
}

inline void reportException(wholenumber_t error, const char *a1, RexxObject *a2, RexxObject *a3)
{
    ActivityManager::currentActivity->reportAnException(error, new_string(a1), a2, a3);
}

inline void reportNomethod(RexxString *message, RexxObject *receiver)
{
    if (!ActivityManager::currentActivity->raiseCondition(OREF_NOMETHOD, OREF_NULL, message, receiver, OREF_NULL))
    {
                                           /* raise as a syntax error           */
        reportException(Error_No_method_name, receiver, message);
    }
}


inline void missingArgument(size_t argumentPosition)
{
                                       /* just raise the error              */
    reportException(Error_Incorrect_method_noarg, argumentPosition);
}


inline RexxActivity *new_activity()  { return ActivityManager::createNewActivity(); }
inline RexxActivity *new_activity(RexxActivity *parent)  { return ActivityManager::createNewActivity(parent); }


inline RexxString *lastMessageName()
/******************************************************************************/
/* Function:  Return name of last message sent via messageSend()              */
/******************************************************************************/
{
  return ActivityManager::currentActivity->getLastMessageName();
}

inline RexxMethod *lastMethod()
/******************************************************************************/
/* Function:  Return last invoked method object (for use by kernel methods    */
/*            only)                                                           */
/******************************************************************************/
{
    return ActivityManager::currentActivity->getLastMethod();
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
    RexxActivity *activity;
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
    RexxActivity *activity;
};



class NativeContextBlock
{
public:
    NativeContextBlock();
    ~NativeContextBlock();
    RexxObject *protect(RexxObject *o);

    RexxNativeActivation *self;        // the native activation we operate under
    RexxActivity         *activity;    // our current activity
    InterpreterInstance  *instance;    // potential interpreter instance
};

#endif

