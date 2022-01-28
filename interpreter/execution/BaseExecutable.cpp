/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2022 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                                                */
/*                                                                            */
/* Base classes for executable objects                                        */
/*                                                                            */
/******************************************************************************/

#include "BaseExecutable.hpp"
#include "PackageClass.hpp"
#include "MethodArguments.hpp"
#include "RexxActivation.hpp"
#include "StringTableClass.hpp"


/**
 * Override of the copy function for executable subclasses.  This
 * makes sure that the annotations are copied in the new version.
 *
 * @return The new object copy.
 */
RexxInternalObject *BaseExecutable::copy()
{
    // make a copy of the base object
    BaseExecutable *newObj = (BaseExecutable *)RexxObject::copy();
    // and copy the contents as well
    if (annotations != OREF_NULL)
    {
        newObj->annotations = (StringTable *)annotations->copy();
    }
    return newObj;
}


/**
 * Resolve a class in the context of an executable.
 *
 * @param className The name of the required class.
 *
 * @return The resolve class, or OREF_NULL if not found.
 */
RexxClass *BaseExecutable::findClass(RexxString *className)
{
    return code->findClass(className);
}


/**
 * Set the source object into a routine or method executable.
 * This is generally used to attach a source context to a
 * native method or function defined on a source directive.  Since
 * native functions can be referenced in multiple packages, but are
 * managed in the package manager context, this may end up
 * returning a copy of the executable.
 *
 * @param s      The new source.
 *
 * @return Either the same executable object, or a new copy with the
 *         context set.
 */
BaseExecutable *BaseExecutable::setPackageObject(PackageClass *s)
{
    // set this into a source object context.  If we get a
    // new object returned, we need to make a copy of the base
    // executable object also
    BaseCode *setCode = code->setPackageObject(s);
    // we're cool if these are equal
    if (setCode == code)
    {
        return this;
    }
    // make a copy of this executable, and set the new code into it.
    BaseExecutable *newBase = (BaseExecutable *)this->copy();
    newBase->code = setCode;
    return newBase;
}


/**
 * Retrieve the package from a base executable.
 *
 * @return The associated package object.  If there is no available package
 *         object, this returns .nil.
 */
PackageClass *BaseExecutable::getPackage()
{
    PackageClass *package = code->getPackage();
    return (PackageClass *)resultOrNil(package);
}


/**
 * Retrieve the source lines for a base executable
 *
 * @return An array of the source lines
 */
ArrayClass *BaseExecutable::source()
{
    return code->getSource();
}


/**
 * Detach the source code from an executable package
 */
ProgramSource *BaseExecutable::detachSource()
{
    return code->detachSource();
}


/**
 * Attach a source object to an executable.
 *
 * @param s      The new source object.
 */
void BaseExecutable::attachSource(ProgramSource *s)
{
    code->attachSource(s);
}


/**
 * Common handling static method for processing method or
 * routine source.  This sorts out the string vs. array
 * argument and returns everything as an array if valid.
 *
 * @param source   The input object.
 * @param position The position (for error reporting)
 *
 * @return An array of the source lines.
 */
ArrayClass *BaseExecutable::processExecutableSource(RexxObject *source, const char *position)
{
    Protected<ArrayClass> sourceArray;

    // if this is a string object, then convert to a a single element array.
    if (isString(source))
    {
        sourceArray = new_array((RexxString *)source);
    }
    else
    {
        // request this as an array.  If not convertable, then we'll use it as a string
        sourceArray = source->requestArray();
        // couldn't convert?
        if (sourceArray == (ArrayClass *)TheNilObject)
        {
            // get the string representation
            RexxString *sourceString = source->makeString();
            // still can't convert?  This is an error
            if (sourceString == (RexxString *)TheNilObject)
            {
                return OREF_NULL;
                reportException(Error_Incorrect_method_no_method, position);
            }
            // wrap an array around the value
            sourceArray = new_array(sourceString);
        }
        else
        {
            // validate the array contents are correct.
            stringArrayArgument(sourceArray, position);
        }
    }
    return sourceArray;
}


/**
 * Process the new arguments for either a Routine
 * or Method class.  This decodes all of the arguments
 * and processes them into acceptable forms.  This includes
 * figuring out the different source types and the
 * different source contexts.
 *
 * @param init_args The original argument pointer passed to the new method.
 *                  This will be advanced over any of the arguments we
 *                  consume to leave the remaining arguments to be passed
 *                  to an init method.
 * @param argCount  The count of arguments.  This will be decremented for
 *                  any arguments we use.
 * @param name      The name option of the call.
 * @param sourceArray
 *                  The array that will be used to create this object.
 * @param sourceContext
 *                  The optional source context this should inherit from.
 */
void BaseExecutable::processNewExecutableArgs(RexxObject **&init_args, size_t &argCount, RexxString *&name,
     Protected<ArrayClass> &sourceArray, PackageClass *&sourceContext)
{
    RexxObject *pgmname;                 // method name
    RexxObject *source;                  // Array or string object

    // do the initial parse of the new arguments.
    RexxClass::processNewArgs(init_args, argCount, init_args, argCount, 2, pgmname, (RexxObject **)&source);
    // get the method name as a string
    name = stringArgument(pgmname, "name");
    // make sure there is something for the second arg.
    requiredArgument(source, "source");

    // figure out the source section.
    sourceArray = processExecutableSource(source, "source");

    // if not a valid source, give an error
    if (sourceArray == OREF_NULL)
    {
        reportException(Error_Incorrect_method_no_method, "source");
    }

    // now process an optional sourcecontext argument
    sourceContext = OREF_NULL;
    // retrieve extra parameter if exists
    // if not explicitly specified, then the program scope is used
    if (argCount == 0)
    {
        // using the calling source context, so get the package from the top activation if
        // there is one.
        // see if we have an active context and use the current source as the basis for the lookup
        RexxActivation *currentContext = ActivityManager::currentActivity->getCurrentRexxFrame();
        if (currentContext != OREF_NULL)
        {
            sourceContext = currentContext->getPackage();
        }
    }
    else
    {
        RexxObject *option;
        // parse off an additional argument
        RexxClass::processNewArgs(init_args, argCount, init_args, argCount, 1, option, NULL);
        // if there are more than 3 options passed, it is possible this one was omitted
        // we're don
        if (option == OREF_NULL)
        {
            return;
        }

        if (isOfClass(Method, option) || isOfClass(Routine, option))
        {
            sourceContext = ((BaseExecutable *)option)->getPackage();
        }
        else if (isOfClass(Package, option))
        {
            sourceContext = (PackageClass *)option;
        }
        else
        {
            // this must be a string (or convertable) and have a specific value
            option = option->requestString();
            if (option == TheNilObject)
            {
                reportException(Error_Incorrect_method_argType, IntegerThree, "Method, Routine, Package, or String object");
            }
            // default given? set option to NULL (see code below)
            if (!((RexxString *)option)->strCaselessCompare("PROGRAMSCOPE"))
            {
                reportException(Error_Incorrect_call_list, "NEW", IntegerThree, "\"PROGRAMSCOPE\", Method, Routine, or Package object", option);
            }

            // using the calling source context, so get the package from the top activation if
            // there is one.
            // see if we have an active context and use the current source as the basis for the lookup
            RexxActivation *currentContext = ActivityManager::currentActivity->getCurrentRexxFrame();
            if (currentContext != OREF_NULL)
            {
                sourceContext = currentContext->getPackage();
            }
        }
    }
}


/**
 * Process the newFile() arguments for either a Routine
 * or Method class.  This decodes all of the arguments
 * and processes them into acceptable forms.  This includes
 * figuring out the different source contexts.
 *
 * @param filename  The filename option of the call.
 * @param sourceContext
 *                  The optional source context this should inherit from.
 */
void BaseExecutable::processNewFileExecutableArgs(RexxString *&filename,
     PackageClass *&sourceContext)
{
    // get the filename as a string
    filename = stringArgument(filename, "name");

    // now process an optional sourcecontext argument
    if (sourceContext == OREF_NULL)
    {
        // using the calling source context, so get the package from the top activation if
        // there is one.
        // see if we have an active context and use the current source as the basis for the lookup
        RexxActivation *currentContext = ActivityManager::currentActivity->getCurrentRexxFrame();
        if (currentContext != OREF_NULL)
        {
            sourceContext = currentContext->getPackage();
        }
    }

    if (isOfClass(Package, sourceContext))
    {
        return;
    }

    if (isOfClass(Method, sourceContext) || isOfClass(Routine, sourceContext))
    {
        sourceContext = ((BaseExecutable *)sourceContext)->getPackage();
    }
    else
    {
        // this must be a string (or convertable) and have a specific value
        RexxString *source = sourceContext->requestString();
        if (source == TheNilObject)
        {
            reportException(Error_Incorrect_method_argType, IntegerTwo, "Method, Routine, Package, or String object");
        }
        // default given?
        if (!(source->strCaselessCompare("PROGRAMSCOPE")))
        {
            reportException(Error_Incorrect_call_list, "NEWFILE", IntegerTwo, "\"PROGRAMSCOPE\", Method, Routine, or Package object", source);
        }

        // using the calling source context, so get the package from the top activation if
        // there is one.
        // see if we have an active context and use the current source as the basis for the lookup
        RexxActivation *currentContext = ActivityManager::currentActivity->getCurrentRexxFrame();
        if (currentContext != OREF_NULL)
        {
            sourceContext = currentContext->getPackage();
        }
    }
}


/**
 * Retrieve the annotations table for an executable object.
 *
 * @return The executable annotations.
 */
StringTable *BaseExecutable::getAnnotations()
{
    // this is a user-modifiable table.  If we have no
    // table created, then add one to this package.
    if (annotations == OREF_NULL)
    {
        setField(annotations, new_string_table());
    }

    return annotations;
}


/**
 * The Rexx stub for the get annotation method
 *
 * @param name   The name of the target annotation.
 *
 * @return The annotation value, or .nil if it does not exist.
 */
RexxObject *BaseExecutable::getAnnotationRexx(RexxObject *name)
{
    return resultOrNil(getAnnotation(stringArgument(name, "name")));
}


/**
 * Get a specific named annotation.
 *
 * @param name   The annotation name
 *
 * @return The annotation value, or OREF_NULL if it doesn't exist.
 */
RexxString *BaseExecutable::getAnnotation(RexxString *name)
{
    if (annotations == OREF_NULL)
    {
        return OREF_NULL;
    }
    return (RexxString *)annotations->entry(name);
}
