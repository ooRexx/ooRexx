/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2021 Rexx Language Association. All rights reserved.    */
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
/*****************************************************************************/
/*                                                                           */
/* Main interpreter control.  This is the preferred location for all         */
/* platform independent global variables.                                    */
/* The interpreter does not instantiate an instance of this                  */
/* class, so most variables and methods should be static.                    */
/*                                                                           */
/*****************************************************************************/

#ifndef RexxInterpreter_Included
#define RexxInterpreter_Included

#include "RexxCore.h"
#include "SysSemaphore.hpp"


class InterpreterInstance;
class ListClass;
class Activity;

class Interpreter
{
public:
    typedef enum
    {
        SAVE_IMAGE_MODE = 0,       // image creation
        RUN_MODE = 1               // normal run mode
    } InterpreterStartupMode;

    static void init();

    static void live(size_t);
    static void liveGeneral(MarkReason reason);

    static void processStartup();
    static void processShutdown();

    static inline void getResourceLock() { resourceLock.request(); }
    static inline void releaseResourceLock() { resourceLock.release(); }
    static inline void getDispatchLock() { dispatchLock.request(); }
    static inline void releaseDispatchLock() { dispatchLock.release(); }
    static inline void createLocks()
    {
        // these are critical-time locks, which involves special processing on Windows
        resourceLock.create(true);
        dispatchLock.create(true);
    }

    static inline void closeLocks()
    {
        dispatchLock.close();
        resourceLock.close();
    }

    static int createInstance(RexxInstance *&instance, RexxThreadContext *&threadContext, RexxOption *options);
    static bool terminateInterpreter();
    static void startInterpreter(InterpreterStartupMode mode, const char *imageTarget);
    static inline bool isTerminated() { return !active; }
    static inline bool isActive() { return active; }
    static bool lastInstance();
    static InterpreterInstance *createInterpreterInstance(RexxOption *options);
    static inline InterpreterInstance *createInterpreterInstance() { return createInterpreterInstance(NULL); }
    static bool terminateInterpreterInstance(InterpreterInstance *instance);
    static bool isInstanceActive(InterpreterInstance *instance);
    static RexxString *getVersionString();
    static void initLocal();
    static size_t getInterpreterVersion();
    static size_t getLanguageLevel();
    static RexxString *getMessageText(wholenumber_t code);

    static inline int getWordSize()
    {
        return sizeof(void *) * 8;
    }

    static inline bool isBigEndian()
    {
        static  const  int mfcone=1;                 // constant 1
        static  const  char *mfctop=(const char *)&mfcone; // -> top byte
        #define LITEND *mfctop             // named flag; 1=little-endian

        return LITEND == 0;
    }


    static void logicError (const char *desc);
    static wholenumber_t messageNumber(RexxString *);
    static bool haltAllActivities(RexxString *);
    static void decodeConditionData(DirectoryClass *conditionObj, RexxCondition *condData);
    static RexxClass *findClass(RexxString *className);
    static RexxString *getCurrentQueue();

    static RexxObject *localServer;         // local environment initialization server
    static const char *languageLevel;       // the language level of this interpreter
    static RexxString* qualifyFileSystemName(RexxString *name);

protected:

    // IMPORTANT NOTE: To avoid deadlocks, never request the kernel lock while holding the resourceLock,
    // otherwise deadlocks are possible. It is permissible to request the resource lock while holding the
    // kernel lock, but this ordering must be strictly observed.
    static SysMutex  resourceLock;   // use to lock resources accessed outside of kernel global lock
    static SysMutex  dispatchLock;   // use to lock when manipulating the activity dispatch queue
    static int    initializations;   // indicates whether we're terminated or not
    static QueueClass *interpreterInstances;  // the set of interpreter instances
    static bool   active;            // indicates whether the interpreter is initialized
    static RexxString *versionNumber;  // our version number information
};


/**
 * Block control for access to the resource lock.
 */
class ResourceSection
{
public:
    inline ResourceSection()
    {
        Interpreter::getResourceLock();
        terminated = false;
    }

    inline ~ResourceSection()
    {
        if (!terminated)
        {
            Interpreter::releaseResourceLock();
        }
    }

    inline void release()
    {
        if (!terminated)
        {
            Interpreter::releaseResourceLock();
            terminated = true;
        }
    }


    inline void reacquire()
    {
        if (terminated)
        {
            Interpreter::getResourceLock();
            terminated = false;
        }
    }

private:

    bool terminated;       // we can release these as needed
};


/**
 * Block control for access to the dispatch queue.
 */
class DispatchSection
{
public:
    inline DispatchSection()
    {
        Interpreter::getDispatchLock();
        terminated = false;
    }

    inline ~DispatchSection()
    {
        if (!terminated)
        {
            Interpreter::releaseDispatchLock();
        }
    }

    inline void release()
    {
        if (!terminated)
        {
            Interpreter::releaseDispatchLock();
            terminated = true;
        }
    }


    inline void reacquire()
    {
        if (terminated)
        {
            Interpreter::getDispatchLock();
            terminated = false;
        }
    }

private:

    bool terminated;       // we can release these as needed
};


class InstanceBlock
{
public:
    InstanceBlock();
    InstanceBlock(RexxOption *options);
    InstanceBlock(PRXSYSEXIT exits, const char *env);
    ~InstanceBlock();

    Activity         *activity;    // our current activity
    InterpreterInstance  *instance;    // potential interpreter instance
};


#endif
