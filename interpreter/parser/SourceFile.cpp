/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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
/******************************************************************************/
/* REXX Kernel                                                                */
/*                                                                            */
/* Primitive Translator Source File Class                                     */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "SourceFile.hpp"
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "StringTable.hpp"
#include "RexxActivity.hpp"
#include "RexxActivation.hpp"
#include "ProgramSource.hpp"
#include "PackageClass.hpp"
#include "SysFileSystem.hpp"
#include "InterpreterInstance.hpp"
#include "RoutineClass.hpp"
#include "ClassDirective.hpp"
#include "LibraryDirective.hpp"
#include "RequiresDirective.hpp"
#include "LanguageParser.hpp"


/**
 * Create a new source package object.
 *
 * @param size   the size of the source object.
 *
 * @return Storage for creating a source object.
 */
void *RexxSource::operator new (size_t size)
{
    return new_object(sizeof(RexxSource), T_RexxSource);
}


/**
 * Initialize a RexxSource instance.
 *
 * @param p      The name of the program we're about to create.
 * @param s      The appropriate ProgramSource object that represents the
 *               program.
 */
RexxSource::RexxSource(RexxString *p, ProgramSource *s)
{
    // NOTE:  For GC purposes, don't do involved processing here that creates lots of objects.
    programName = p;
    source = s;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxSource::live(size_t liveMark)
{
    memory_mark(source);
    memory_mark(parentSource);
    memory_mark(programName);
    memory_mark(programDirectory);
    memory_mark(programExtension);
    memory_mark(programFile);
    memory_mark(securityManager);
    memory_mark(initCode);
    memory_mark(mainExecutable);
    memory_mark(package);
    memory_mark(parentSource);
    memory_mark(routines);
    memory_mark(publicRoutines);
    memory_mark(libraries);
    memory_mark(requires);
    memory_mark(classes);
    memory_mark(dataAssets);
    memory_mark(loadedPackages);
    memory_mark(package);
    memory_mark(unattachedMethods);
    memory_mark(loadedPackages);
    memory_mark(installedPublicClasses);
    memory_mark(installedClasses);
    memory_mark(mergedPublicClasses);
    memory_mark(mergedPublicRoutines);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxSource::liveGeneral(MarkReason reason)
{
    // detach the source if we're preparing the image for a save.
    if (reason == PREPARINGIMAGE)
    {
        detachSource();
    }

    memory_mark_general(source);
    memory_mark_general(parentSource);
    memory_mark_general(programName);
    memory_mark_general(programDirectory);
    memory_mark_general(programExtension);
    memory_mark_general(programFile);
    memory_mark_general(securityManager);
    memory_mark_general(initCode);
    memory_mark_general(mainExecutable);
    memory_mark_general(package);
    memory_mark_general(parentSource);
    memory_mark_general(routines);
    memory_mark_general(publicRoutines);
    memory_mark_general(libraries);
    memory_mark_general(requires);
    memory_mark_general(classes);
    memory_mark_general(dataAssets);
    memory_mark_general(loadedPackages);
    memory_mark_general(package);
    memory_mark_general(unattachedMethods);
    memory_mark_general(loadedPackages);
    memory_mark_general(installedPublicClasses);
    memory_mark_general(installedClasses);
    memory_mark_general(mergedPublicClasses);
    memory_mark_general(mergedPublicRoutines);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxSource::flatten (Envelope *envelope)
{
    setUpFlatten(RexxSource)

    securityManager = OREF_NULL;
    flattenRef(source);
    flattenRef(parentSource);
    flattenRef(programName);
    flattenRef(programDirectory);
    flattenRef(programExtension);
    flattenRef(programFile);
    flattenRef(securityManager);
    flattenRef(initCode);
    flattenRef(mainExecutable);
    flattenRef(package);
    flattenRef(parentSource);
    flattenRef(routines);
    flattenRef(publicRoutines);
    flattenRef(libraries);
    flattenRef(requires);
    flattenRef(classes);
    flattenRef(dataAssets);
    flattenRef(loadedPackages);
    flattenRef(package);
    flattenRef(unattachedMethods);
    flattenRef(loadedPackages);
    flattenRef(installedPublicClasses);
    flattenRef(installedClasses);
    flattenRef(mergedPublicClasses);
    flattenRef(mergedPublicRoutines);

    cleanUpFlatten
}


/**
 * Extract various bits of the source name to give us directory,
 * extension and file portions to be used for searches for additional
 * files.
 */
void RexxSource::extractNameInformation()
{
    if (programName == OREF_NULL)
    {
        return;
    }

    // NOTE:  This is normally done during source translation, but
    // this is also performed after a compiled source restore to update
    // the name to the restored file version.  Therefore, the guarded
    // version needs to be performed.
    setField(programDirectory, SysFileSystem::extractDirectory(programName));
    setField(programExtension, SysFileSystem::extractExtension(programName));
    setField(programFile, SysFileSystem::extractFile(programName));
}


/**
 * Perform pre-parsing setup of the source object.
 */
void RexxSource::setup()
{
    // break up the name into its component pieces
    extractNameInformation();
    // prepare the source object for parsing (can get errors here)
    source->setup();
}


/**
 * Set a program name for this source object.  Usually used after
 * a program restore to update the restored routine object.  This
 * will also update the extension and directory information.
 *
 * @param name   The new program name.
 */
void RexxSource::setProgramName(RexxString *name)
{
    // NOTE...still need the guarded version here.
    setField(programName, name);
    extractNameInformation();
}


/**
 * Return count of lines in the source.  This could be zero
 * if no source is available for this method.
 *
 * @return The current source line count.
 */
size_t RexxSource::sourceSize()
{
    // Get the line count from the source object.  This could be zero if the
    // current object is not holding source at the moment.
    return source->getLineCount();
}


/**
 * Test if the current source context is traceable.
 *
 * @return True if the program has source available, false otherwise.
 */
bool RexxSource::isTraceable()
{
    // the source object knows the score.
    return source->isTraceable();
}


/**
 * Get a line from the current source.  Returns this as a
 * String object (used for Sourcelines()).
 *
 * @param _position The target source line.
 *
 * @return The string version of the source line.  Returns OREF_NULL
 *         if the line is not available.
 */
RexxString *RexxSource::getLine(size_t position)
{
    // the source object does the heavy lifting here.
    return source->getStringLine(position);
}

// extra space required to format a result line.  This overhead is
//
//   8 leading spaces for the line number +
//   1 space +
//   3 (length of the message prefix +
//   1 space +
//   2 for an indent +
//   2 for the quotes surrounding the value
const size_t TRACE_OVERHEAD = 16;

// overhead for a traced instruction
//
//   8 digit line number +
//   1 space +
//   3 character prefix +
//   1 blank
const size_t INSTRUCTION_OVERHEAD = 11;

// size of a line number
const size_t LINENUMBER = 6;

// offset of the prefix information
const size_t PREFIX_OFFSET = (LINENUMBER + 1);
// length of the prefix flag
const size_t PREFIX_LENGTH = 3;
// amount of indent spacing for results lines
const size_t INDENT_SPACING = 2;


/**
 * Format a source line for tracing
 *
 * @param activation The activation of the current running code.  This can be
 *                   null if this is a translation time error.
 * @param location   The source line location.
 * @param indent     The indentation amount to apply to the trace line
 * @param trace      This is a traced line vs. an error line
 *
 * @return A formatted trace line, including headers and indentations.
 */
RexxString *RexxSource::traceBack(RexxActivation *activation, SourceLocation &location,
     size_t indent, bool trace)
{
    char         linenumber[11];         /* formatted line number             */

    // format the line number as a string
    sprintf(linenumber,"%lu", location.getLineNumber());

    // get the line from the source string...this can return "" if the source is
    // not available or this string is somehow out of bounds.
    RexxString *line = source->extract(location);

    // not available...we provide some sort of information about what is there, even
    // if we can't display the source line.
    if (line == OREF_NULLSTRING)
    {
        // old space code means this is part of the interpreter image.  Don't include
        // the package name in the message
        if (isOldSpace())
        {
            line = ActivityManager::currentActivity->buildMessage(Message_Translations_internal_code, new_array((size_t)0));
        }

        // if we have an activation (and we should, since the only time we won't would be for a
        // translation time error...and we have source then), ask it to provide a line describing
        // the invocation situation
        if (activation != OREF_NULL)
        {
            line = activation->formatSourcelessTraceLine(isInternalCode() ? OREF_REXX : programName);
        }

        // this could be part of the internal code...give a generic message that doesn't identify
        // the actual package.
        else if (isInternalCode())
        {
            line = ActivityManager::currentActivity->buildMessage(Message_Translations_internal_code, new_array((size_t)0));
        }
        else
        {
            // generic package message.
            ArrayClass *args = new_array(programName);
            ProtectedObject p(args);
            line = ActivityManager::currentActivity->buildMessage(Message_Translations_no_source_available, args);
        }
    }

    ProtectedObject p(line);

    // get a raw empty string so we can build this trace up.
    RexxString *buffer = raw_string(line->getLength() + INSTRUCTION_OVERHEAD + indent * INDENT_SPACING);
    // blank out the first part
    buffer->set(0, ' ', INSTRUCTION_OVERHEAD + indent * INDENT_SPACING);

    //copy in the source line(s)
    buffer->put(INSTRUCTION_OVERHEAD + indent * INDENT_SPACING, line->getStringData(), line->getLength());

    // now add in the line number.  If the number is two large to add, we just overlay a question mark.
    // I have never seen that happen!
    size_t outlength = strlen(linenumber);
    char *linepointer = linenumber;
    if (outlength > LINENUMBER)
    {
        linepointer += outlength - LINENUMBER;
        *linepointer = '?';                /* overlay a question mark           */
        outlength = LINENUMBER;            /* shorten the length                */
    }
    // copy in the line number
    buffer->put(LINENUMBER - outlength, linepointer, outlength);
    // add the traceback prefix, and we're done.
    buffer->put(PREFIX_OFFSET, "*-*", PREFIX_LENGTH);
    return buffer;
}


/**
 * Extract all of the source from the package.
 *
 * @return An array of the source lines.
 */
ArrayClass *RexxSource::extractSource()
{
    // this location value gets everything.
    SourceLocation location;

    location.setLineNumber(1);
    location.setEndLine(0);
    location.setOffset(0);

    return extractSource(location);
}



/**
 * Extract a range of source lines as defined by the
 * location marker.
 *
 * @param location The location giving the start and end locations to extract.
 *
 * @return An array of all lines (or partial lines), defined by
 *         the extract.
 */
ArrayClass *RexxSource::extractSource(SourceLocation &location )
{
    // the program source handles everything.
    return source->extractSourceLines(location);
}


/**
 * Merge a parent source context into our context so all of the
 * bits that are visible in the parent are also resolvable in our
 * context.  This is mostly used for dynamically created methods.
 *
 * @param parent The parent source context.
 */
void RexxSource::inheritSourceContext(PackageClass *parentPackage)
{
    // set this as a parent
    setField(parentSource, parentPackage->getSourceObject());
}


/**
 * Merge all of the information from inherited packages
 * into this lookup context.
 *
 * @param source The source object we're merging from.
 */
void RexxSource::mergeRequired(RexxSource *source)
/******************************************************************************/
/* Function:  Merge all public class and routine information from a called    */
/*            program into the full public information of this program.       */
/******************************************************************************/
{
    // has the source already merged in some public routines?  pull those in first,
    // so that the direct set will override
    if (source->mergedPublicRoutines != OREF_NULL)
    {
        // first merged attempt?  Create our directory
        if (mergedPublicRoutines == OREF_NULL)
        {
            setField(mergedPublicRoutines, new_string_table());
        }
        // loop through the list of routines
        for (HashLink i = source->mergedPublicRoutines->first(); source->mergedPublicRoutines->available(i); i = source->mergedPublicRoutines->next(i))
        {
            // copy the routine over
            mergedPublicRoutines->setEntry((RexxString *)source->mergedPublicRoutines->index(i), source->mergedPublicRoutines->value(i));
        }

    }

    // now process the direct set
    if (source->publicRoutines != OREF_NULL)
    {
        // first merged attempt?  Create out directory
        if (mergedPublicRoutines == OREF_NULL)
        {
            setField(mergedPublicRoutines, new_string_table());
        }
        // now copy all of the direct routines
        for (HashLink i = source->publicRoutines->first(); source->publicRoutines->available(i); i = source->publicRoutines->next(i))
        {
            mergedPublicRoutines->setEntry((RexxString *)source->publicRoutines->index(i), source->publicRoutines->value(i));
        }
    }


    // now do the same process for any of the class contexts
    if (source->mergedPublicClasses != OREF_NULL)
    {
        if (mergedPublicClasses == OREF_NULL)
        {
            setField(mergedPublicClasses, new_string_table());
        }
        for (HashLink i = source->mergedPublicClasses->first(); source->mergedPublicClasses->available(i); i = source->mergedPublicClasses->next(i))
        {
            mergedPublicClasses->setEntry((RexxString *)source->mergedPublicClasses->index(i), source->mergedPublicClasses->value(i));
        }
    }

    // the installed ones are processed second as they will overwrite the imported one, which
    // is the behaviour we want
    if (source->installedPublicClasses != OREF_NULL)
    {
        if (mergedPublicClasses == OREF_NULL)
        {
            setField(mergedPublicClasses, new_string_table());
        }
        for (HashLink i = source->installedPublicClasses->first(); source->installedPublicClasses->available(i); i = source->installedPublicClasses->next(i))
        {
            mergedPublicClasses->setEntry((RexxString *)source->installedPublicClasses->index(i), source->installedPublicClasses->value(i));
        }
    }
}


/**
 * Resolve a directly defined class object in this or a parent
 * context.
 *
 * @param name   The name we're searching for (all uppercase).
 *
 * @return A resolved class object, if found.
 */
RoutineClass *RexxSource::findLocalRoutine(RexxString *name)
{
    // if we have one locally, then return it.
    if (routines != OREF_NULL)
    {
        /* try for a local one first         */
        RoutineClass *result = (RoutineClass *)(routines->fastAt(name));
        if (result != OREF_NULL)
        {
            return result;
        }
    }

    // we might have a chained context.  We check this after any locally
    // defined ones in this source.
    if (parentSource != OREF_NULL)
    {
        return parentSource->findLocalRoutine(name);
    }
    // nope, no got one
    return OREF_NULL;
}


/**
 * Resolve a public routine in this source context
 *
 * @param name   The target name.
 *
 * @return A resolved Routine object, if found.
 */
RoutineClass *RexxSource::findPublicRoutine(RexxString *name)
{
    // if we have one locally, then return it.
    if (mergedPublicRoutines != OREF_NULL)
    {
        /* try for a local one first         */
        RoutineClass *result = (RoutineClass *)(mergedPublicRoutines->fastAt(name));
        if (result != OREF_NULL)
        {
            return result;
        }
    }

    // we might have a chained context, so check it also
    // The inherited context comes after any directly included
    // context.  In for methods or routines that are created from
    // a parent context, this will be the only thing here.
    if (parentSource != OREF_NULL)
    {
        return parentSource->findPublicRoutine(name);
    }
    // nope, no got one
    return OREF_NULL;
}


/**
 * Resolve a routine from this source files base context.
 *
 * @param routineName
 *               The routine name of interest.
 *
 * @return A RoutineClass instance if located.  Returns OREF_NULL if this
 *         is not known at this level.
 */
RoutineClass *RexxSource::findRoutine(RexxString *routineName)
{
    // These lookups are case insensive, so the table are all created using the opper
    // case names.  Use it once and reuse it.
    RexxString *upperName = routineName->upper();
    ProtectedObject p1(upperName);
    RoutineClass *routineObject = findLocalRoutine(upperName);
    if (routineObject != OREF_NULL)
    {
        return routineObject;
    }

    // now try for one pulled in from ::REQUIRES objects
    return findPublicRoutine(upperName);
}


/**
 * Resolve an external call in the context of the program making the
 * call.  This will use the directory and extension of the context
 * program to modify the search order.
 *
 * @param activity The current activity
 * @param name     The target name
 *
 * @return The fully resolved string name of the target program, if one is
 *         located.
 */
RexxString *RexxSource::resolveProgramName(RexxActivity *activity, RexxString *name)
{
    return activity->getInstance()->resolveProgramName(name, programDirectory, programExtension);
}


/**
 * Resolve a directly defined class object in this or a parent
 * context.
 *
 * @param name   The name we're searching for (all uppercase).
 *
 * @return A resolved class object, if found.
 */
RexxClass *RexxSource::findInstalledClass(RexxString *name)
{
    // if we have one locally, then return it.
    if (installedClasses != OREF_NULL)
    {
        /* try for a local one first         */
        RexxClass *result = (RexxClass *)(installedClasses->fastAt(name));
        if (result != OREF_NULL)
        {
            return result;
        }
    }

    // we might have a chained context, so check it also
    // the parents ones come after ones we define.
    if (parentSource != OREF_NULL)
    {
        return parentSource->findInstalledClass(name);
    }
    // nope, no got one
    return OREF_NULL;
}


/**
 * Find a public class that we might have inherited from
 * our included packages.
 *
 * @param name   The target class name.
 *
 * @return A resolved class object, or OREF_NULL if this cannot be found.
 */
RexxClass *RexxSource::findPublicClass(RexxString *name)
{
    // if we have one locally, then return it.
    if (mergedPublicClasses != OREF_NULL)
    {
        /* try for a local one first         */
        RexxClass *result = (RexxClass *)(mergedPublicClasses->fastAt(name));
        if (result != OREF_NULL)
        {
            return result;
        }
    }

    // we might have a chained context, so check it also
    // The inherited context comes after any directly included
    // context.  In for methods or routines that are created from
    // a parent context, this will be the only thing here.
    if (parentSource != OREF_NULL)
    {
        return parentSource->findPublicClass(name);
    }
    // nope, no got one
    return OREF_NULL;
}


/**
 * Resolve a class from this source file context (including any
 * chained parent contexts).
 *
 * @param className The target name of the class.
 *
 * @return The resolved class object, if any.
 */
RexxClass *RexxSource::findClass(RexxString *className)
{
    RexxString *internalName = className->upper();   /* upper case it                     */
    // check for a directly defined one in the source context chain
    RexxClass *classObject = findInstalledClass(internalName);
    // return if we got one
    if (classObject != OREF_NULL)
    {
        return classObject;
    }
    // now try for public classes we pulled in from other contexts
    classObject = findPublicClass(internalName);
    // return if we got one
    if (classObject != OREF_NULL)
    {
        return classObject;
    }

    // give the security manager a go
    if (securityManager != OREF_NULL)
    {
        classObject = (RexxClass *)securityManager->checkLocalAccess(internalName);
        if (classObject != OREF_NULL)
        {
            return classObject;
        }
    }

    /* send message to .local            */
    classObject = (RexxClass *)(ActivityManager::getLocalEnvironment(internalName));
    if (classObject != OREF_NULL)
    {
        return classObject;
    }

    /* normal execution?                 */
    if (securityManager != OREF_NULL)
    {
        classObject = (RexxClass *)securityManager->checkEnvironmentAccess(internalName);
        if (classObject != OREF_NULL)
        {
            return classObject;
        }
    }

    /* last chance, try the environment  */
    return(RexxClass *)(TheEnvironment->at(internalName));
}


/**
 * Perform a non-contextual install of a package.  This
 * processes the install without calling any leading code
 * section.
 */
void RexxSource::install()
{
    if (needsInstallation())
    {
        // In order to install, we need to call something.  We manage this by
        // creating a dummy stub routine that we can call to force things to install
        Protected<RoutineClass> code = new RoutineClass(programName, new RexxCode(this, OREF_NULL));
        ProtectedObject dummy;
        code->call(ActivityManager::currentActivity, programName, NULL, 0, dummy);
    }
}


/**
 * Process directive information contained within a method, calling
 * all ::requires routines, creating all ::class objects, and
 * loading all required libraries.
 *
 * @param activation
 */
void RexxSource::processInstall(RexxActivation *activation)
{
    // turn the install flag off immediately, otherwise we may
    // run into a recursion problem when class init methods are  processed
    installRequired = false;

    // native packages are processed first.  The requires might actually need
    // functons loaded by the packages
    if (libraries != OREF_NULL)
    {
        // now loop through the requires items
        size_t count = libraries->items();
        for (size_t i = 1; i <= count; i++)
        {
            // and have it do the installs processing
            LibraryDirective *library = (LibraryDirective *)libraries->get(i);
            library->install(activation);
        }
    }

    // native methods and routines are lazy resolved on first use, so we don't
    // need to process them here.

    // do we have requires to process?
    if (requires != OREF_NULL)
    {
        // now loop through the requires items
        size_t count = requires->items();
        for (size_t i = 1; i <= count; i++)
        {
            // and have it do the installs processing.  This is a little roundabout, but
            // we end up back in our own context while processing this, and the merge
            // of the information happens then.
            RequiresDirective *_requires = (RequiresDirective *)requires->get(i);
            _requires->install(activation);
        }
    }

    // and finally process classes
    if (classes != OREF_NULL)
    {
        /* get an installed classes directory*/
        setField(installedClasses, new_string_table());
        /* and the public classes            */
        setField(installedPublicClasses, new_string_table());
        Protected<ArrayClass> createdClasses = new_array(classes->items());

        size_t count = classes->items();
        for (size_t i = 1; i <= count; i++)
        {
            /* get the class info                */
            ClassDirective *current_class = (ClassDirective *)classes->get(i);
            // save the newly created class in our array so we can send the activate
            // message at the end
            RexxClass *newClass = current_class->install(this, activation);
            createdClasses->put(newClass, i);
        }
        // now send an activate message to each of these classes
        count = createdClasses->items();
        for (size_t i = 1; i <= count; i++)
        {
            RexxClass *clz = (RexxClass *)createdClasses->get(i);
            clz->sendMessage(OREF_ACTIVATE);
        }
    }
}


/**
 * Load a ::REQUIRES directive when the source file is first
 * invoked.
 *
 * @param target The name of the ::REQUIRES
 * @param instruction
 *               The directive instruction being processed.
 */
PackageClass *RexxSource::loadRequires(RexxActivity *activity, RexxString *target)
{
    // we need the instance this is associated with
    InterpreterInstance *instance = activity->getInstance();

    // get a fully resolved name for this....we might locate this under either name, but the
    // fully resolved name is generated from this source file context.
    RexxString *fullName = resolveProgramName(activity, target);
    ProtectedObject p(fullName);

    // if we've already loaded this in this instance, just return it.
    PackageClass *packageInstance = instance->loadRequires(activity, target, fullName);

    if (packageInstance == OREF_NULL)       /* couldn't create this?             */
    {
        /* report an error                   */
        reportException(Error_Routine_not_found_requires, target);
    }
    // add this to the source context
    addPackage(packageInstance);
    return packageInstance;
}


/**
 * Load a ::REQUIRES directive from an provided source target
 *
 * @param target The name of the ::REQUIRES
 */
PackageClass *RexxSource::loadRequires(RexxActivity *activity, RexxString *target, ArrayClass *s)
{
    // we need the instance this is associated with
    InterpreterInstance *instance = activity->getInstance();

    // if we've already loaded this in this instance, just return it.
    PackageClass *packageInstance = instance->loadRequires(activity, target, s);

    if (packageInstance == OREF_NULL)             /* couldn't create this?             */
    {
        /* report an error                   */
        reportException(Error_Routine_not_found_requires, target);
    }
    // add this to the source context
    addPackage(packageInstance);
    return packageInstance;
}


/**
 * Add a package to a source file context.  This allows new
 * packages to be imported into a source.
 *
 * @param p
 */
void RexxSource::addPackage(PackageClass *p)
{
    // force the directives to be processed first
    install();
    // we only create this on the first use
    if (loadedPackages == OREF_NULL)
    {
        loadedPackages = new_array();
    }
    else
    {
        // we only add a given package item once.
        if (loadedPackages->hasItem(p))
        {
            return;
        }
    }

    // add this to the list and merge the information
    loadedPackages->append(p);
    // not merge all of the info from the imported package
    mergeRequired(p->getSourceObject());
}


/**
 * Retrieve the package wrapper associated with this source.
 *
 * @return The package instance that fronts for this source in Rexx code.
 */
PackageClass *RexxSource::getPackage()
{
    if (package == OREF_NULL)
    {
        setField(package, new PackageClass(this));
    }
    return package;
}


/**
 * Add an installed class to this source package
 *
 * @param name   The class name
 * @param classObject
 *               The class object
 * @param publicClass
 *               Indicates whether this needs to be added to the public list as well.
 */
void RexxSource::addInstalledClass(RexxString *name, RexxClass *classObject, bool publicClass)
{
    // force the directives to be processed first
    install();
    // make sure we have this created
    if (installedClasses == OREF_NULL)
    {
        setField(installedClasses, new_string_table());
    }
    installedClasses->setEntry(name, classObject);
    if (publicClass)
    {
        // make sure we have this created also
        if (installedPublicClasses == OREF_NULL)
        {
            setField(installedPublicClasses, new_string_table());
        }
        installedPublicClasses->setEntry(name, classObject);
    }
}


/**
 * Add an installed routine to this source package
 *
 * @param name   The routine name
 * @param classObject
 *               The routine object
 * @param publicClass
 *               Indicates whether this needs to be added to the public list as well.
 */
void RexxSource::addInstalledRoutine(RexxString *name, RoutineClass *routineObject, bool publicRoutine)
{
    // force the directives to be processed first
    install();
    // make sure we have this created
    if (routines == OREF_NULL)
    {
        setField(routines, new_string_table());
    }
    routines->setEntry(name, routineObject);
    if (publicRoutine)
    {
        // make sure we have this created
        if (publicRoutines == OREF_NULL)
        {
            setField(publicRoutines, new_string_table());
        }
        publicRoutines->setEntry(name, routineObject);
    }
}


/**
 * Attach a buffered source object to a source that
 * has been saved in sourceless form.  Normally used
 * for instore RexxStart calls.
 *
 * @param s      The Buffer with the source code in original form.
 */
void RexxSource::attachSource(BufferClass *s)
{
    // replace the current source object (likely the dummy one)
    source = new BufferProgramSource(s);
    // Go create the source line indices
    source->setup();
}


/**
 * Convert this package to a sourceless form.
 */
void RexxSource::detachSource()
{
    // replace this with the base program source, which
    // does not return anything.
    source = new ProgramSource();
}


/**
 * Return the full default trace setting for this package.
 *
 * @return The current trace setting formatted into readable form.
 */
RexxString *RexxSource::getTrace()
{
    return LanguageParser::formatTraceSetting(traceSetting);
}
