/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
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


// global resource lock
SMTX Interpreter::resourceLock = 0;

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
}

void Interpreter::liveGeneral(int reason)
{
  if (!memoryObject.savingImage())
  {
      memory_mark_general(interpreterInstances);
      memory_mark_general(localServer);
  }
}

void Interpreter::processStartup()
{
    // the locks get create in order
    createLocks();
    ActivityManager::createLocks();
    RexxMemory::createLocks();

    // This is unconditional...it will fail if already loaded.
    if (RexxRegisterFunctionDll("SYSLOADFUNCS", "REXXUTIL", "SysLoadFuncs") == 0)
    {
        /* default return code buffer        */
        char      default_return_buffer[DEFRXSTRING];
        RXSTRING funcresult;
        int functionrc;                      /* Return code from function         */

        /* first registration?               */
        /* set up an result RXSTRING         */
        MAKERXSTRING(funcresult, default_return_buffer, sizeof(default_return_buffer));
        /* call the function loader          */
        RexxCallFunction("SYSLOADFUNCS", 0, (PCONSTRXSTRING)NULL, &functionrc, &funcresult, "");
    }
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
        // TODO:  Make sure these are necessary in shared code
        setbuf(stdout, NULL);              // turn off buffering for the output streams
        setbuf(stderr, NULL);
        SystemInterpreter::startInterpreter();   // perform system specific initialization
        // initialize the memory manager , and restore the
        // memory image
        memoryObject.initialize(mode == RUN_MODE);
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
                server_class->messageSend(OREF_NEW, 0, OREF_NULL, result);
                localServer = (RexxObject *)result;
            }
        }
    }
    // we're live now
    active = true;
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
        // if already shutdown, then we've got a quick return
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
    }

    // we need to wait for the activity manager to tell us everything is
    // ready to go, but without holding the resource lock
    ActivityManager::waitForTermination();
    {
        // no initialized interpreter environment any more.
        active = false;
        ResourceSection lock;      // lock in this section
        SystemInterpreter::terminateInterpreter();

        // now shutdown the memory object
        memoryObject.shutdown();
        interpreterInstances = OREF_NULL;
    }
    return true;
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
InterpreterInstance *Interpreter::createInterpreterInstance(PRXSYSEXIT exits, const char *defaultEnvironment)
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
    instance->initialize(rootActivity, exits, defaultEnvironment);
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
    // this might not be in a state where it can be terminated
    if (!instance->terminate())
    {
        return false;
    }

    ResourceSection lock;

    interpreterInstances->removeItem((RexxObject *)instance);
    return true;
}


/**
 * Tell the interpreter to have all of the instances halt its activities.
 */
void Interpreter::haltAllActivities()
{
    ResourceSection lock;

    for (size_t listIndex = interpreterInstances->firstIndex() ;
         listIndex != LIST_END;
         listIndex = interpreterInstances->nextIndex(listIndex) )
    {
                                         /* Get the next message object to    */
                                         /*process                            */
        InterpreterInstance *instance = (InterpreterInstance *)interpreterInstances->getValue(listIndex);
        // halt every thing
        instance->haltAllActivities();
    }
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
InstanceBlock::InstanceBlock(PRXSYSEXIT exits, const char *defaultEnvironment)
{
    // Get an instance.  This also gives the root activity of the instance
    // the kernel lock.
    instance = Interpreter::createInterpreterInstance(exits, defaultEnvironment);
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
