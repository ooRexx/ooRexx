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
/* REXX Kernel                                                                */
/*                                                                            */
/* Primitive Package management                                               */
/*                                                                            */
/******************************************************************************/

#include "RexxCore.h"
#include "PackageManager.hpp"
#include "LibraryPackage.hpp"
#include "Interpreter.hpp"
#include "RexxNativeCode.hpp"
#include "DirectoryClass.hpp"
#include "ActivityManager.hpp"
#include "RoutineClass.hpp"
#include "RexxInternalApis.h"
#include "ProtectedObject.hpp"
#include "SecurityManager.hpp"
#include "WeakReferenceClass.hpp"
#include "RexxActivation.hpp"
#include "PackageClass.hpp"

// this first set is the inital image set, which we preserve the references
// to in order to reset the package loads when the Rexx environment shuts down
RexxDirectory *PackageManager::imagePackages = OREF_NULL;
RexxDirectory *PackageManager::imagePackageRoutines = OREF_NULL;
RexxDirectory *PackageManager::imageRegisteredRoutines = OREF_NULL;
RexxDirectory *PackageManager::imageLoadedRequires = OREF_NULL;

RexxDirectory *PackageManager::packages = OREF_NULL;        // our loaded packages
RexxDirectory *PackageManager::packageRoutines = OREF_NULL;     // table of functions loaded from packages
RexxDirectory *PackageManager::registeredRoutines = OREF_NULL;
RexxDirectory *PackageManager::loadedRequires = OREF_NULL;

/**
 * Initialize the package manager global state.
 */
void PackageManager::initialize()
{
    packages = new_directory();               // create the tables for the manager
    packageRoutines = new_directory();
    registeredRoutines = new_directory();
    loadedRequires = new_directory();
    // load the internal library first
    loadInternalPackage(OREF_REXX, rexxPackage);
    loadLibrary(OREF_REXXUTIL); // load the rexxutil package automatically
}


/**
 * Return the information that needs to be saved in the saved
 * image.
 *
 * @return An array of the items added to the saved image.
 */
RexxArray *PackageManager::getImageData()
{

    RexxArray *imageArray = new_array(IMAGE_ARRAY_SIZE);
    imageArray->put(packages, IMAGE_PACKAGES);
    imageArray->put(packageRoutines, IMAGE_PACKAGE_ROUTINES);
    imageArray->put(registeredRoutines, IMAGE_REGISTERED_ROUTINES);
    imageArray->put(loadedRequires, IMAGE_REQUIRES);

    return imageArray;
}


/**
 * Restore the saved image data.
 *
 * @param imageArray The array we placed in the save image originally.
 */
void PackageManager::restore(RexxArray *imageArray)
{
    // The memory manager is not initialized yet, so we just store the references
    // at this point.  A little later, we'll replace these with copies.
    imagePackages = (RexxDirectory *)imageArray->get(IMAGE_PACKAGES);
    imagePackageRoutines = (RexxDirectory *)imageArray->get(IMAGE_PACKAGE_ROUTINES);
    imageRegisteredRoutines = (RexxDirectory *)imageArray->get(IMAGE_REGISTERED_ROUTINES);
    imageLoadedRequires = (RexxDirectory *)imageArray->get(IMAGE_REQUIRES);
}


/**
 * Restore the saved image data to operational status
 */
void PackageManager::restore()
{
    // we use copies of the image directories to avoid old-to-new image problems.
    // this also allows us to restore the environment after interpreter shutdown
    packages = (RexxDirectory *)imagePackages->copy();
    packageRoutines = (RexxDirectory *)imagePackageRoutines->copy();
    registeredRoutines = (RexxDirectory *)imageRegisteredRoutines->copy();
    loadedRequires = (RexxDirectory *)imageLoadedRequires->copy();

    for (HashLink i = packages->first(); packages->available(i); i = packages->next(i))
    {
        // get the next package
        LibraryPackage *package = (LibraryPackage *)packages->value(i);
        // not one of the internal packages, so reload.
        if (!package->isInternal())
        {
            package->reload();
            package->makeInternal();   // make this part of the persistent set now
        }
        else
        {
            // the only internal package is the Rexx one
            package->reload(rexxPackage);
        }
    }
}


/**
 * Normal live marking.
 */
void PackageManager::live(size_t liveMark)
{
    memory_mark(packages);
    memory_mark(packageRoutines);
    memory_mark(registeredRoutines);
    memory_mark(loadedRequires);
}

/**
 * Generalized live marking.
 */
void PackageManager::liveGeneral(int reason)
{
    memory_mark_general(packages);
    memory_mark_general(packageRoutines);
    memory_mark_general(registeredRoutines);
    memory_mark_general(loadedRequires);
}


/**
 * Resolve a named package, dynamically loading the package
 * if not already in the table.
 *
 * @param name   The name of the library associated with this package.
 *
 * @return A resolved package...throws an exception if the package
 *         is not loadable.
 */
LibraryPackage *PackageManager::getLibrary(RexxString *name)
{
    // have we already loaded this package?
    // may need to bootstrap it up first.
    LibraryPackage *package = loadLibrary(name);
    if (package == NULL)
    {
        // this is an error
        reportException(Error_Execution_library, name);
    }
    return package;
}


/**
 * Attempt to load a library without raising an error.  Returns
 * a LibraryPackage object for the library if the load was successful.
 *
 * @param name   The target library name.
 *
 * @return A LibraryPackage object for the library, or OREF_NULL if was
 *         not resolvable.
 */
LibraryPackage *PackageManager::loadLibrary(RexxString *name)
{
    // have we already loaded this package?
    // may need to bootstrap it up first.
    LibraryPackage *package = (LibraryPackage *)packages->at(name);
    if (package == NULL)
    {
        package = new LibraryPackage(name);
        // add this to our package list.
        packages->put((RexxObject *)package, name);
        // now force the package to load.
        if (!package->load())
        {
             // unable to load the library, so remove this and return NULL.
             packages->remove(name);
             return OREF_NULL;
        }
    }
    return package;
}


/**
 * Create a Native method from a registered package.
 *
 * @param packageName
 *                   The name of the package the library is loaded from.
 *
 * @param methodName The name of the procedure to resolve from the package.
 *
 * @return A Native method that represents this package entry.  Returns
 *         NULL if not found.
 */
RexxNativeMethod *PackageManager::resolveMethod(RexxString *packageName, RexxString *methodName)
{
    // have we already loaded this package?
    // may need to bootstrap it up first.
    LibraryPackage *package = getLibrary(packageName);

    // now see if this can be resolved.
    return package->resolveMethod(methodName);
}


/**
 * Quietly create a Native method from a registered package.
 *
 * @param packageName
 *                   The name of the package the library is loaded from.
 *
 * @param methodName The name of the procedure to resolve from the package.
 *
 * @return A Native method that represents this package entry.  Returns
 *         NULL if not found.
 */
RexxNativeMethod *PackageManager::loadMethod(RexxString *packageName, RexxString *methodName)
{
    // have we already loaded this package?
    // may need to bootstrap it up first.
    LibraryPackage *package = loadLibrary(packageName);
    // silently fail this if it couldn't load
    if (package == OREF_NULL)
    {
        return OREF_NULL;
    }

    // now see if this can be resolved.
    return package->resolveMethod(methodName);
}



/**
 * Resolve a package function activator.
 *
 * @param function  The function name.
 * @param package   The package/library name.
 * @param procedure Procedure name.  Only used if directly loaded from a library
 *                  file.
 *
 * @return A function activator for this function, if it can be
 *         resolved.
 */
RoutineClass *PackageManager::resolveRoutine(RexxString *function, RexxString *packageName, RexxString *procedure)
{
    // see if we have this one already
    RoutineClass *func = (RoutineClass *)registeredRoutines->at(function);

    // if we have this, then we can return it directly.
    if (func != OREF_NULL)
    {
        return func;
    }

    const char *functionName = function->getStringData();
    const char *libraryName = packageName->getStringData();
    const char *procedureName = procedure->getStringData();

    {
        UnsafeBlock releaser;   // don't hold the block while making the API call
        // go register this (unconditionally....at this point, we don't care if this fails)
        RexxRegisterFunctionDll(functionName, libraryName, procedureName);
    }


    // resolve a registered entry, if we can and add it to the cache
    return createRegisteredRoutine(function);
}


/**
 * Resolve a registered function.
 *
 * @param function  The function name.
 *
 * @return A function activator for this function, if it can be
 *         resolved.
 */
RoutineClass *PackageManager::resolveRoutine(RexxString *function)
{
    // see if we have this one already as a package function
    RoutineClass *func = getLoadedRoutine(function);

    // if we have this, then we can return it directly.
    if (func != OREF_NULL)
    {
        return func;
    }

    // resolve a registered entry, if we can and add it to the cache
    return createRegisteredRoutine(function);
}


/**
 * Resolve a package function.  This goes explicitly to a loaded
 * package to resolve the name rather than relying on the global
 * cache.  This will resolve to the same routine object as the
 * global cache, but this prevents us from picking one a
 * different one in case of a name conflict.
 *
 * @param packageName
 *                 The package name.
 * @param function The function name.
 *
 * @return A routine object for this function.
 */
RoutineClass *PackageManager::resolveRoutine(RexxString *packageName, RexxString *function)
{
    // have we already loaded this package?
    // may need to bootstrap it up first.
    LibraryPackage *package = getLibrary(packageName);

    // now see if this can be resolved.
    return package->resolveRoutine(function);
}


/**
 * Quietly load a package function.  This goes explicitly to a
 * loaded package to resolve the name rather than relying on the
 * global cache.  This will resolve to the same routine object
 * as the global cache, but this prevents us from picking one a
 * different one in case of a name conflict.
 *
 * @param packageName
 *                 The package name.
 * @param function The function name.
 *
 * @return A routine object for this function.
 */
RoutineClass *PackageManager::loadRoutine(RexxString *packageName, RexxString *function)
{
    // have we already loaded this package?
    // may need to bootstrap it up first.
    LibraryPackage *package = loadLibrary(packageName);
    if (package == OREF_NULL)
    {
        return OREF_NULL;
    }

    // now see if this can be resolved.
    return package->resolveRoutine(function);
}



/**
 * Locate an already loaded function.
 *
 * @param function  The function name.
 *
 * @return A function activator for this function, if it can be
 *         resolved.
 */
RoutineClass *PackageManager::getLoadedRoutine(RexxString *function)
{
    // see if we have this one already as a package function
    RoutineClass *func = (RoutineClass *)packageRoutines->at(function);

    // if we have this, then we can return it directly.
    if (func != OREF_NULL)
    {
        return func;
    }

    // see if we have this one already as a registered function
    return (RoutineClass *)registeredRoutines->at(function);
}


/**
 * Create a new registered function entry and add to the
 * function cache.
 *
 * @param function
 *
 * @return
 */
RoutineClass *PackageManager::createRegisteredRoutine(RexxString *function)
{
    REXXPFN entry = NULL;

    const char *functionName = function->getStringData();

    {
        UnsafeBlock releaser;    // don't hold the lock while calling
        // now go resolve this entry pointer
        RexxResolveRoutine(functionName, &entry);
    }

    // this is a failure
    if (entry == NULL)
    {
        return OREF_NULL;
    }

    // create a code handler and add to the cache
    RoutineClass *func = new RoutineClass(function, new RegisteredRoutine(function, (RexxRoutineHandler *)entry));
    registeredRoutines->put(func, function->upper());
    // we got this
    return func;
}


/**
 * Load an internal package into our list.  This does not
 * load a library, but links package routines ones already
 * contained inside the interpreter.
 *
 * @param name   Name of the package...this is probably "REXX" or "REXXUTIL".
 * @param p      The package descriptor with the method and function list.
 */
void PackageManager::loadInternalPackage(RexxString *name, RexxPackageEntry *p)
{
    // load up the package and add it to our cache
    LibraryPackage *package = new LibraryPackage(name, p);
    // have we already loaded this package?
    packages->put((RexxObject *)package, name);
}


/**
 * Register an in-process library package.
 *
 * @param name   The name to register under.
 * @param p      The package table information.
 *
 * @return true if this was successfully registered, false if a package
 *         is already registered under the name.
 */
bool PackageManager::registerPackage(RexxString *name, RexxPackageEntry *p)
{
    // don't replace any already loaded packages
    if (packages->at(name) != OREF_NULL)
    {
        return false;
    }
    // handle like an internal package
    loadInternalPackage(name, p);
    return true;
}



/**
 * Add a function to the package-defined functions table.
 *
 * @param name   The name of the function.
 * @param func
 */
void PackageManager::addPackageRoutine(RexxString *name, RoutineClass *func)
{
    packageRoutines->put(func, name);
}


/**
 * Process the basics of RxFuncAdd().  This will return true
 * if the function can be resolved and is callable, false
 * otherwise.  If the target function is not in a loadable
 * package file, this will also do the global registration.
 *
 * @param name   The name of the registered function.
 * @param module The name of the library containing the function.
 * @param proc   The target procedure name (ignored if the target library
 *               is a self-loading one).
 *
 * @return True if the function registration worked and the function
 *         is callable.  False otherwise.
 */
RexxObject *PackageManager::addRegisteredRoutine(RexxString *name, RexxString *module, RexxString *proc)
{
    // make sure we're using uppercase name versions here.
    name = name->upper();
    ProtectedObject p1(name);

    // see if we have this one already, either from a package or previously loaded
    RoutineClass *func = getLoadedRoutine(name);

    // if we have this, then we can return it directly.
    if (func != OREF_NULL)
    {
        return TheFalseObject;
    }

    // see if this package is resolveable/loadable.
    LibraryPackage *package = loadLibrary(module);
    if (package != OREF_NULL)
    {
        // See if this is resolvable in this context.  If we got it,
        // return True.
        return getLoadedRoutine(name) != OREF_NULL ? TheFalseObject : TheTrueObject;
    }

    // ok, this is not a converted new-style package.  Now try registering the function and
    // resolving it in this process.  This will also add this to the local cache
    return resolveRoutine(name, module, proc) != OREF_NULL ? TheFalseObject : TheTrueObject;
}


/**
 * Drop a registered function.
 *
 * @param name   Name of the registered function.
 *
 * @return True if this was deregistered from the global table, false
 *         otherwise.
 */
RexxObject *PackageManager::dropRegisteredRoutine(RexxString *name)
{
    // we register this using the uppercase name, so make sure we uppercase it
    // before looking in the tables.
    name = name->upper();
    // remove this from the local cache, then remove it from the global function
    // registration.
    registeredRoutines->remove(name);
    const char *functionName = name->getStringData();

    {
        UnsafeBlock releaser;
        // just allow this to pass through to Rexxapi.  If this was truely registered
        // instead of loaded implicitly, this will remove the entry.  Otherwise, it will
        // return false.  Regardless, we leave it in our internal tables until we exit.
        return RexxDeregisterFunction(functionName) == 0 ? TheFalseObject : TheTrueObject;
    }
}


/**
 * The query method backing RxFuncQuery().  This checks both
 * our local tables and the global tables to see if something
 * has been registered.
 *
 * @param name   Target name.
 *
 * @return True if the external function exists in our local tables or
 *         in the global registry.
 */
RexxObject *PackageManager::queryRegisteredRoutine(RexxString *name)
{
    // we register this using the uppercase name, so make sure we uppercase it
    // before looking in the tables.
    name = name->upper();
    // does this name exist in our table?
    if (getLoadedRoutine(name) != OREF_NULL)
    {
        return TheFalseObject;
    }

    const char *functionName = name->getStringData();
    {
        UnsafeBlock releaser;
        // just allow this to pass through to Rexxapi.  If this was truly registered
        // instead of loaded implicitly, it will find it.
        return RexxQueryFunction(functionName) != 0 ? TheTrueObject : TheFalseObject;
    }
}


/**
 * Unload all of the libraries loaded in this Rexx process.
 */
void PackageManager::unload()
{
    // traverse the package table, and force an unload for each library we've loaded up.
    for (HashLink i = packages->first(); packages->available(i); i = packages->next(i))
    {
        // get the next package
        LibraryPackage *package = (LibraryPackage *)packages->value(i);
        // not one of the internal packages, so reload.
        if (!package->isInternal())
        {
            package->unload();
        }
    }

    // now roll back to a copy of the image versions of these directories so we only
    // have the orignal image set once again
    packages = (RexxDirectory *)imagePackages->copy();
    packageRoutines = (RexxDirectory *)imagePackageRoutines->copy();
    registeredRoutines = (RexxDirectory *)imageRegisteredRoutines->copy();
    loadedRequires = (RexxDirectory *)imageLoadedRequires->copy();
}


/**
 * Attempt to call a native code function.  This will call either
 * new-style package functions or old-style registered functions.
 *
 * @param activity  The activity we're running under.
 * @param name      The target name of the routine.
 * @param arguments The function arguments.
 * @param argcount  The argument count.
 * @param result    The return result.
 *
 * @return true if we located and successfully called this function.  false
 *         means the function is not located as a native function.
 */
bool PackageManager::callNativeRoutine(RexxActivity *activity, RexxString *name,
    RexxObject **arguments, size_t argcount, ProtectedObject &result)
{
    // all of our tables use uppercase names...make this a case-insensitive lookup
    name = name->upper();

    // package functions come first
    RoutineClass *function = (RoutineClass *)packageRoutines->at(name);
    if (function != OREF_NULL)
    {
        function->call(activity, name, arguments, argcount, result);
        return true;
    }

    // now check for registered functions.  This will either return a cached value,
    // or resolve a routine for the first time and return the cached value.
    function = resolveRoutine(name);
    if (function != OREF_NULL)
    {
        function->call(activity, name, arguments, argcount, result);
        return true;
    }

    // not one of these
    return false;
}


/**
 * Retrieve a ::REQUIRES file.  This will cache the entries so
 * that the same requires entry is returned for every request.
 *
 * @param activity  The current activity.
 * @param shortName The short name of the package.
 * @param resolvedName
 *                  The fully resolved name of a potential package file.  The short
 *                  name is used for checking in the MacroSpace, the long name
 *                  is used for file searches.
 * @param result    The return package routine.
 *
 * @return The package routine (also returned in the result protected object).
 */
RoutineClass *PackageManager::loadRequires(RexxActivity *activity, RexxString *shortName, RexxString *resolvedName, ProtectedObject &result)
{
    result = OREF_NULL;

    SecurityManager *manager = activity->getEffectiveSecurityManager();
    RexxObject *securityManager = OREF_NULL;

    shortName = manager->checkRequiresAccess(shortName, securityManager);
    // no return means forbidden access to this name.  Just return
    // nothing
    if (shortName == OREF_NULL)
    {
        return OREF_NULL;
    }


    // first check this using the specified name.  Since we need to perform checks in the
    // macro space, it's possible this will be loaded under the simple name.  We'll need to check
    // table again using the fully resolved name afterward.

    RoutineClass *package = checkRequiresCache(shortName, result);
    if (package != OREF_NULL)
    {
        return package;
    }

    unsigned short macroPosition;    // a macrospace position marker

    // we need to look in the macrospace before we try checking for a file-based
    // requires.  The macrospace version uses the original name for all checks.  Once we
    // get to the file-based version, we switch to the full resolved name.
    bool checkMacroSpace = RexxQueryMacro(shortName->getStringData(), &macroPosition) == 0;
    if (checkMacroSpace && (macroPosition == RXMACRO_SEARCH_BEFORE))
    {
        return getMacroSpaceRequires(activity, shortName, result, securityManager);
    }

    // it's possible we don't have a file version of this
    if (resolvedName != OREF_NULL)
    {
        resolvedName = manager->checkRequiresAccess(resolvedName, securityManager);
        // no return means forbidden access to this name.  Just return
        // nothing
        if (resolvedName == OREF_NULL)
        {
            return OREF_NULL;
        }


        // now check again using the longer name
        package = checkRequiresCache(resolvedName, result);
        if (package != OREF_NULL)
        {
            return package;
        }

        // load the file version of this.
        return getRequiresFile(activity, resolvedName, securityManager, result);
    }

    // do the macrospace after checks
    if (checkMacroSpace)
    {
        return getMacroSpaceRequires(activity, shortName, result, securityManager);
    }

    // nothing to return
    return OREF_NULL;
}


/**
 * Retrieve a ::REQUIRES file from the macrospace.
 *
 * @param activity The current activity.
 * @param name     The target name.
 * @param result   The returned Routine object for the package.
 * @param securityManager
 *                 A security manager to associated with the package.
 *
 * @return The located ::REQUIRES file.
 */
RoutineClass *PackageManager::getMacroSpaceRequires(RexxActivity *activity, RexxString *name, ProtectedObject &result, RexxObject *securityManager)
{
    // make sure we're not stuck in a circular reference
    activity->checkRequires(name);
    // unflatten the method and protect it
    RoutineClass *code = RexxActivation::getMacroCode(name);
    result = code;

    if (securityManager == OREF_NULL)
    {
        code->setSecurityManager(securityManager);
    }
    // we place the code in the package table so we have
    // access to it to run the prologue code in other instances
    // We also add this before running the prolog in case another
    // thread tries to load the same thing.
    WeakReference *ref = new WeakReference(code);
    loadedRequires->put(ref, name);

    return code;
}


/**
 * Retrieve a file version of a ::REQUIRES file.
 *
 * @param activity The current activity.
 * @param name     The fully resolved file name.
 * @param result   The return routine object.
 *
 * @return The return Routine instance.
 */
RoutineClass *PackageManager::getRequiresFile(RexxActivity *activity, RexxString *name, RexxObject *securityManager, ProtectedObject &result)
{
    // make sure we're not stuck in a circular reference
    activity->checkRequires(name);
    // try to load this from a previously compiled source file or
    // translate it a new if not.
    RoutineClass *code = RoutineClass::fromFile(name);
    result = code;   // we need to protect this until things are fully resolved.

    if (securityManager == OREF_NULL)
    {
        code->setSecurityManager(securityManager);
    }
    return code;
}


/**
 * Loade a requires file from an in-store source.  NOTE:  This
 * is not cached like the other requires files
 *
 * @param activity The current activity.
 * @param name     The fully resolved file name.
 * @param result   The return routine object.
 *
 * @return The return Routine instance.
 */
RoutineClass *PackageManager::loadRequires(RexxActivity *activity, RexxString *name, const char *data, size_t length, ProtectedObject &result)
{
    // first check this using the specified name.
    RoutineClass *resolved = checkRequiresCache(name, result);
    if (resolved != OREF_NULL)
    {
        return resolved;
    }

    RoutineClass *code = new RoutineClass(name, data, length);
    result = code;

    // we place the code in the package table so we have
    // access to it to run the prologue code in other instances
    // We also add this before running the prolog in case another
    // thread tries to load the same thing.
    WeakReference *ref = new WeakReference(code);
    loadedRequires->put(ref, name);
    return code;
}


/**
 * Loade a requires file from an array source.  NOTE:  This is
 * not cached like the other requires files
 *
 * @param activity The current activity.
 * @param name     The fully resolved file name.
 * @param result   The return routine object.
 *
 * @return The return Routine instance.
 */
RoutineClass *PackageManager::loadRequires(RexxActivity *activity, RexxString *name, RexxArray *data, ProtectedObject &result)
{
    // first check this using the specified name.
    RoutineClass *code = checkRequiresCache(name, result);
    if (code == OREF_NULL)
    {
        code = new RoutineClass(name, data);
        result = code;

        // we place the code in the package table so we have
        // access to it to run the prologue code in other instances
        // We also add this before running the prolog in case another
        // thread tries to load the same thing.
        WeakReference *ref = new WeakReference(code);
        loadedRequires->put(ref, name);
    }
    return code;
}


/**
 * Check for a package already in the requires cache.
 *
 * @param name   The name of the target.
 *
 * @return The PackageClass instance, if any.
 */
RoutineClass *PackageManager::checkRequiresCache(RexxString *name, ProtectedObject &result)
{
    // first check this using the specified name.  Since we need to perform checks in the
    // macro space, it's possible this will be loaded under the simple name.  We'll need to check
    // table again using the fully resolved name afterward.
    WeakReference *requiresRef = (WeakReference *)loadedRequires->get(name);
    if (requiresRef != OREF_NULL)
    {
        RoutineClass *resolved = (RoutineClass *)requiresRef->get();
        if (resolved != OREF_NULL)
        {
            result = resolved;
            return resolved;
        }
        // this was garbage collected, remove it from the table
        loadedRequires->remove(name);
    }
    return OREF_NULL;
}


/**
 * Resolve an entry point for a package method entry (used on a
 * restore or reflatten);
 *
 * @param name   Name of the target method.
 *
 * @return The target entry point.
 */
PNATIVEMETHOD PackageManager::resolveMethodEntry(RexxString *packageName, RexxString *name)
{
    LibraryPackage *package = loadLibrary(packageName);

    // if no entry, something bad has gone wrong
    if (package == NULL)
    {
        reportException(Error_Execution_library_method, name, packageName);
    }
    return package->resolveMethodEntry(name);
}


/**
 * Resolve an entry point for a package function entry (used on
 * a restore or reflatten);
 *
 * @param name   Name of the target function.
 *
 * @return The target entry point.
 */
PNATIVEROUTINE PackageManager::resolveRoutineEntry(RexxString *packageName, RexxString *name)
{
    LibraryPackage *package = loadLibrary(packageName);

    // if no entry, something bad has gone wrong
    if (package == NULL)
    {
        reportException(Error_Execution_library_method, name, packageName);
    }
    return package->resolveRoutineEntry(name);
}


/**
 * Resolve an entry point for a package function entry (used on
 * a restore or reflatten);
 *
 * @param name   Name of the target function.
 *
 * @return The target entry point.
 */
PREGISTEREDROUTINE PackageManager::resolveRegisteredRoutineEntry(RexxString *packageName, RexxString *name)
{
    // if there's no package name, then this is stored in raw name form (I don't believe this should
    // ever happen....but.
    if (packageName == OREF_NULL)
    {
        REXXPFN entry = NULL;

        const char *functionName = name->getStringData();
        {
            UnsafeBlock releaser;
            // now go resolve this entry pointer
            RexxResolveRoutine(functionName, &entry);
        }

        // this is a failure
        if (entry == NULL)
        {
            reportException(Error_Execution_library_routine, name);
        }
        return (PREGISTEREDROUTINE)entry;
    }
    else
    {
        LibraryPackage *package = loadLibrary(packageName);

        // if no entry, something bad has gone wrong
        if (package == NULL)
        {
            reportException(Error_Execution_library_routine, name, packageName);
        }
        return package->resolveRegisteredRoutineEntry(name);
    }
}
