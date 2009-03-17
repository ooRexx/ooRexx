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
/******************************************************************************/
/* Implementation of the InterpreterInstance class                            */
/*                                                                            */
/******************************************************************************/

#include "InterpreterInstance.hpp"
#include "Interpreter.hpp"
#include "SystemInterpreter.hpp"
#include "ActivityManager.hpp"
#include "RexxActivation.hpp"
#include "PackageManager.hpp"
#include "DirectoryClass.hpp"
#include "CommandHandler.hpp"

/**
 * Create a new Package object instance.
 *
 * @param size   Size of the object.
 *
 * @return Pointer to new object storage.
 */
void *InterpreterInstance::operator new(size_t size)
{
    return new_object(size, T_InterpreterInstance);
}

InterpreterInstance::InterpreterInstance()
{
    // this needs to be created and set
    terminationSem.create();
    terminationSem.reset();

    // fill in the interface vectore
    context.instanceContext.functions = &interfaceVector;
    // this back-link allows us to recover the instance pointer on the
    // API callbacks.
    context.instance = this;
}


void InterpreterInstance::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
    memory_mark(rootActivity);
    memory_mark(allActivities);
    memory_mark(globalReferences);
    memory_mark(defaultEnvironment);
    memory_mark(searchPath);
    memory_mark(searchExtensions);
    memory_mark(securityManager);
    memory_mark(localEnvironment);
    memory_mark(commandHandlers);
}


void InterpreterInstance::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
    memory_mark_general(rootActivity);
    memory_mark_general(allActivities);
    memory_mark_general(globalReferences);
    memory_mark_general(defaultEnvironment);
    memory_mark_general(searchPath);
    memory_mark_general(searchExtensions);
    memory_mark_general(securityManager);
    memory_mark_general(localEnvironment);
    memory_mark_general(commandHandlers);
}


/**
 * Initialize an interpreter instance.
 *
 * @param activity The root activity for the interpreter instance.
 * @param handlers The exit handlers used by all threads running under this instance.
 * @param defaultEnvironment
 *                 The default address environment for this interpreter instance.  Each
 *                 active interpreter instance can define its own default environment.
 */
void InterpreterInstance::initialize(RexxActivity *activity, RexxOption *options)
{
    rootActivity = activity;
    allActivities = new_list();
    searchExtensions = new_list();     // this will be filled in during options processing
    // this gets added to the entire active list.
    allActivities->append((RexxObject *)activity);
    globalReferences = new_identity_table();
    // create a default wrapper for this security manager
    securityManager = new SecurityManager(OREF_NULL);
    // set the default system address environment (can be overridden by options)
    defaultEnvironment = SystemInterpreter::getDefaultAddressName();
    // our list of command handlers (must be done before options are processed)
    commandHandlers = new_directory();
    processOptions(options);
    // do system specific initialization
    sysInstance.initialize(this, options);
    // register the system command handlers for this platform.
    sysInstance.registerCommandHandlers(this);

    // associate the thread with this instance
    activity->setupAttachedActivity(this);
    // create a local environment
    localEnvironment = new_directory();
    // now do the local initialization;
    Interpreter::initLocal();
}


/**
 * Set a new security manager object for this instance.
 *
 * @param m      The security manager to set.
 */
void InterpreterInstance::setSecurityManager(RexxObject *m)
{
    securityManager = new SecurityManager(m);
}


/**
 * Attach a thread to an interpreter instance, returning the
 * activity thread context.
 *
 * @param attachedContext
 *               The pointer for returning the thread context.
 *
 * @return 0 indicates success.
 */
int InterpreterInstance::attachThread(RexxThreadContext *&attachedContext)
{
    RexxActivity *activity = attachThread();
    attachedContext = activity->getThreadContext();
    // When we attach, we get the current lock.  We need to ensure we
    // release this before returning control to the outside world.
    activity->releaseAccess();
    return 0;
}


/**
 * Attach a thread to an interpreter instance.
 *
 * @return The attached activity.
 */
RexxActivity *InterpreterInstance::attachThread()
{
    // first check for an existing activity
    RexxActivity *activity = findActivity();
    // do we have this?  we can just return it
    if (activity != OREF_NULL)
    {
        // make sure we mark this as attached...we might be nested and don't want to
        // clean this up until we complete
        activity->nestAttach();
        return activity;
    }

    // we need to get a new activity set up for this particular thread
    activity = ActivityManager::attachThread();
    // this is still attached, but we'll release it once it is detached.  We start with
    // a count of 1 and cleanup once we hit zero.
    activity->nestAttach();
    // resource lock must come AFTER we attach the thread, otherwise
    // we can create a deadlock situation when we attempt to get the kernel
    // lock
    ResourceSection lock;
    // add this to the activity lists
    allActivities->append((RexxObject *)activity);
    // associate the thread with this instance
    activity->setupAttachedActivity(this);
    return activity;
}


/**
 * Detach a thread from this interpreter instance.
 *
 * @param activity The activity to detach
 *
 * @return true if this worked ok.
 */
bool InterpreterInstance::detachThread(RexxActivity *activity)
{
    // if the thread in question is not found, is not an attached thread, or
    // the thread is currently busy, this fails
    if (activity == OREF_NULL || !activity->isAttached() || activity->isActive())
    {
        return false;
    }

    // if we reused the activity because of a nested callback attach, then
    // we just decrement the nesting count and return without cleaning up
    // any resources.
    activity->returnAttach();
    if (activity->isNestedAttach())
    {
        return true;
    }


    // this activity owned the kernel semaphore before entering here...release it
    // now.
    activity->releaseAccess();
    ResourceSection lock;

    allActivities->removeItem((RexxObject *)activity);
    // have the activity manager remove this from the global tables
    // and perform resource cleanup
    ActivityManager::returnActivity(activity);

    // Was this the last detach of an thread?  Signal the shutdown event
    if (allActivities->items() == 0 && terminating)
    {
        terminationSem.post();
    }
    return true;
}


/**
 * Detach a thread from this interpreter instance.
 *
 * @return true if this worked ok.
 */
bool InterpreterInstance::detachThread()
{
    // first check for an existing activity
    return detachThread(findActivity());
}



/**
 * Spawn off a new thread from an existing thread.
 *
 * @param parent The parent thread.
 *
 * @return The attached activity.
 */
RexxActivity *InterpreterInstance::spawnActivity(RexxActivity *parent)
{
    // create a new activity
    RexxActivity *activity = ActivityManager::createNewActivity(parent);
    // associate the thread with this instance
    activity->addToInstance(this);
    // add this to the activities list
    ResourceSection lock;

    allActivities->append((RexxObject *)activity);
    return activity;
}


/**
 * Return a spawned activity back to the activity pool.  This
 * will disassociate the activity from the interpreter instance
 * and place the thread back into the global thread pool.
 *
 * @param activity The activity to return.
 *
 * @return true if the activity was added to the pool, false if the
 *         activity should continue with termination.
 */
bool InterpreterInstance::poolActivity(RexxActivity *activity)
{
    ResourceSection lock;
    // detach from this instance
    activity->detachInstance();
    // remove from the activities lists for the instance
    allActivities->removeItem((RexxObject*)activity);
    if (terminating)
    {
        // is this the last one to finish up?  Generally, the main thread
        // will be waiting for this to terminate.  That is thread 1, we're thread
        // 2.  In reality, this is the test for the last "spawned" thread.
        if (allActivities->items() <= 1)
        {
            terminationSem.post();
        }
        // don't allow this to be pooled
        return false;
    }
    // and move this to the global activity pool
    return ActivityManager::poolActivity(activity);
}


/**
 * Locate an activity for a specific thread ID that's attached
 * to this instance.
 *
 * @param threadId The target thread id.
 *
 * @return The associated activity, or OREF_NULL if the current thread
 *         is not attached.
 */
RexxActivity *InterpreterInstance::findActivity(thread_id_t threadId)
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


/**
 * Find an activity for the current thread that's associated
 * with this activity.
 *
 * @return The target activity.
 */
RexxActivity *InterpreterInstance::findActivity()
{
    return findActivity(SysActivity::queryThreadID());
}


/**
 * Enter on the current thread context, making sure the interpreter
 * lock is obtained.
 *
 * @return The activity object associated with this thread/instance
 *         combination.
 */
RexxActivity *InterpreterInstance::enterOnCurrentThread()
{
    RexxActivity *activity;
    {
        ResourceSection lock;              // lock the outer control block access
        // attach this thread to the current activity
        activity = attachThread();
        // this will also get us the kernel lock, and take care of nesting
        activity->activate();
    }
    // we need to ensure the resource lock is released before we attempt to
    // acquire the kernel lock
    activity->requestAccess();
    // return the activity in case the caller needs it.
    return activity;
}


/**
 * We're leaving the current thread.  So we need to deactivate
 * this.
 */
void InterpreterInstance::exitCurrentThread()
{
    // find the current activity and deactivate it, and
    // release the kernel lock.
    RexxActivity *activity = findActivity();
    activity->exitCurrentThread();
}


void InterpreterInstance::removeInactiveActivities()
{
    size_t count = allActivities->items();

    // This is a bit complicated.  Each activity will be removed from the
    // head of the list, and any activity not ready for termination is
    // put back on the end.  Since we're using the initial count of the
    // items for handling this, we'll look at each activity at most once.
    // If there are any items left, those are activities we can't release yet.
    for (size_t i = 0; i < count; i++)
    {
        RexxActivity *activity = (RexxActivity *)allActivities->removeFirstItem();
        if (activity->isActive())
        {
            allActivities->append((RexxObject *)activity);
        }
        else
        {
            // have the inactive thread wake up and terminate
            activity->terminatePoolActivity();
        }
    }
}


/**
 * Attempt to shutdown the interpreter instance.  This can only be done
 * from the root activity.
 *
 * @return true if shutdown has been initiated, false otherwise.
 */
bool InterpreterInstance::terminate()
{
    // if our current activity is not the root one, we can't do that
    RexxActivity *current = findActivity();
    // we also can't be doing active work on the root thread
    if (current != rootActivity || rootActivity->isActive())
    {
        return false;
    }

    terminated = false;
    // turn on the global termination in process flag
    terminating = true;

    {

        ResourceSection lock;
        // remove the current activity from the list so we don't clean everything
        // up.  We need to
        allActivities->removeItem((RexxObject *)current);
        // go remove all of the activities that are not doing work for this instance
        removeInactiveActivities();
        // no activities left?  We can leave now
        terminated = allActivities->items() == 0;
        // we need to restore the rootActivity to the list for potentially running uninits
        allActivities->append((RexxObject *)current);
    }

    // if there are active threads still running, we need to wait until
    // they all finish
    if (!terminated)
    {
        terminationSem.wait();
    }

    // if everything has terminated, then make sure we run the uninits before shutting down.
    // This activity is currently the current activity.  We're going to run the
    // uninits on this one, so reactivate it until we're done running
    enterOnCurrentThread();
    // release any global references we've been holding.
    globalReferences->empty();
    // before we update of the data structures, make sure we process any
    // pending uninit activity.
    memoryObject.collectAndUninit(Interpreter::lastInstance());

    // do system specific termination of an instance
    sysInstance.terminate();

    // ok, deactivate this again...this will return the activity because the terminating
    // flag is on.
    exitCurrentThread();
    terminationSem.close();

    // make sure the root activity is removed by the ActivityManager;
    ActivityManager::returnRootActivity(current);

    // tell the main interpreter controller we're gone.
    Interpreter::terminateInterpreterInstance(this);
    return true;
}


/**
 * Add an object to the global references table.
 *
 * @param o      The added object.
 */
void InterpreterInstance::addGlobalReference(RexxObject *o)
{
    if (o != OREF_NULL)
    {
        globalReferences->put(o, o);
    }
}

/**
 * Remove the global reference protection from an object.
 *
 * @param o      The protected object.
 */
void InterpreterInstance::removeGlobalReference(RexxObject *o)
{
    if (o != OREF_NULL)
    {
        globalReferences->remove(o);
    }
}


/**
 * Raise a halt condition on all running activities.
 */
bool InterpreterInstance::haltAllActivities()
{
    bool result = true;
    for (size_t listIndex = allActivities->firstIndex() ;
         listIndex != LIST_END;
         listIndex = allActivities->nextIndex(listIndex) )
    {
                                         /* Get the next message object to    */
                                         /*process                            */
        RexxActivity *activity = (RexxActivity *)allActivities->getValue(listIndex);
        // only halt the active ones
        if (activity->isActive())
        {
            result = result && activity->halt(OREF_NULL);
        }
    }
    return result;
}


/**
 * Raise a trace condition on all running activities.
 */
void InterpreterInstance::traceAllActivities(bool on)
{
    for (size_t listIndex = allActivities->firstIndex() ;
         listIndex != LIST_END;
         listIndex = allActivities->nextIndex(listIndex) )
    {
                                         /* Get the next message object to    */
                                         /*process                            */
        RexxActivity *activity = (RexxActivity *)allActivities->getValue(listIndex);
        // only activate the active ones
        if (activity->isActive())
        {
            activity->setTrace(on);
        }
    }
}


/**
 * Process interpreter instance options.
 *
 * @param options The list of defined options.
 *
 * @return True if the options were processed correctly, false otherwise.
 */
bool InterpreterInstance::processOptions(RexxOption *options)
{
    // options are, well, optional...if nothing specified, we're done.
    if (options == NULL)
    {
        return true;
    }
    // loop until we get to the last option item
    while (options->optionName != NULL)
    {
        // an initial address environment
        if (strcmp(options->optionName, INITIAL_ADDRESS_ENVIRONMENT) == 0)
        {
            defaultEnvironment = new_string(options->option.value.value_CSTRING);
        }
        // application data
        else if (strcmp(options->optionName, APPLICATION_DATA) == 0)
        {
            // this is filled in to the instance context vector
            context.instanceContext.applicationData = options->option.value.value_POINTER;
        }
        // an additional search path
        else if (strcmp(options->optionName, EXTERNAL_CALL_PATH) == 0)
        {
            searchPath = new_string(options->option.value.value_CSTRING);
        }
        // additional extensions for processing
        else if (strcmp(options->optionName, EXTERNAL_CALL_EXTENSIONS) == 0)
        {
            const char *extStart = options->option.value.value_CSTRING;
            const char *extEnd = extStart + strlen(extStart);

            while (extStart < extEnd)
            {
                const char *delim = strchr(extStart, ',');
                if (delim == NULL)
                {
                    delim = extEnd;
                }
                // make this into a string value and append
                RexxString *ext = new_string(extStart, delim - extStart);
                searchExtensions->append(ext);

                // step past the delimiter and loop
                extStart = delim + 1;
            }
        }
        // old-style registered exit
        else if (strcmp(options->optionName, REGISTERED_EXITS) == 0)
        {
            RXSYSEXIT *handlers = (RXSYSEXIT *)options->option.value.value_POINTER;
            // if we have handlers, initialize the array
            if (handlers != NULL)
            {
                                               /* while not the list ender          */
                for (int i= 0; handlers[i].sysexit_code != RXENDLST; i++)
                {
                    /* enable this exit                  */
                    setExitHandler(handlers[i]);
                }
            }
        }
        // new-style context exit
        else if (strcmp(options->optionName, DIRECT_EXITS) == 0)
        {
            RexxContextExit *handlers = (RexxContextExit *)options->option.value.value_POINTER;
            // if we have handlers, initialize the array
            if (handlers != NULL)
            {
                                               /* while not the list ender          */
                for (int i= 0; handlers[i].sysexit_code != RXENDLST; i++)
                {
                    /* enable this exit                  */
                    setExitHandler(handlers[i]);
                }
            }
        }
        // old-style registered command handler
        else if (strcmp(options->optionName, REGISTERED_ENVIRONMENTS) == 0)
        {
            RexxRegisteredEnvironment *handlers = (RexxRegisteredEnvironment *)options->option.value.value_POINTER;
            // if we have handlers, initialize the array
            if (handlers != NULL)
            {
                                               /* while not the list ender          */
                for (int i= 0; handlers[i].registeredName != NULL; i++)
                {
                    // add the handler to this setup
                    addCommandHandler(handlers[i].name, handlers[i].registeredName);
                }
            }
        }
        // a direct call command handler
        else if (strcmp(options->optionName, DIRECT_ENVIRONMENTS) == 0)
        {
            RexxContextEnvironment *handlers = (RexxContextEnvironment *)options->option.value.value_POINTER;
            // if we have handlers, initialize the array
            if (handlers != NULL)
            {
                                               /* while not the list ender          */
                for (int i= 0; handlers[i].handler != NULL; i++)
                {
                    // add the handler to this setup
                    addCommandHandler(handlers[i].name, (REXXPFN)handlers[i].handler);
                }
            }
        }
        // a package to load at startup
        else if (strcmp(options->optionName, LOAD_REQUIRED_LIBRARY) == 0)
        {
            RexxString *libraryName = new_string(options->option.value.value_CSTRING);

            // this must load ok in order for this to work
            PackageManager::getLibrary(libraryName);
        }
        else
        {
            // unknown option
            return false;
        }
        // step to the next option value
        options++;
    }
    return true;
}


/**
 * Get the thread context vector for the root activity of the
 * instance.
 *
 * @return The root RexxThreadContext environment;
 */
RexxThreadContext *InterpreterInstance::getRootThreadContext()
{
    return rootActivity->getThreadContext();
}


/**
 * Retrieve a value from the instance local environment.
 *
 * @param name   The name of the .local object.
 *
 * @return The object stored at the given name.
 */
RexxObject *InterpreterInstance::getLocalEnvironment(RexxString *name)
{
    if (localEnvironment == OREF_NULL)
    {
        return TheNilObject;
    }
    return localEnvironment->at(name);
}

/**
 * Add a handler to the environment list.
 *
 * @param name       The name of the address environment this services.
 * @param entryPoint The entry point address of the handler.
 */
void InterpreterInstance::addCommandHandler(const char *name, REXXPFN entryPoint)
{
    RexxString *handlerName = new_upper_string(name);
    commandHandlers->put((RexxObject *)new CommandHandler(entryPoint), handlerName);
}

/**
 * Add a handler for a registered subcom handler to the
 * address handler list.
 *
 * @param name   The environment name of the handler.
 * @param registeredName
 *               The name of the registered subcom handler.
 */
void InterpreterInstance::addCommandHandler(const char *name, const char *registeredName)
{
    RexxString *handlerName = new_upper_string(name);
    CommandHandler *handler = new CommandHandler(registeredName);
    // it's possible we were give a bogus name, so validate this first
    if (handler->isResolved())
    {
        commandHandlers->put((RexxObject *)handler, handlerName);
    }
}


/**
 * Resolve a command handler for invoking a command.
 *
 * @param name   The name of the target address environment.
 *
 * @return The resolved handler, or OREF_NULL if this is not known.
 */
CommandHandler *InterpreterInstance::resolveCommandHandler(RexxString *name)
{
    // all names in the cache are in upper case
    RexxString *upperName = name->upper();
    CommandHandler *handler = (CommandHandler *)commandHandlers->at(upperName);
    if (handler == OREF_NULL)
    {
        handler = new CommandHandler(name->getStringData());
        if (!handler->isResolved())
        {
            return OREF_NULL;   // can't find this
        }
        commandHandlers->put((RexxObject *)handler, upperName);
    }
    return handler;
}


