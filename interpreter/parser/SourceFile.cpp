/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                                                */
/*                                                                            */
/* Primitive Translator Source File Class                                     */
/*                                                                            */
/******************************************************************************/
#include <ctype.h>
#include <string.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "DirectoryClass.hpp"
#include "BufferClass.hpp"
#include "RexxActivity.hpp"
#include "RexxActivation.hpp"
#include "SourceFile.hpp"


// start of static methods for creating source objects from different program
// sources.

/**
 * Create a RexxSource object from a name and a program source.
 * This does base initialization for the object, but does not
 * compile the code.
 *
 * @param filename The source file name.
 *
 * @return An initialized RexxSource object prepared for reading from the
 *         file.
 */
RexxSource *RexxSource::createSource(RexxString *filename, ProgramSource *s)
{
    // now attach that to the source object
    RexxSource *source = new RexxSource(filename, s);
    // protect this from GC so we can complete setup.
    ProtectedObject p(source);

    // this can potentially raise an exception
    source->setup();
    return source;
}

/**
 * Create a RexxSource object from a file.  This does
 * base initialization for the object, but does not compile the code.
 *
 * @param filename The source file name.
 *
 * @return An initialized RexxSource object prepared for reading from the
 *         file.
 */
RexxSource *RexxSource::createSource(RexxString *filename)
{
    return createSource(filename,  new FileProgramSource(filename));
}


/**
 * Create a RexxSource object from data already in a buffer.
 * This does base initialization for the object, but does not
 * compile the code.
 *
 * @param filename The source file name...used for naming a
 *                 resolution, but not to actually read a file.
 * @param buffer   The buffer object containing the source data.
 *
 * @return An initialized RexxSource object prepared for reading from the
 *         file.
 */
RexxSource *RexxSource::createSource(RexxString *filename, RexxBuffer *buffer)
{
    return createSource(filename,  new BufferProgramSource(buffer));
}


/**
 * Create a RexxSource object from data contained in an array.
 * This does base initialization for the object, but does not
 * compile the code.
 *
 * @param filename The source file name...used for naming a
 *                 resolution, but not to actually read a file.
 * @param buffer   The buffer object containing the source data.
 *
 * @return An initialized RexxSource object prepared for reading from the
 *         file.
 */
RexxSource *RexxSource::createSource(RexxString *filename, RexxArray *programSource)
{
    return createSource(filename,  new ArrayProgramSource(programSource));
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
  memory_mark(this->parentSource);
  memory_mark(this->sourceArray);
  memory_mark(this->programName);
  memory_mark(this->programDirectory);
  memory_mark(this->programExtension);
  memory_mark(this->programFile);
  memory_mark(this->securityManager);
  memory_mark(this->sourceBuffer);
  memory_mark(this->sourceIndices);
  memory_mark(this->routines);
  memory_mark(this->public_routines);
  memory_mark(this->requires);
  memory_mark(this->libraries);
  memory_mark(this->loadedPackages);
  memory_mark(this->package);
  memory_mark(this->classes);
  memory_mark(this->installed_public_classes);
  memory_mark(this->installed_classes);
  memory_mark(this->merged_public_classes);
  memory_mark(this->merged_public_routines);
  memory_mark(this->methods);
  memory_mark(this->initCode);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxSource::liveGeneral(int reason)
{
#ifndef KEEPSOURCE
  if (memoryObject.savingImage()) {    /* save image time?                  */
                                       /* don't save the source image       */
    OrefSet(this, this->sourceArray, OREF_NULL);
    OrefSet(this, this->sourceBuffer, OREF_NULL);
    OrefSet(this, this->sourceIndices, OREF_NULL);
                                       /* don't save the install information*/
    OrefSet(this, this->methods, OREF_NULL);
    OrefSet(this, this->requires, OREF_NULL);
    OrefSet(this, this->classes, OREF_NULL);
    OrefSet(this, this->routines, OREF_NULL);
    OrefSet(this, this->libraries, OREF_NULL);
    OrefSet(this, this->installed_classes, OREF_NULL);
    OrefSet(this, this->installed_public_classes, OREF_NULL);
    OrefSet(this, this->merged_public_classes, OREF_NULL);
    OrefSet(this, this->merged_public_routines, OREF_NULL);
    this->flags &= ~reclaim_possible;  /* can't recover source immediately  */
  }
#endif
  memory_mark_general(this->sourceArray);
  memory_mark_general(this->parentSource);
  memory_mark_general(this->programName);
  memory_mark_general(this->programDirectory);
  memory_mark_general(this->programExtension);
  memory_mark_general(this->programFile);
  memory_mark_general(this->securityManager);
  memory_mark_general(this->sourceBuffer);
  memory_mark_general(this->sourceIndices);
  memory_mark_general(this->labels);
  memory_mark_general(this->routines);
  memory_mark_general(this->public_routines);
  memory_mark_general(this->requires);
  memory_mark_general(this->libraries);
  memory_mark_general(this->loadedPackages);
  memory_mark_general(this->package);
  memory_mark_general(this->classes);
  memory_mark_general(this->installed_public_classes);
  memory_mark_general(this->installed_classes);
  memory_mark_general(this->merged_public_classes);
  memory_mark_general(this->merged_public_routines);
  memory_mark_general(this->methods);
  memory_mark_general(this->initCode);
}

/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxSource::flatten (RexxEnvelope *envelope)
{
  setUpFlatten(RexxSource)
                                       /* if we are flattening for EA's, we   */
                                       /* don't need to to keep source info   */
                                       /* so ask the envelope if this is a    */
                                       /*  flatten to save the method image   */
    this->sourceArray = OREF_NULL;
    this->sourceBuffer = OREF_NULL;
    this->sourceIndices = OREF_NULL;
    this->securityManager = OREF_NULL;
    flatten_reference(newThis->sourceArray, envelope);
    flatten_reference(newThis->parentSource, envelope);
    flatten_reference(newThis->programName, envelope);
    flatten_reference(newThis->programDirectory, envelope);
    flatten_reference(newThis->programExtension, envelope);
    flatten_reference(newThis->programFile, envelope);
    flatten_reference(newThis->securityManager, envelope);
    flatten_reference(newThis->sourceBuffer, envelope);
    flatten_reference(newThis->sourceIndices, envelope);
    flatten_reference(newThis->labels, envelope);
    flatten_reference(newThis->routines, envelope);
    flatten_reference(newThis->public_routines, envelope);
    flatten_reference(newThis->requires, envelope);
    flatten_reference(newThis->libraries, envelope);
    flatten_reference(newThis->loadedPackages, envelope);
    flatten_reference(newThis->package, envelope);
    flatten_reference(newThis->classes, envelope);
    flatten_reference(newThis->installed_public_classes, envelope);
    flatten_reference(newThis->installed_classes, envelope);
    flatten_reference(newThis->merged_public_classes, envelope);
    flatten_reference(newThis->merged_public_routines, envelope);
    flatten_reference(newThis->methods, envelope);
    flatten_reference(newThis->initCode, envelope);

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

    OrefSet(this, this->programDirectory, SysFileSystem::extractDirectory(programName));
    OrefSet(this, this->programExtension, SysFileSystem::extractExtension(programName));
    OrefSet(this, this->programFile, SysFileSystem::extractFile(programName));
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
    OrefSet(this, this->programName, name);
    extractNameInformation();
}

void RexxSource::interpretLine(size_t _line_number)
/******************************************************************************/
/* Arguments:  interpret line location                                        */
/*                                                                            */
/* Function:  Adjust the source object so that it thinks it is scanning a     */
/*            1-line source file with a line number other than 1 so that      */
/*            errors and trace of an interpreted instruction will display     */
/*            the interpret instructions line number.                         */
/******************************************************************************/
{
                                       /* fill in the source size           */
  this->line_count = _line_number;     /* size is now the line number       */
  this->line_number = _line_number;    /* we are now on line "nn of nn"     */
                                       /* remember for positioning          */
  this->interpret_adjust = _line_number - 1;
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
RexxString *RexxSource::get(size_t _position)
{
    // the source object does the heavy lifting here.
    return souce->getStringLine(position);
}

                                       /* extra space required to format a  */
                                       /* result line.  This overhead is    */
                                       /* 8 leading spaces for the line     */
                                       /* number, + 1 space + length of the */
                                       /* message prefix (3) + 1 space +    */
                                       /* 2 for an indent + 2 for the       */
                                       /* quotes surrounding the value      */
#define TRACE_OVERHEAD 16
                                       /* overhead for a traced instruction */
                                       /* (8 digit line number, blank,      */
                                       /* 3 character prefix, and a blank   */
#define INSTRUCTION_OVERHEAD 11
#define LINENUMBER 6                   /* size of a line number             */
#define PREFIX_OFFSET (LINENUMBER + 1) /* location of the prefix field      */
#define PREFIX_LENGTH 3                /* length of the prefix flag         */
#define INDENT_SPACING 2               /* spaces per indentation amount     */


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
    RexxString  *buffer;                 /* buffer for building result        */
    RexxString  *line;                   /* actual line data                  */
    size_t       outlength;              /* output length                     */
    char        *linepointer;            /* pointer to the line number        */
    char         linenumber[11];         /* formatted line number             */

    // format the line number as a string
    sprintf(linenumber,"%lu", location.getLineNumber());

    // get the line from the source string...this can return "" if the source is
    // not available or this string is somehow out of bounds.
    line = source->extract(location);

    // not available...we provide some sort of information about what is there, even
    // if we can't display the source line.
    if (line == OREF_NULLSTRING)
    {
        // old space code means this is part of the interpreter image.  Don't include
        // the package name in the message
        if (this->isOldSpace())
        {
            line = ActivityManager::currentActivity->buildMessage(Message_Translations_internal_code, new_array((size_t)0));
        }
        // if we have an activation (and we should, since the only time we won't would be for a
        // translation time error...and we have source then), ask it to provide a line describing
        // the invocation situation
        if (activation != OREF_NULL)
        {
            line = activation->formatSourcelessTraceLine(isInternalCode() ? OREF_REXX : this->programName);
        }
        // this could be part of the internal code...give a generic message that doesn't identify
        // the actual package.
        else if (this->isInternalCode())
        {
            line = ActivityManager::currentActivity->buildMessage(Message_Translations_internal_code, new_array((size_t)0));
        }
        else
        {
            // generic package message.
            RexxArray *args = new_array(this->programName);
            ProtectedObject p(args);
            line = ActivityManager::currentActivity->buildMessage(Message_Translations_no_source_available, args);
        }
    }

    ProtectedObject p(line);

    // get a raw empty string so we can build this trace up.
    buffer = raw_string(line->getLength() + INSTRUCTION_OVERHEAD + indent * INDENT_SPACING);
    // blank out the first part
    buffer->set(0, ' ', INSTRUCTION_OVERHEAD + indent * INDENT_SPACING);

    //copy in the source line(s)
    buffer->put(INSTRUCTION_OVERHEAD + indent * INDENT_SPACING, line->getStringData(), line->getLength());

    // now add in the line number.  If the number is two large to add, we just overlay a question mark.
    // I have never seen that happen!
    outlength = strlen(linenumber);
    linepointer = linenumber;
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
RexxArray *RexxSource::extractSource()
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
RexxArray *RexxSource::extractSource(SourceLocation &location )
{
    // the program source handles everything.
    return source->extractSourceLines(location);
}

/**
 * Process an interpret instruction.
 *
 * @param string  The string value to interpret.
 * @param _labels The labels inherited from the parent source context.
 * @param _line_number
 *                The line number of the interpret instruction (used for
 *                line number offsets).
 *
 * @return A translated code object.
 */
RexxCode *RexxSource::interpret(RexxString *string, RexxDirectory *_labels,
    size_t _line_number )
{
    // TODO:  lots of work needed here, specifically creating the source object
    // with a program source configured with the interpret ovvset.


    RexxSource *source = RexxSource::createSource(programName, new_array(string));
    ProtectedObject p(source);
    source->interpretLine(_line_number);  /* fudge the line numbering          */
                                       /* convert to executable form        */
    return source->interpretMethod(_labels);
}


/**
 * Merge a parent source context into our context so all of the
 * bits that are visible in the parent are also resolvable in our
 * context.  This is mostly used for dynamically created methods.
 *
 * @param parent The parent source context.
 */
void RexxSource::inheritSourceContext(RexxSource *source)
{
    // set this as a parent
    OrefSet(this, this->parentSource, source);
}


void RexxSource::mergeRequired(RexxSource *source)
/******************************************************************************/
/* Function:  Merge all public class and routine information from a called    */
/*            program into the full public information of this program.       */
/******************************************************************************/
{
    // has the source already merged in some public routines?  pull those in first,
    // so that the direct set will override
    if (source->merged_public_routines != OREF_NULL)
    {
        /* first merged attempt?             */
        if (this->merged_public_routines == OREF_NULL)
        {
            /* get the directory                 */
            OrefSet(this, this->merged_public_routines, new_directory());
        }
        /* loop through the list of routines */
        for (HashLink i = source->merged_public_routines->first(); source->merged_public_routines->available(i); i = source->merged_public_routines->next(i))
        {
            /* copy the routine over             */
            this->merged_public_routines->setEntry((RexxString *)source->merged_public_routines->index(i), source->merged_public_routines->value(i));
        }

    }

    // now process the direct set
    if (source->public_routines != OREF_NULL)
    {
        /* first merged attempt?             */
        if (this->merged_public_routines == OREF_NULL)
        {
            /* get the directory                 */
            OrefSet(this, this->merged_public_routines, new_directory());
        }
        /* loop through the list of routines */
        for (HashLink i = source->public_routines->first(); source->public_routines->available(i); i = source->public_routines->next(i))
        {
            /* copy the routine over             */
            this->merged_public_routines->setEntry((RexxString *)source->public_routines->index(i), source->public_routines->value(i));
        }
    }


    // now do the same process for any of the class contexts
    if (source->merged_public_classes != OREF_NULL)
    {
        if (this->merged_public_classes == OREF_NULL)
        {
            /* get the directory                 */
            OrefSet(this, this->merged_public_classes, new_directory());
        }
        /* loop through the list of classes, */
        for (HashLink i = source->merged_public_classes->first(); source->merged_public_classes->available(i); i = source->merged_public_classes->next(i))
        {
            /* copy the routine over             */
            this->merged_public_classes->setEntry((RexxString *)source->merged_public_classes->index(i), source->merged_public_classes->value(i));
        }
    }

    // the installed ones are processed second as they will overwrite the imported one, which
    // is the behaviour we want
    if (source->installed_public_classes != OREF_NULL)
    {
        if (this->merged_public_classes == OREF_NULL)
        {
            /* get the directory                 */
            OrefSet(this, this->merged_public_classes, new_directory());
        }
        /* loop through the list of classes, */
        for (HashLink i = source->installed_public_classes->first(); source->installed_public_classes->available(i); i = source->installed_public_classes->next(i))
        {
            /* copy the routine over             */
            this->merged_public_classes->setEntry((RexxString *)source->installed_public_classes->index(i), source->installed_public_classes->value(i));
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
    if (this->routines != OREF_NULL)
    {
        /* try for a local one first         */
        RoutineClass *result = (RoutineClass *)(this->routines->fastAt(name));
        if (result != OREF_NULL)
        {
            return result;
        }
    }

    // we might have a chained context, so check it also
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
    if (this->merged_public_routines != OREF_NULL)
    {
        /* try for a local one first         */
        RoutineClass *result = (RoutineClass *)(this->merged_public_routines->fastAt(name));
        if (result != OREF_NULL)
        {
            return result;
        }
    }

    // we might have a chained context, so check it also
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
    if (this->installed_classes != OREF_NULL)
    {
        /* try for a local one first         */
        RexxClass *result = (RexxClass *)(this->installed_classes->fastAt(name));
        if (result != OREF_NULL)
        {
            return result;
        }
    }

    // we might have a chained context, so check it also
    if (parentSource != OREF_NULL)
    {
        return parentSource->findInstalledClass(name);
    }
    // nope, no got one
    return OREF_NULL;
}


RexxClass *RexxSource::findPublicClass(RexxString *name)
{
    // if we have one locally, then return it.
    if (this->merged_public_classes != OREF_NULL)
    {
        /* try for a local one first         */
        RexxClass *result = (RexxClass *)(this->merged_public_classes->fastAt(name));
        if (result != OREF_NULL)
        {
            return result;
        }
    }

    // we might have a chained context, so check it also
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
    if (this->securityManager != OREF_NULL)
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
    if (this->securityManager != OREF_NULL)
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
 * Perform a non-contextual install of a package.
 */
void RexxSource::install()
{
    if (needsInstallation())
    {
        // In order to install, we need to call something.  We manage this by
        // creating a dummy stub routine that we can call to force things to install
        RexxCode *stub = new RexxCode(this, OREF_NULL, OREF_NULL, 10, FIRST_VARIABLE_INDEX);
        ProtectedObject p2(stub);
        RoutineClass *code = new RoutineClass(programName, stub);
        p2 = code;
        ProtectedObject dummy;
        code->call(ActivityManager::currentActivity, programName, NULL, 0, dummy);
    }
}


void RexxSource::processInstall(
    RexxActivation *activation)        /* invoking activation               */
/******************************************************************************/
/* Function:  Process directive information contained within a method, calling*/
/*            all ::requires routines, creating all ::class methods, and      */
/*            processing all ::routines.                                      */
/******************************************************************************/
{
    /* turn the install flag off         */
    /* immediately, otherwise we may     */
    /* run into a recursion problem      */
    /* when class init methods are       */
    /* processed                         */
    this->flags &= ~_install;            /* we are now installed              */

    // native packages are processed first.  The requires might actually need
    // functons loaded by the packages
    if (this->libraries != OREF_NULL)
    {
        /* classes and routines              */
        // now loop through the requires items
        for (size_t i = libraries->firstIndex(); i != LIST_END; i = libraries->nextIndex(i))
        {
            // and have it do the installs processing
            LibraryDirective *library = (LibraryDirective *)this->libraries->getValue(i);
            library->install(activation);
        }
    }

    // native methods and routines are lazy resolved on first use, so we don't
    // need to process them here.

    if (this->requires != OREF_NULL)     /* need to process ::requires?       */
    {
        /* classes and routines              */
        // now loop through the requires items
        for (size_t i = requires->firstIndex(); i != LIST_END; i = requires->nextIndex(i))
        {
            // and have it do the installs processing.  This is a little roundabout, but
            // we end up back in our own context while processing this, and the merge
            // of the information happens then.
            RequiresDirective *_requires = (RequiresDirective *)this->requires->getValue(i);
            _requires->install(activation);
        }
    }

    // and finally process classes
    if (this->classes != OREF_NULL)
    {
        /* get an installed classes directory*/
        OrefSet(this, this->installed_classes, new_directory());
        /* and the public classes            */
        OrefSet(this, this->installed_public_classes, new_directory());
        RexxArray *createdClasses = new_array(classes->items());

        ProtectedObject p(createdClasses);
        size_t index = 1;       // used for keeping track of install order
        for (size_t i = classes->firstIndex(); i != LIST_END; i = classes->nextIndex(i))
        {
            /* get the class info                */
            ClassDirective *current_class = (ClassDirective *)this->classes->getValue(i);
            // save the newly created class in our array so we can send the activate
            // message at the end
            RexxClass *newClass = current_class->install(this, activation);
            createdClasses->put(newClass, index++);
        }
        // now send an activate message to each of these classes
        for (size_t j = 1; j < index; j++)
        {
            RexxClass *clz = (RexxClass *)createdClasses->get(j);
            clz->sendMessage(OREF_ACTIVATE);
        }
    }
}

void *RexxSource::operator new (size_t size)
/******************************************************************************/
/* Function:  Create a new translator object from an array                    */
/******************************************************************************/
{
    /* Get new object                    */
    return new_object(sizeof(RexxSource), T_RexxSource);
}

/**
 * Parse a trace setting value into a decoded setting
 * and the RexxActivation debug flag set to allow
 * new trace settings to be processed more quickly.
 *
 * @param value      The string source of the trace setting.
 * @param newSetting The returned setting in binary form.
 * @param debugFlags The debug flag representation of the trace setting.
 */
bool RexxSource::parseTraceSetting(RexxString *value, size_t &newSetting, size_t &debugFlags, char &badOption)
{
    size_t setting = TRACE_IGNORE;       /* don't change trace setting yet    */
    size_t debug = DEBUG_IGNORE;         /* and the default debug change      */

    size_t length = value->getLength();  /* get the string length             */
    /* null string?                      */
    if (length == 0)
    {
        setting = TRACE_NORMAL;           /* use default trace setting         */
        debug = DEBUG_OFF;                /* turn off debug mode               */
    }
    else
    {
        /* start at the beginning            */
        /* while more length to process      */
        /* step one each character           */
        for (size_t _position = 0; _position < length; _position++)
        {

            /* process the next character        */
            switch (value->getChar(_position))
            {

                case '?':                      /* debug toggle character            */
                    /* already toggling?                 */
                    if (debug == DEBUG_TOGGLE)
                    {
                        debug = DEBUG_IGNORE;     /* this is back to no change at all  */
                    }
                    else
                    {
                        debug = DEBUG_TOGGLE;     /* need to toggle the debug mode     */
                    }
                    continue;                    /* go loop again                     */

                case 'a':                      /* TRACE ALL                         */
                case 'A':
                    setting = TRACE_ALL;
                    break;

                case 'c':                      /* TRACE COMMANDS                    */
                case 'C':
                    setting = TRACE_COMMANDS;
                    break;

                case 'l':                      /* TRACE LABELS                      */
                case 'L':
                    setting = TRACE_LABELS;
                    break;

                case 'e':                      /* TRACE ERRORS                      */
                case 'E':
                    setting = TRACE_ERRORS;
                    break;

                case 'f':                      /* TRACE FAILURES                    */
                case 'F':
                    setting = TRACE_FAILURES;
                    break;

                case 'n':                      /* TRACE NORMAL                      */
                case 'N':
                    setting = TRACE_NORMAL;
                    break;

                case 'o':                      /* TRACE OFF                         */
                case 'O':
                    setting = TRACE_OFF;
                    break;

                case 'r':                      /* TRACE RESULTS                     */
                case 'R':
                    setting = TRACE_RESULTS;
                    break;

                case 'i':                      /* TRACE INTERMEDIATES               */
                case 'I':
                    setting = TRACE_INTERMEDIATES;
                    break;

                default:                       /* unknown trace setting             */
                    // each context handles it's own error reporting, so give back the
                    // information needed for the message.
                    badOption = value->getChar(_position);
                    return false;
                    break;
            }
            break;                           /* non-prefix char found             */
        }
    }
    // return the merged setting
    newSetting = setting | debug;
    // create the activation-specific flags
    debugFlags = RexxActivation::processTraceSetting(newSetting);
    return true;
}


/**
 * Format an encoded trace setting back into human readable form.
 *
 * @param setting The source setting.
 *
 * @return The string representation of the trace setting.
 */
RexxString * RexxSource::formatTraceSetting(size_t source)
{
    char         setting[3];             /* returned trace setting            */
    setting[0] = '\0';                   /* start with a null string          */
                                         /* debug mode?                       */
    if (source & DEBUG_ON)
    {
        setting[0] = '?';                  /* add the question mark             */
                                           /* add current trace option          */
        setting[1] = (char)source&TRACE_SETTING_MASK;
        /* create a string form              */
        return new_string(setting, 2);
    }
    else                                 /* no debug prefix                   */
    {
        /* add current trace option          */
        setting[0] = (char)source&TRACE_SETTING_MASK;
        /* create a string form              */
        return new_string(setting, 1);
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
PackageClass *RexxSource::loadRequires(RexxActivity *activity, RexxString *target, RexxArray *s)
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
        loadedPackages = new_list();
    }
    else
    {
        // we only add a given package item once.
        if (loadedPackages->hasItem(p) == TheTrueObject)
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
        OrefSet(this, this->package, new PackageClass(this));
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
    if (installed_classes == OREF_NULL)
    {
        OrefSet(this, installed_classes, new_directory());
    }
    installed_classes->setEntry(name, classObject);
    if (publicClass)
    {
        // make sure we have this created also
        if (installed_public_classes == OREF_NULL)
        {
            OrefSet(this, installed_public_classes, new_directory());
        }
        installed_public_classes->setEntry(name, classObject);
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
        OrefSet(this, routines, new_directory());
    }
    routines->setEntry(name, routineObject);
    if (publicRoutine)
    {
        // make sure we have this created
        if (public_routines == OREF_NULL)
        {
            OrefSet(this, public_routines, new_directory());
        }
        public_routines->setEntry(name, routineObject);
    }
}



