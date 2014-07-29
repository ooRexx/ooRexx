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
/* Base classes for executable objects                                        */
/*                                                                            */
/******************************************************************************/

#include "BaseCode.hpp"
#include "ProtectedObject.hpp"
#include "MethodArguments.hpp"
#include "MethodClass.hpp"
#include "RoutineClass.hpp"
#include "PackageClass.hpp"
#include "RexxActivation.hpp"


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
void BaseExecutable::detachSource()
{
    code->detachSource();
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
ArrayClass *BaseExecutable::processExecutableSource(RexxObject *source, RexxObject *position)
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
                reportException(Error_Incorrect_method_no_method, position);
            }
            // wrap an array around the value
            sourceArray = new_array(sourceString);
        }
        // have an array of strings (hopefully)
        else
        {
            // must be single dimension
            if (!sourceArray->isSingleDimensional())
            {
                reportException(Error_Incorrect_method_noarray, position);
            }

            for (size_t counter = 1; counter <= sourceArray->size(); counter++)
            {
                RexxString *sourceString = sourceArray ->get(counter)->makeString();
                // if this did not convert, this is an error
                if (sourceString == (RexxString *)TheNilObject)
                {
                    reportException(Error_Incorrect_method_nostring_inarray, position);
                }
                else
                {
                    // replace the original item in the array
                    sourceArray->put(sourceString, counter);
                }
            }
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
    size_t initCount = 0;                // count of arguments we pass along

    // do the initial parse of the new arguments.
    RexxClass::processNewArgs(init_args, argCount, &init_args, &initCount, 2, (RexxObject **)&pgmname, (RexxObject **)&source);
    // get the method name as a string
    RexxString *nameString = stringArgument(pgmname, ARG_ONE);
    // make sure there is something for the second arg.
    requiredArgument(source, ARG_TWO);

    // figure out the source section.
    sourceArray = processExecutableSource(source, IntegerTwo);

    // now process an optional sourcecontext argument
    sourceContext = OREF_NULL;
    // retrieve extra parameter if exists
    if (initCount != 0)
    {
        RexxObject *option;
        // parse off an additional argument
        RexxClass::processNewArgs(init_args, initCount, &init_args, &initCount, 1, &option, NULL);
        // if there are more than 3 options passed, it is possible this one was omitted
        // we're don
        if (option == OREF_NULL)
        {
            return;
        }

        if (isOfClass(Method, option))
        {
            sourceContext = ((MethodClass *)option)->getPackage();
        }
        else if (isOfClass(Routine, option))
        {
            sourceContext = ((RoutineClass *)option)->getPackage();
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
                reportException(Error_Incorrect_call_list, "NEW", IntegerThree, "\"PROGRAMSCOPE\", Method, Routine, Package object", option);
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
 * Run this code as a method invocation.
 *
 * @param activity  The current activity.
 * @param method    The method we're invoking.
 * @param receiver  The method target object.
 * @param msgname   The name the method was invoked under.
 * @param argCount  The count of arguments.
 * @param arguments The argument pointer.
 * @param result    The returned result.
 */
void BaseCode::run(Activity *activity, MethodClass *method, RexxObject *receiver, RexxString *msgname, RexxObject **arguments, size_t argCount, ProtectedObject &result)
{
    // The subcasses decide which of run and call are allowed
    reportException(Error_Interpretation);
}


/**
 * Invoke a code element as a call target.  This form is generally
 * only used for calls from Rexx code to Rexx code or for top level
 * program invocation.
 *
 * @param activity  The activity we're running under.
 * @param msgname   The name of the program or name used to invoke the routine.
 * @param arguments The arguments to the method.
 * @param argcount  The count of arguments.
 * @param ct        The call context.
 * @param env       The current address environment.
 * @param context   The type of call being made (program call, internal call, interpret,
 *                  etc.)
 * @param result    The returned result.
 */
void BaseCode::call(Activity *activity, RoutineClass *routine, RexxString *msgname, RexxObject **arguments, size_t argcount, RexxString *ct, RexxString *env, int context, ProtectedObject &result)
{
    // the default for this is the simplified call.   This is used by Rexx code to make calls to
    // both Rexx programs and native routines, so the polymorphism simplifies the processing.
    call(activity, routine, msgname, arguments, argcount, result);
}


/**
 * Simplified call form used for calling from Rexx code to native code.
 *
 * @param activity  The current activity.
 * @param msgname   The name of the call.
 * @param arguments the call arguments.
 * @param argcount  The count of arguments.
 * @param result    The returned result.
 */
void BaseCode::call(Activity *activity, RoutineClass *routine, RexxString *msgname, RexxObject **arguments, size_t argcount, ProtectedObject &result)
{
    // The subcasses decide which of run and call are allowed
    reportException(Error_Interpretation);
}


/**
 * Return source informaton for a BaseCode object.  If not
 * representing an element in a source file, this returns
 * an empty array.
 *
 * @return A null array.
 */
ArrayClass *BaseCode::getSource()
{
                                       /* this is always a null array       */
    return (ArrayClass *)TheNullArray->copy();
}


/**
 * Set the security manager in the code source context.
 *
 * @param manager The new security manager.
 *
 * @return Returns true if the manager could be set.  Non-Rexx code objects
 *         just return false unconditionally.
 */
RexxObject *BaseCode::setSecurityManager(RexxObject *manager)
{
    // the default is just to return a failure
    return TheFalseObject;
}


/**
 * Retrieve the source object associated with a code object.
 *
 * @return
 */
PackageClass *BaseCode::getPackageObject()
{
    return OREF_NULL;
}


/**
 * Default class resolution...which only looks in the environment
 * or .local.
 *
 * @param className The target class name.
 *
 * @return The resolved class object, or OREF_NULL if this is not known.
 */
RexxClass *BaseCode::findClass(RexxString *className)
{
    // the interpreter class handles the default lookups
    return Interpreter::findClass(className);
}



/**
 * Set a source object into a code context.  The default
 * implementation is just to return the same object without
 * setting a source.  This is used mostly for attaching a source
 * context to native code methods and routines defined on
 * directives.
 *
 * @param s      The new source object.
 *
 * @return Either the same object, or a new copy of the code object.
 */
BaseCode *BaseCode::setPackageObject(PackageClass *s)
{
    return this;         // this is just a nop
}


/**
 * Retrieve the package associated with a code object.  Returns
 * OREF_NULL if this code object doesn't have a package source.
 *
 * @return The associated package, or OREF_NULL.
 */
PackageClass *BaseCode::getPackage()
{
    return getPackageObject();
}


/**
 * Detach the source from a code object.
 */
void BaseCode::detachSource()
{
    PackageClass *package = getPackageObject();
    if (package != OREF_NULL)
    {
        package->detachSource();
    }
}

