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
/* Primitive LibraryPackage management                                        */
/*                                                                            */
/******************************************************************************/

#include "RexxCore.h"
#include "LibraryPackage.hpp"
#include "PackageManager.hpp"
#include "Interpreter.hpp"
#include "RexxNativeCode.hpp"
#include "DirectoryClass.hpp"
#include "RoutineClass.hpp"
#include "ProtectedObject.hpp"

/**
 * Create a new LibraryPackage object instance.
 *
 * @param size   Size of the object.
 *
 * @return Pointer to new object storage.
 */
void *LibraryPackage::operator new(size_t size)
{
    return new_object(size, T_LibraryPackage);
}


/**
 * Constructor for a loaded package.
 *
 * @param n      Name of the library associated with this package.  This is
 *               also the name used to load the library when requested.
 */
LibraryPackage::LibraryPackage(RexxString *n)
{
    OrefSet(this, libraryName, n);
}

/**
 * Constructor for a loaded package.
 *
 * @param n      Name of the library associated with this package.  This is
 *               also the name used to load the library when requested.
 * @param m      The package manager that orchestrates the loading operations.
 * @param p      The packag table attached to this package name.
 */
LibraryPackage::LibraryPackage(RexxString *n, RexxPackageEntry *p)
{
    OrefSet(this, libraryName, n);
    ProtectedObject p2(this);
    // store the registered package entry
    package = p;
    // this is an internal package.
    internal = true;
}

/**
 * Normal live marking.
 */
void LibraryPackage::live(size_t liveMark)
{
    memory_mark(libraryName);
    memory_mark(routines);
    memory_mark(methods);
}

/**
 * Generalized live marking.
 */
void LibraryPackage::liveGeneral(int reason)
{
    memory_mark_general(libraryName);
    memory_mark_general(routines);
    memory_mark_general(methods);
}


/**
 * Perform the initial loading of a package.  The loading
 * process involves resolving the external library and
 * attempting to resolve a Rexx package exporter routine
 * in the library.  If the library loads, but does not have
 * the package exporter function, this is a classic library.
 *
 * If we do find a package exporter, then we can load all of
 * the routines immediately.  Method loads are deferred until
 * the first request.
 *
 * @param manager The package manager we're attached to.
 *
 * @return True if we were able to load this as proper ooRexx package
 *         file, false if either step failed.  We do not throw
 *         exceptions here, since these are usually loaded in the
 *         context of operations that return an error result instead
 *         of an exception.
 */
bool LibraryPackage::load()
{
    // try to load the package table.
    package = getPackageTable();
    // if this is NULL, return false to the manager
    if (package == NULL)
    {
        return false;
    }
    // call the loader to get the package tables and set them up.
    loadPackage();
    return true;
}



/**
 * Unload a package library.
 */
void LibraryPackage::unload()
{
    // call an unloader, if we have one.
    if (package->unloader != NULL)
    {
        // go run the dispatcher call
        LibraryUnloaderDispatcher dispatcher(package->unloader);

        ActivityManager::currentActivity->run(dispatcher);
    }
    // the internal packages don't get unloaded because
    // we'll be reusing the definition
    if (loaded && !internal)
    {
        lib.unload();
    }
}


/**
 * Load a library and see if it is possible to retrieve
 * a package entry from the library.
 *
 * @return A package table entry, if possible.  A load failure or
 *         no package loading routines returns NULL.
 */
RexxPackageEntry *LibraryPackage::getPackageTable()
{
    // first try to load the libary
    PACKAGE_LOADER loader;
    // reset the library handle that was saved in the image.
    lib.reset();

    if (!lib.load(libraryName->getStringData()))
    {
        // we don't report an exception here.  This may have
        // just been a probe attempt to see if we're real.  We'll
        // leave the exception decisions up to the package manager.
        return NULL;
    }

    // we're loaded now, vs. just a package fronting a name.
    loaded = true;
    // the try to resolve a package getting structure
    // resolve the function address
    void *entry = lib.getProcedure("RexxGetPackage");
    if (entry == NULL)
    {
        // again, this is not an exception...this could just be
        // a classic style function registration.
        return NULL;
    }
    loader = (PACKAGE_LOADER)entry;
    // call the loader to get the package tables and set them up.
    return (*loader)();
}


/**
 * Load a package with a provided package definition.
 *
 * @param p       The package table entry.
 */
void LibraryPackage::loadPackage()
{
    // verify that this library is compatible
    if (package->requiredVersion != 0 && package->requiredVersion < REXX_CURRENT_INTERPRETER_VERSION)
    {
        reportException(Error_Execution_library_version, libraryName);
    }
    // load the function table
    loadRoutines(package->routines);

    // call a loader, if we have one.
    if (package->loader != NULL)
    {
        // go run the dispatcher call
        LibraryLoaderDispatcher dispatcher(package->loader);

        ActivityManager::currentActivity->run(dispatcher);
    }
}


/**
 * Load all of the routines in a package, registering them with
 * the package manager.
 *
 * @param table   The package table describing this package.
 */
void LibraryPackage::loadRoutines(RexxRoutineEntry *table)
{
    // no routines exported by this package?  Just return without
    // doing anything.
    if (table == NULL)
    {
        return;
    }

    // create a directory of loaded routines
    OrefSet(this, routines, new_directory());

    while (table->style != 0)
    {
        // table names tend to be specified in friendly form, we need to
        // convert them to uppercase because "normal" Rexx function names
        // tend to be uppercase.
        RexxString *target = new_upper_string(table->name);
        RexxString *routineName = new_string(table->name);

        RexxRoutine *func = OREF_NULL;
        if (table->style == ROUTINE_CLASSIC_STYLE)
        {
            func = new RegisteredRoutine(libraryName, routineName, (RexxRoutineHandler *)table->entryPoint);
        }
        else
        {
            func = new RexxNativeRoutine(libraryName, routineName, (PNATIVEROUTINE)table->entryPoint);
        }

        RoutineClass *routine = new RoutineClass(routineName, func);
        // add this to our local table.  Our local table needs to keep the original case,
        // since those will be referenced by ::ROUTINE statements.
        routines->put(routine, routineName);

        // add this to the global function pool
        PackageManager::addPackageRoutine(target, routine);
        // step to the next table entry
        table++;
    }
}



/**
 * Locate a named method entry from the package registration
 * table.
 *
 * @param name   The target name.
 *
 * @return The entry associated with the target entry, if it exists.
 *         NULL indicates a not found condition.
 */
RexxMethodEntry *LibraryPackage::locateMethodEntry(RexxString *name)
{
    RexxMethodEntry *entry = package->methods;

    if (entry != NULL)
    {
        // scan the exported method table for the required method
        while (entry->style != 0)
        {
            // is this one a name match?  Make a method, add it to
            // the table, and return.
            if (name->strCaselessCompare(entry->name))
            {
                return entry;
            }
            entry++;
        }
    }
    return NULL;
}


/**
 * Locate a named function entry from the package registration
 * table.
 *
 * @param name   The target name.
 *
 * @return A pointer to the located function structure.  Returns NULL
 *         if the package doesn't exist.
 */
RexxRoutineEntry *LibraryPackage::locateRoutineEntry(RexxString *name)
{
    RexxRoutineEntry *entry = package->routines;

    if ( entry != NULL )
    {
        // scan the exported method table for the required method
        while (entry->style != 0)
        {
            // is this one a name match?  Make a method, add it to
            // the table, and return.
            if (name->strCaselessCompare(entry->name))
            {
                return entry;
            }
            entry++;
        }
    }
    return NULL;
}


/**
 * Get a NativeCode object for a method associated with a
 * package.
 *
 * @param name   Name of the target method.
 *
 * @return A RexxNativeCode object for this method, if located.
 */
RexxNativeMethod *LibraryPackage::resolveMethod(RexxString *name)
{
    // create our methods table if not yet created.
    if (methods == OREF_NULL)
    {
        OrefSet(this, methods, new_directory());
    }

    // see if this is in the table yet.
    RexxNativeMethod *code = (RexxNativeMethod *)methods->at(name);
    if (code == OREF_NULL)
    {
        // find the package definition
        RexxMethodEntry *entry = locateMethodEntry(name);
        // if we found one with this name, create a native method out of it.
        if (entry != NULL)
        {
            code = new RexxNativeMethod(libraryName, name, (PNATIVEMETHOD)entry->entryPoint);
            methods->put((RexxObject *)code, name);
            return code;
        }
        // This, we know from nothing....
        return OREF_NULL;
    }
    // had this cached already.
    return code;
}


/**
 * Get a Routine object for a method associated with a package.
 *
 * @param name   Name of the target method.
 *
 * @return A RexxNativeCode object for this method, if located.
 */
RoutineClass *LibraryPackage::resolveRoutine(RexxString *name)
{
    // we resolve all of these at load time, so this is either in the table, or it's not.
    return (RoutineClass *)routines->at(name);
}


/**
 * Resolve an entry point for a package method entry (used on a
 * restore or reflatten);
 *
 * @param name   Name of the target method.
 *
 * @return The target entry point.
 */
PNATIVEMETHOD LibraryPackage::resolveMethodEntry(RexxString *name)
{
    // find the package definition
    RexxMethodEntry *entry = locateMethodEntry(name);
    // if no entry, something bad has gone wrong
    if (entry == NULL)
    {
        reportException(Error_Execution_library_method, name, libraryName);
    }
    return (PNATIVEMETHOD)entry->entryPoint;
}


/**
 * Resolve an entry point for a package function entry (used on
 * a restore or reflatten);
 *
 * @param name   Name of the target function.
 *
 * @return The target entry point.
 */
PNATIVEROUTINE LibraryPackage::resolveRoutineEntry(RexxString *name)
{
    // find the package definition
    RexxRoutineEntry *entry = locateRoutineEntry(name);
    // if no entry, something bad has gone wrong
    if (entry == NULL)
    {
        reportException(Error_Execution_library_routine, name, libraryName);
    }

    // style mismatch...this is incompatible
    if (entry->style == ROUTINE_CLASSIC_STYLE)
    {
        reportException(Error_Execution_library_routine, name, libraryName);
    }
    return (PNATIVEROUTINE)entry->entryPoint;
}


/**
 * Resolve an entry point for a package function entry (used on
 * a restore or reflatten);
 *
 * @param name   Name of the target function.
 *
 * @return The target entry point.
 */
PREGISTEREDROUTINE LibraryPackage::resolveRegisteredRoutineEntry(RexxString *name)
{
    // find the package definition
    RexxRoutineEntry *entry = locateRoutineEntry(name);
    // if no entry, something bad has gone wrong
    if (entry == NULL)
    {
        reportException(Error_Execution_library_routine, name, libraryName);
    }

    // style mismatch...this is incompatible
    if (entry->style != ROUTINE_CLASSIC_STYLE)
    {
        reportException(Error_Execution_library_routine, name, libraryName);
    }
    return (PREGISTEREDROUTINE)entry->entryPoint;
}


/**
 * Refresh a non-internal package after an image restore.
 */
void LibraryPackage::reload()
{
    package = getPackageTable();
    if (package == OREF_NULL)
    {
        Interpreter::logicError("Failure loading required base library");
    }
}


/**
 * Refresh an internal package after an image restore.
 *
 * @param pack   The internal package entry.
 */
void LibraryPackage::reload(RexxPackageEntry *pack)
{
    package = pack;
}



/**
 * Process a callout to package loader function
 */
void LibraryLoaderDispatcher::run()
{
    RexxThreadContext *context = activity->getThreadContext();

    loader(context);
}



/**
 * Process a callout to package loader function
 */
void LibraryUnloaderDispatcher::run()
{
    RexxThreadContext *context = activity->getThreadContext();

    unloader(context);
}
