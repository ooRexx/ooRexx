/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2024 Rexx Language Association. All rights reserved.    */
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
#include "PackageClass.hpp"
#include "WeakReferenceClass.hpp"
#include "RoutineClass.hpp"
#include "NativeActivation.hpp"
#include <atomic>


static std::atomic<uint32_t> counter(0); // to generate idntfr for concurrency trace

uint32_t InterpreterInstance::getIdntfr()
{
    if (idntfr == 0) idntfr = ++counter;
    return idntfr;
}


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


/**
 * Constructor for an interpreter instance.
 */
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


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void InterpreterInstance::live(size_t liveMark)
{
    memory_mark(rootActivity);
    memory_mark(allActivities);
    memory_mark(defaultEnvironment);
    memory_mark(searchPath);
    memory_mark(searchExtensions);
    memory_mark(securityManager);
    memory_mark(localEnvironment);
    memory_mark(commandHandlers);
    memory_mark(requiresFiles);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void InterpreterInstance::liveGeneral(MarkReason reason)
{
    memory_mark_general(rootActivity);
    memory_mark_general(allActivities);
    memory_mark_general(defaultEnvironment);
    memory_mark_general(searchPath);
    memory_mark_general(searchExtensions);
    memory_mark_general(securityManager);
    memory_mark_general(localEnvironment);
    memory_mark_general(commandHandlers);
    memory_mark_general(requiresFiles);
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
void InterpreterInstance::initialize(Activity *activity, RexxOption *options)
{
    rootActivity = activity;
    allActivities = new_queue();
    searchExtensions = new_array();       // this will be filled in during options processing
    requiresFiles = new_string_table();   // our list of loaded requires packages
    // this gets added to the entire active list.
    allActivities->append(activity);
    // create a default wrapper for this security manager
    securityManager = new SecurityManager(OREF_NULL);
    // set the default system address environment (can be overridden by options)
    defaultEnvironment = SystemInterpreter::getDefaultAddressName();
    // our list of command handlers (must be done before options are processed)
    commandHandlers = new_string_table();

    // associate the thread with this instance
    activity->setupAttachedActivity(this);
    // create a local environment
    localEnvironment = new_directory();
    processOptions(options);
    // when handled originally, we didn't have the exits setup
    // do this now.
    activity->setupExits();
    // do system specific initialization
    sysInstance.initialize(this, options);
    // register the system command handlers for this platform.
    sysInstance.registerCommandHandlers(this);
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
    Activity *activity = attachThread();
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
Activity *InterpreterInstance::attachThread()
{
    // first check for an existing activity
    Activity *oldActivity = findActivity();
    // do we have an activity for this already? There are possible
    // situations here. Normally, this will be a case where the thread
    // exits the interpreter and calls external code that is then
    // reattaches to the interpreter using the same interpreter instance.
    // We treat this as a nested  activity and keep the same activity
    // object for use.
    //
    // On Windows, however there's another situation that can occur
    // that's more difficult to handle. When we wait for a semaphore on
    // Windows, we also process the Windows message queue. If the activity
    // is waiting for the kernel semaphore and a
    // dispatched messages make ooRexx API calls on the same thread, then
    // the original activity can get its semaphore posted, but it cannot wake
    // up until the message returns, which might also be sitting waiting to be
    // dispatched. The result is a hang.
    if (oldActivity != OREF_NULL && !oldActivity->isNestable())
    {
        // make sure we mark this as attached...we might be nested and don't want to
        // clean this up until we complete
        oldActivity->nestAttach();
        return oldActivity;
    }

    // we need to get a new activity set up for this particular thread
    // NB: The activity manager handles the special case of an activity
    // on the dispatch queue.
    Activity *activity = ActivityManager::attachThread();

    // The creation of the new activity also made that activity the current
    // thread.

    // resource lock must come AFTER we attach the thread and acquire the
    // kernel lock. We must never try to acquire the kernel lock while holding
    // the resource lock.
    ResourceSection lock;
    // add this to the activity lists
    allActivities->append(activity);
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
bool InterpreterInstance::detachThread(Activity *activity)
{
    // if the thread in question is not found or this is not an attached thread,
    // this fails
    if (activity == OREF_NULL || !activity->isAttached())
    {
        return false;
    }

    if (activity->isNestedAttach())
    {
        // if we reused the activity because of a nested callback attach, then
        // we just decrement the nesting count and return without cleaning up
        // any resources.
        activity->returnAttach();
        return true;
    }


    // this activity owned the kernel semaphore before entering here...release it
    // now.
    activity->releaseAccess();
    ResourceSection lock;

    allActivities->removeItem(activity);
    // have the activity manager remove this from the global tables
    // and perform resource cleanup
    ActivityManager::returnActivity(activity);

    // Was this the last detach of an thread?  Signal the shutdown event
    if (allActivities->items() <= 1 && terminating)
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
Activity* InterpreterInstance::spawnActivity(Activity *parent)
{
    // create a new activity
    Activity *activity = ActivityManager::createNewActivity(parent);
    // associate the thread with this instance
    activity->addToInstance(this);
    // add this to the activities list
    ResourceSection lock;

    allActivities->append(activity);
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
bool InterpreterInstance::poolActivity(Activity *activity)
{
    ResourceSection lock;
    // detach from this instance
    activity->detachInstance();
    // remove from the activities lists for the instance
    allActivities->removeItem(activity);
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
Activity* InterpreterInstance::findActivity(thread_id_t threadId)
{
    // this is a critical section
    ResourceSection lock;
    // NB:  New activities are pushed on to the end, so it's prudent to search
    // from the list end toward the front of the list.  Also, this ensures we
    // will find the toplevel activity nested on a given thread first.
    for (size_t listIndex = allActivities->items(); listIndex > 0; listIndex--)
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
 * Find an activity for the current thread that's associated
 * with this activity.
 *
 * @return The target activity.
 */
Activity* InterpreterInstance::findActivity()
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
Activity* InterpreterInstance::enterOnCurrentThread()
{
    // attach this thread to the current activity
    Activity *activity = attachThread();
    // indicate this is a nested entry
    activity->activate();
    // from this point forward, we want to be the active activity, so
    // acquire the kernel lock
    activity->requestApiAccess();
    // return the activity for use
    return activity;
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
        Activity *activity = (Activity *)allActivities->pull();
        // we never terminate the root activity or any activity current in use
        if (activity == rootActivity || activity->isActive())
        {
            allActivities->append(activity);
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
    // check this is in fact a valid instance
    if (!Interpreter::isInstanceActive(this))
    {
        return false;
    }


    // we can't be doing active work on the root thread
    if (rootActivity->isActive())
    {
        return false;
    }

    {

        ResourceSection lock;

        // it's possible to get a call on a second thread or even recursively, let's make sure
        // we're not already in the process of shutting down

        if (terminating)
        {
            return false;
        }

        terminated = false;
        // turn on the global termination in process flag
        terminating = true;

        // go remove all of the activities that are not doing work for this instance
        removeInactiveActivities();
        // if we just have the single root activity left, then we can shutdown
        terminated = allActivities->items() == 1;
    }

    // if there are active threads still running, we need to wait until
    // they all finish
    if (!terminated)
    {
        terminationSem.wait();
    }

    Activity *current;

    try
    {
        // if everything has terminated, then make sure we run the uninits before shutting down.
        // This activity is currently the current activity.  We're going to run the
        // uninits on this one, so reactivate it until we're done running. If we were not actually
        // called on an attached thread, an attach will be performed.
        current = enterOnCurrentThread();

        // this might be holding some local references. Make sure we clear these
        // before running the garbage collector
        rootActivity->clearLocalReferences();

        // before we update of the data structures, make sure we process any
        // pending uninit activity.
        memoryObject.collectAndUninit(Interpreter::lastInstance());

        // do system specific termination of an instance
        sysInstance.terminate();

        // ok, deactivate this again...this will return the activity because the terminating
        // flag is on.
        current->exitCurrentThread();
    }
    // do the release in a catch block to ensure we really release this
    catch (NativeActivation *)
    {
        // ok, deactivate this again...this will return the activity because the terminating
        // flag is on.
        current->exitCurrentThread();

    }

    terminationSem.close();

    // make sure the root activity is removed by the ActivityManager;
    ActivityManager::returnRootActivity(rootActivity);

    // just in case there's still a reference held to this, clear out all object reference fields
    rootActivity = OREF_NULL;
    securityManager = OREF_NULL;
    allActivities = OREF_NULL;
    defaultEnvironment = OREF_NULL;
    searchPath = OREF_NULL;
    searchExtensions = OREF_NULL;
    localEnvironment = OREF_NULL;
    commandHandlers = OREF_NULL;
    requiresFiles = OREF_NULL;


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
        memoryObject.addGlobalReference(o);
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
        memoryObject.addGlobalReference(o);
    }
}


/**
 * Raise a halt condition on all running activities.
 */
bool InterpreterInstance::haltAllActivities(RexxString *name)
{
    // make sure we lock this, since it is possible the table can get updated
    // as a result of setting these flags
    ResourceSection lock;
    bool result = true;

    for (size_t listIndex = 1; listIndex <= allActivities->items(); listIndex++)
    {
        Activity *activity = (Activity *)allActivities->get(listIndex);
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
    // make sure we lock this, since it is possible the table can get updated
    // as a result of setting these flags
    ResourceSection lock;
    for (size_t listIndex = 1; listIndex <= allActivities->items(); listIndex++)
    {
        Activity *activity = (Activity *)allActivities->get(listIndex);
        // only tap the active ones
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
                // while not at the list ender
                for (int i = 0; handlers[i].sysexit_code != RXENDLST; i++)
                {
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
                for (int i = 0; handlers[i].sysexit_code != RXENDLST; i++)
                {
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
                for (int i = 0; handlers[i].registeredName != NULL; i++)
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
                for (int i = 0; handlers[i].name != NULL && handlers[i].handler != NULL; i++)
                {
                    // add the handler to this setup
                    addCommandHandler(handlers[i].name, (REXXPFN)handlers[i].handler, HandlerType::DIRECT);
                }
            }
        }
        // a redirectiong command handler
        else if (strcmp(options->optionName, REDIRECTING_ENVIRONMENTS) == 0)
        {
            RexxRedirectingEnvironment *handlers = (RexxRedirectingEnvironment *)options->option.value.value_POINTER;
            // if we have handlers, initialize the array
            if (handlers != NULL)
            {
                for (int i = 0; handlers[i].name != NULL && handlers[i].handler != NULL; i++)
                {
                    // add the handler to this setup
                    addCommandHandler(handlers[i].name, (REXXPFN)handlers[i].handler, HandlerType::REDIRECTING);
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
        // a package to load at startup
        else if (strcmp(options->optionName, REGISTER_LIBRARY) == 0)
        {
            RexxLibraryPackage *package = (RexxLibraryPackage *)options->option.value.value_POINTER;
            RexxString *libraryName = new_string(package->registeredName);

            // this must load ok in order for this to work
            PackageManager::registerPackage(libraryName, package->table);
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
    return (RexxObject *)localEnvironment->get(name);
}

/**
 * Add a handler to the environment list.
 *
 * @param name       The name of the address environment this services.
 * @param entryPoint The entry point address of the handler.
 * @param type       The category of handler to add.
 */
void InterpreterInstance::addCommandHandler(const char *name, REXXPFN entryPoint, HandlerType::Enum type)
{
    RexxString *handlerName = new_upper_string(name);
    commandHandlers->put(new CommandHandler(entryPoint, type), handlerName);
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
        commandHandlers->put(handler, handlerName);
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
    CommandHandler *handler = (CommandHandler *)commandHandlers->get(upperName);
    if (handler == OREF_NULL)
    {
        handler = new CommandHandler(name->getStringData());
        if (!handler->isResolved())
        {
            return OREF_NULL;   // can't find this
        }
        commandHandlers->put(handler, upperName);
    }
    return handler;
}


/**
 * Retrieve a requires file that might be loaded for this
 * instance.
 *
 * @param name   The name used for the requires file.
 *
 * @return The loaded requires file, or OREF_NULL if this instance
 *         has not used the file yet.
 */
PackageClass *InterpreterInstance::getRequiresFile(Activity *activity, RexxString *name)
{
    WeakReference *ref = (WeakReference *)requiresFiles->get(name);
    if (ref != OREF_NULL)
    {
        PackageClass *resolved = (PackageClass *)ref->get();
        if (resolved != OREF_NULL)
        {
            // get the guard lock on this...this will ensure that
            // the initializer is run before we grab this from the cache
            GuardLock lock(activity, resolved, ThePackageClass);
            return resolved;
        }
        // this was garbage collected, remove it from the table
        requiresFiles->remove(name);
    }
    return OREF_NULL;
}


/**
 * Add a package to our cache, using weak references.
 *
 * @param shortName The shortName (always provided).
 * @param fullName  A second, fully resolved alias name.
 * @param package   The package to add
 */
void InterpreterInstance::addRequiresFile(RexxString *shortName, RexxString *fullName, PackageClass *package)
{
    WeakReference *ref = new WeakReference(package);
    requiresFiles->put(ref, shortName);
    // add under both the short and long names
    if (fullName != OREF_NULL)
    {
        requiresFiles->put(ref, fullName);
    }
}


/**
 * Load a ::requires file into this interpreter instance.
 *
 * @param activity  The current activity we're loading on.,
 * @param shortName The original short name of this package.
 * @param fullName  An expanded fully resolved file name.
 *
 * @return The loaded package class, if located.
 */
PackageClass *InterpreterInstance::loadRequires(Activity *activity, RexxString *shortName, RexxString *fullName)
{
    // if we've already loaded this in this instance, just return it.
    Protected<PackageClass> package = getRequiresFile(activity, shortName);
    if (!package.isNull())
    {
        // check for recursion here. We only need to do this if it's already in the cache
        activity->checkRequires(package->getProgramName());
        return package;
    }

    // if there is a fully resolved full name, check this next
    if (fullName != OREF_NULL)
    {
        // if we've already loaded this in this instance, just return it.
        package = getRequiresFile(activity, fullName);
        if (!package.isNull())
        {
            // check for recursion here. We only need to do this if it's already in the cache
            activity->checkRequires(package->getProgramName());
            // add this to the cache using the short name, since they resolve to the same
            addRequiresFile(shortName, OREF_NULL, package);
            return package;
        }
    }

    // add the package manager to load this
    Protected<PackageClass> p;
    package = PackageManager::loadRequires(activity, shortName, fullName, p);
    // couldn't load this?  report the error
    if (package.isNull())
    {
        reportException(Error_Routine_not_found_requires, shortName);
    }

    // make sure we lock this package until we finish running the requires.
    GuardLock lock(activity, package, ThePackageClass);
    // add this to the instance cache too, under both the long
    // name and the fullName (if it was resolved)
    addRequiresFile(shortName, fullName, package);
    // for any requires file loaded to this instance, we run the prolog within the instance.
    package->runProlog(activity);
    return package;
}


/**
 * Load a ::requires file into this interpreter instance.
 *
 * @param activity  The current activity we're loading on.,
 * @param shortName The original short name of this package.
 * @param fullName  An expanded fully resolved file name.
 *
 * @return The loaded package class, if located.
 */
PackageClass *InterpreterInstance::loadRequires(Activity *activity, RexxString *shortName, ArrayClass *source)
{
    // if we've already loaded this in this instance, just return it.
    PackageClass *package = getRequiresFile(activity, shortName);
    if (package != OREF_NULL)
    {
        return package;
    }

    // add the package manager to load this
    Protected<PackageClass> p;
    package = PackageManager::loadRequires(activity, shortName, source, p);
    // any load failure is an error
    if (package == OREF_NULL)
    {
        reportException(Error_Routine_not_found_requires, shortName);
    }

    // make sure we lock this package until we finish running the requires.
    GuardLock lock(activity, package, ThePackageClass);
    // add this to the instance cache too, under both the long
    // name and the fullName (if it was resolved)
    addRequiresFile(shortName, OREF_NULL, package);

    // for any requires file loaded to this instance, we run the prolog within the instance.
    package->runProlog(activity);

    return package;
}


/**
 * Load a ::requires file into this interpreter instance.
 *
 * @param activity  The current activity we're loading on.,
 * @param shortName The original short name of this package.
 * @param data      The source file data.
 * @param length    The length of the source data.
 *
 * @return The loaded package class, if located.
 */
PackageClass *InterpreterInstance::loadRequires(Activity *activity, RexxString *shortName, const char *data, size_t length)
{
    // if we've already loaded this in this instance, just return it.
    PackageClass *package = getRequiresFile(activity, shortName);
    if (package != OREF_NULL)
    {
        return package;
    }

    // add the package manager to load this
    Protected<PackageClass> p;
    package = PackageManager::loadRequires(activity, shortName, data, length, p);

    if (package == OREF_NULL)
    {
        reportException(Error_Routine_not_found_requires, shortName);
    }

    // make sure we lock this package until we finish running the requires.
    GuardLock lock(activity, package, ThePackageClass);
    // add this to the instance cache too, under both the long
    // name and the fullName (if it was resolved)
    addRequiresFile(shortName, OREF_NULL, package);

    // for any requires file loaded to this instance, we run the prolog within the instance.
    package->runProlog(activity);

    return package;
}




/**
 * Resolve a program for intial loading or a subroutine call.
 *
 * @param _name      The target name.  This can be fully qualified, or a simple name
 *                   without an extension.
 * @param _parentDir The directory of the file of our calling program.  The first place
 *                   we'll look is in the same directory as the program.
 * @param _parentExtension
 *                   The extension our calling program has.  If there is an extension,
 *                   we'll use that version first before trying any of the default
 *                   extensions.
 * @param type       Either RESOLVE_REQUIRES, which requests an additional extension
 *                   to be tried before the standard search order, or RESOLVE_DEFAULT.
 *
 * @return A string version of the file name, if found.  Returns OREF_NULL if
 *         the program cannot be found.
 */
RexxString* InterpreterInstance::resolveProgramName(RexxString *_name, RexxString *_parentDir, RexxString *_parentExtension, ResolveType type)
{
    FileNameBuffer resolvedName;

    const char *name = _name->getStringData();
    const char *parentDir = _parentDir == OREF_NULL ? NULL : _parentDir->getStringData();
    const char *parentExtension = _parentExtension == OREF_NULL ? NULL : _parentExtension->getStringData();
    const char *pathExtension = searchPath == OREF_NULL ? NULL : searchPath->getStringData();

    SysSearchPath searchPath(parentDir, pathExtension);

    // if the file already has an extension, this dramatically reduces the number
    // of searches we need to make.
    if (SysFileSystem::hasExtension(name))
    {
        if (SysFileSystem::searchName(name, searchPath.path, NULL, resolvedName))
        {
            return new_string(resolvedName);
        }
        return OREF_NULL;
    }

    // if we are resolving a REQUIRES file, we first try extension .cls
    if (type == RESOLVE_REQUIRES)
    {
        if (SysFileSystem::searchName(name, searchPath.path, ".cls", resolvedName))
        {
            return new_string(resolvedName);
        }
    }

    // if we have a parent extension provided, use that in preference to any default searches
    if (parentExtension != NULL)
    {
        if (SysFileSystem::searchName(name, searchPath.path, parentExtension, resolvedName))
        {
            return new_string(resolvedName);
        }
    }

    // ok, now time to try each of the individual extensions along the way.
    for (size_t i = 1; i <= searchExtensions->items(); i++)
    {
        RexxString *ext = (RexxString *)searchExtensions->get(i);

        if (SysFileSystem::searchName(name, searchPath.path, ext->getStringData(), resolvedName))
        {
            return new_string(resolvedName);
        }
    }

    // The file may purposefully have no extension.
    if (SysFileSystem::searchName(name, searchPath.path, NULL, resolvedName))
    {
        return new_string(resolvedName);
    }

    return OREF_NULL;
}
