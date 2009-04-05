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
/******************************************************************************/
/* REXX Kernel                                             PackageClass.cpp   */
/*                                                                            */
/* Primitive Package class                                                    */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "ArrayClass.hpp"
#include "SourceFile.hpp"
#include "DirectoryClass.hpp"
#include "ProtectedObject.hpp"
#include "PackageClass.hpp"
#include "RoutineClass.hpp"
#include "InterpreterInstance.hpp"
#include "PackageManager.hpp"


// singleton class instance
RexxClass *PackageClass::classInstance = OREF_NULL;


/**
 * Create initial class object at bootstrap time.
 */
void PackageClass::createInstance()
{
    CLASS_CREATE(Package, "Package", RexxClass);
}


void *PackageClass::operator new (size_t size)
/******************************************************************************/
/* Function:  create a new method instance                                    */
/******************************************************************************/
{
                                         /* get a new method object           */
    return new_object(size, T_Package);
}


PackageClass::PackageClass(RexxSource *s)
/******************************************************************************/
/* Function:  Initialize a method object                                      */
/******************************************************************************/
{
    OrefSet(this, this->source, s);      /* store the code                    */
}


void PackageClass::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
    memory_mark(this->source);
    memory_mark(this->objectVariables);
}

void PackageClass::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
    memory_mark_general(this->source);
    memory_mark_general(this->objectVariables);
}

void PackageClass::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(PackageClass)

   flatten_reference(newThis->source, envelope);
  flatten_reference(newThis->objectVariables, envelope);

  cleanUpFlatten
}


/**
 * Get the program name of the package
 *
 * @return The name of the original source package.
 */
RexxString *PackageClass::getName()
{
    return source->getProgramName();
}


/**
 * Return all of the source lines for the package, as an array.
 *
 * @return The entire array of source lines.
 */
RexxArray *PackageClass::getSource()
{
    return source->extractSource();
}


/**
 * Extract a specific line from the program source.
 *
 * @param n      The line position.
 *
 * @return The extracted line.
 */
RexxString *PackageClass::getSourceLine(size_t n)
{
    return source->get(n);
}


/**
 * Extract a specific line from the program source.
 *
 * @param n      The line position.
 *
 * @return The extracted line.
 */
RexxString *PackageClass::getSourceLineRexx(RexxObject *position)
{
    // the starting position isn't optional
    size_t n = positionArgument(position, ARG_ONE);
    return source->get(n);
}


/**
 * Get the number of source lines in the package
 *
 * @return the count of lines
 */
RexxInteger *PackageClass::getSourceSize()
{
    return new_integer(source->sourceSize());
}


/**
 * Retrieve all classes defined by this package.
 *
 * @return A directory of all of the classes defined by this package.
 */
RexxDirectory *PackageClass::getClasses()
{
    // we need to return a copy.  The source might necessarily have any of these,
    // so we return an empty directory if it's not there.
    RexxDirectory *classes = source->getInstalledClasses();
    if (classes != OREF_NULL)
    {
        return (RexxDirectory *)classes->copy();
    }
    else
    {
        return new_directory();
    }
}


/**
 * Retrieve all public classes defined by this package.
 *
 * @return A directory of the public classes.
 */
RexxDirectory *PackageClass::getPublicClasses()
{
    // we need to return a copy.  The source might necessarily have any of these,
    // so we return an empty directory if it's not there.
    RexxDirectory *classes = source->getInstalledPublicClasses();
    if (classes != OREF_NULL)
    {
        return (RexxDirectory *)classes->copy();
    }
    else
    {
        return new_directory();
    }
}


/**
 * Retrieve all of the classes imported into this package from
 * other packages.
 *
 * @return A directory of the imported classes.
 */
RexxDirectory *PackageClass::getImportedClasses()
{
    // we need to return a copy.  The source might necessarily have any of these,
    // so we return an empty directory if it's not there.
    RexxDirectory *classes = source->getImportedClasses();
    if (classes != OREF_NULL)
    {
        return (RexxDirectory *)classes->copy();
    }
    else
    {
        return new_directory();
    }
}


/**
 * Get a list of all routines defined by this package.
 *
 * @return A directory of the routines.
 */
RexxDirectory *PackageClass::getRoutines()
{
    // we need to return a copy.  The source might necessarily have any of these,
    // so we return an empty directory if it's not there.
    RexxDirectory *routines = source->getInstalledRoutines();
    if (routines != OREF_NULL)
    {
        return (RexxDirectory *)routines->copy();
    }
    else
    {
        return new_directory();
    }
}


/**
 * Return a directory of the Public routines defined by this
 * package.
 *
 * @return A directory holding the public routines.
 */
RexxDirectory *PackageClass::getPublicRoutines()
{
    // we need to return a copy.  The source might necessarily have any of these,
    // so we return an empty directory if it's not there.
    RexxDirectory *routines = source->getInstalledPublicRoutines();
    if (routines != OREF_NULL)
    {
        return (RexxDirectory *)routines->copy();
    }
    else
    {
        return new_directory();
    }
}


/**
 * Get the directory of routines that have been imported into
 * to this package form other packages.
 *
 * @return A directory of the imported routines.
 */
RexxDirectory *PackageClass::getImportedRoutines()
{
    // we need to return a copy.  The source might necessarily have any of these,
    // so we return an empty directory if it's not there.
    RexxDirectory *routines = source->getImportedRoutines();
    if (routines != OREF_NULL)
    {
        return (RexxDirectory *)routines->copy();
    }
    else
    {
        return new_directory();
    }
}


/**
 * Get all of the unattached methods defined in this package.
 *
 * @return A directory of the unattached methods.
 */
RexxDirectory *PackageClass::getMethods()
{
    // we need to return a copy.  The source might necessarily have any of these,
    // so we return an empty directory if it's not there.
    RexxDirectory *methods = source->getMethods();
    if (methods != OREF_NULL)
    {
        return (RexxDirectory *)methods->copy();
    }
    else
    {
        return new_directory();
    }
}


/**
 * Get all of the packages that have been added to this package
 * context.
 *
 * @return An array of the added packages.
 */
RexxArray *PackageClass::getImportedPackages()
{
    RexxList *packages = source->getPackages();
    if (packages != OREF_NULL)
    {
        return packages->makeArray();
    }
    else
    {
        return new_array((size_t)0);
    }

}


/**
 * Load a package in a source context.
 *
 * @param name   The target package name.
 * @param s      The optional source lines for the package, as an array.
 *
 * @return The loaded package object.
 */
PackageClass *PackageClass::loadPackage(RexxString *name, RexxArray *s)
{
    // make sure we have a valid name and delegate to the source object
    name = stringArgument(name, 1);
    // if no source provided, this comes from a file
    if (s == OREF_NULL)
    {
        return source->loadRequires(ActivityManager::currentActivity, name);
    }
    else
    {
        s = arrayArgument(s, "source");
        return source->loadRequires(ActivityManager::currentActivity, name, s);
    }
}


/**
 * Load a package in a source context.
 *
 * @param name   The target package name.
 *
 * @return The loaded package object.
 */
RexxObject *PackageClass::addPackage(PackageClass *package)
{
    classArgument(package, ThePackageClass, "package");
    source->addPackage(package);
    return this;
}


/**
 * Add a routine to this package's private routine list.
 *
 * @param routine The routine to add.
 *
 * @return The target package object.
 */
RexxObject *PackageClass::addRoutine(RexxString *name, RoutineClass *routine)
{
    name = stringArgument(name, "name");
    classArgument(routine, TheRoutineClass, "routine");
    source->addInstalledRoutine(name, routine, false);
    return this;
}


/**
 * Add a routine to this package's public routine list.
 *
 * @param routine The routine to add.
 *
 * @return The target package object.
 */
RexxObject *PackageClass::addPublicRoutine(RexxString *name, RoutineClass *routine)
{
    name = stringArgument(name, "name");
    classArgument(routine, TheRoutineClass, "routine");
    source->addInstalledRoutine(name, routine, true);
    return this;
}


/**
 * Add a class to this package's class list.
 *
 * @param clazz The class to add.
 *
 * @return The target package object.
 */
RexxObject *PackageClass::addClass(RexxString *name, RexxClass *clazz)
{
    name = stringArgument(name, "name");
    classArgument(clazz, TheClassClass, "class");
    source->addInstalledClass(name, clazz, false);
    return this;
}


/**
 * Add a class to this package's public class list.
 *
 * @param clazz The class to add.
 *
 * @return The target package object.
 */
RexxObject *PackageClass::addPublicClass(RexxString *name, RexxClass *clazz)
{
    name = stringArgument(name, "name");
    classArgument(clazz, TheClassClass, "class");
    source->addInstalledClass(name, clazz, true);
    return this;
}


/**
 * Resolve a class in the context of a package.
 *
 * @param name   The required class name.
 *
 * @return The resolved class object.
 */
RexxClass *PackageClass::findClass(RexxString *name)
{
    RexxClass *classObj = source->findClass(name);
    // we need to filter this to always return a class object
    if (classObj != OREF_NULL && classObj->isInstanceOf(TheClassClass))
    {
        return classObj;
    }
    return OREF_NULL;
}


/**
 * Resolve a class in the context of a package.
 *
 * @param name   The required class name.
 *
 * @return The resolved class object.
 */
RexxClass *PackageClass::findClassRexx(RexxString *name)
{
    name = stringArgument(name, "name");
    RexxClass *cls = source->findClass(name);
    if (cls == OREF_NULL)
    {
        return (RexxClass *)TheNilObject;
    }
    return cls;
}


/**
 * Resolve a routine in the context of a package.
 *
 * @param name   The required routine name.
 *
 * @return The resolved routine object.
 */
RoutineClass *PackageClass::findRoutine(RexxString *name)
{
    return source->findRoutine(name);
}


/**
 * Resolve a routine in the context of a package.
 *
 * @param name   The required routine name.
 *
 * @return The resolved routine object.
 */
RoutineClass *PackageClass::findRoutineRexx(RexxString *name)
{
    name = stringArgument(name, "name");
    RoutineClass *routine = findRoutine(name);
    if (routine == OREF_NULL)
    {
        return (RoutineClass *)TheNilObject;
    }
    return routine;
}


/**
 * Set a security manager on a package.
 *
 * @param manager The security manager object.
 *
 * @return The security manager object.
 */
RexxObject *PackageClass::setSecurityManager(RexxObject *manager)
{
    source->setSecurityManager(manager);
    return TheTrueObject;
}


PackageClass *PackageClass::newRexx(
    RexxObject **init_args,            /* subclass init arguments           */
    size_t       argCount)             /* number of arguments passed        */
/******************************************************************************/
/* Function:  Create a new packag from REXX code contained in a file or an    */
/*            array                                                           */
/******************************************************************************/
{
    RexxObject *pgmname;                 /* method name                       */
    RexxObject *_source;                  /* Array or string object            */
    size_t initCount = 0;                /* count of arguments we pass along  */
    RexxActivity *activity = ActivityManager::currentActivity;
    InterpreterInstance *instance = activity->getInstance();

                                         /* break up the arguments            */

    RexxClass::processNewArgs(init_args, argCount, &init_args, &initCount, 2, (RexxObject **)&pgmname, (RexxObject **)&_source);

    PackageClass *package = OREF_NULL;

    /* get the package name as a string   */
    RexxString *nameString = stringArgument(pgmname, "name");
    if (_source == OREF_NULL)
    {
        // if no directly provided source, resolve the name in the global context and have the instance
        // load the file.
        RexxString *resolvedName = instance->resolveProgramName(nameString, OREF_NULL, OREF_NULL);
        package = instance->loadRequires(activity, nameString, resolvedName);
    }
    else
    {
        // add this to the instance context
        RexxArray *sourceArray = arrayArgument(_source, "source");
        package = instance->loadRequires(activity, nameString, sourceArray);
    }

    /* Give new object its behaviour     */
    package->setBehaviour(((RexxClass *)this)->getInstanceBehaviour());
    if (((RexxClass *)this)->hasUninitDefined())
    {
        package->hasUninit();         /* Make sure everyone is notified.   */
    }
    /* now send an INIT message          */
    package->sendMessage(OREF_INIT, init_args, initCount);
    return package;                      /* return the new method             */
}


/**
 * Dynamically load a library package
 *
 * @param name   The required package name.
 *
 * @return True if the package was loaded and resolved, false if
 *         the package could not be loaded.
 */
RexxObject *PackageClass::loadLibrary(RexxString *name)
{
    name = stringArgument(name, "name");
    // have we already loaded this package?
    // may need to bootstrap it up first.
    LibraryPackage *package = PackageManager::loadLibrary(name);
    return package == NULL ? TheFalseObject : TheTrueObject;
}


/**
 * Return the package-defined digits setting
 *
 * @return The digits setting defined for this package.
 */
RexxObject *PackageClass::digits()
{
    return new_integer(source->getDigits());
}


/**
 * Return the package-defined default fuzz setting.
 *
 * @return The package defined fuzz setting.
 */
RexxObject *PackageClass::fuzz()
{
    return new_integer(source->getFuzz());
}


/**
 * Return the package-defined default form setting.
 *
 * @return The default form setting.
 */
RexxObject *PackageClass::form()
{
    return source->getForm() == Numerics::FORM_SCIENTIFIC ? OREF_SCIENTIFIC : OREF_ENGINEERING;
}


/**
 * Return the package-defined default trace setting.
 *
 * @return The string-formatted trace setting.
 */
RexxObject *PackageClass::trace()
{
    return source->getTrace();
}
