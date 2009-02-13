/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
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
/* Primitive Method Class                                                     */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "RexxCode.hpp"
#include "RexxNativeCode.hpp"
#include "RexxActivity.hpp"
#include "RexxActivation.hpp"
#include "RexxNativeActivation.hpp"
#include "MethodClass.hpp"
#include "SourceFile.hpp"
#include "DirectoryClass.hpp"
#include "ProtectedObject.hpp"
#include "BufferClass.hpp"
#include "RexxInternalApis.h"
#include "RoutineClass.hpp"
#include "PackageClass.hpp"
#include "Interpreter.hpp"
#include "RexxCode.hpp"
#include "PackageManager.hpp"

// singleton class instance
RexxClass *RexxMethod::classInstance = OREF_NULL;


/**
 * Create initial class object at bootstrap time.
 */
void RexxMethod::createInstance()
{
    CLASS_CREATE(Method, "Method", RexxClass);
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
BaseExecutable *BaseExecutable::setSourceObject(RexxSource *s)
{
    // set this into a source object context.  If we get a
    // new object returned, we need to make a copy of the base
    // executable object also
    BaseCode *setCode = code->setSourceObject(s);
    // we're cool if these are equal
    if (setCode == code)
    {
        return this;
    }
    // make a copy of this executable, and set the new code into it.
    BaseExecutable *newBase = (BaseExecutable *)this->copy();
    OrefSet(newBase, newBase->code, setCode);
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
    if (package == OREF_NULL)
    {
        return (PackageClass *)TheNilObject;
    }
    return package;
}


/**
 * Retrieve the source lines for a base executable
 *
 * @return An array of the source lines
 */
RexxArray *BaseExecutable::source()
{
    return code->getSource();
}



/**
 * Generate a method directly from a source object.
 *
 * @param source The source object.
 */
RexxMethod::RexxMethod(RexxString *name, RexxSource *_source)
{
    // we need to protect this object until the constructor completes.
    // the code generation step will create lots of new objects, giving a
    // pretty high probability that it will be collected.
    ProtectedObject p(this);
    ProtectedObject p2(_source);
    OrefSet(this, this->executableName, name);
    // generate our code object and make the file hook up.
    RexxCode *codeObj = _source->generateCode(true);
    OrefSet(this, this->code, codeObj);
}


/**
 * Initialize a Routine object from a generated code object. Generally
 * used for routines generated from ::METHOD directives.
 *
 * @param name    The routine name.
 * @param codeObj The associated code object.
 */
RexxMethod::RexxMethod(RexxString *name, BaseCode *codeObj)
{
    OrefSet(this, this->executableName, name);
    OrefSet(this, this->code, codeObj);  /* store the code                    */
}


/**
 * Initialize a RexxMethod object from a file source.
 *
 * @param name   The routine name (and the resolved name of the file).
 */
RexxMethod::RexxMethod(RexxString *name)
{
    // we need to protect this object until the constructor completes.
    // the code generation step will create lots of new objects, giving a
    // pretty high probability that it will be collected.
    ProtectedObject p(this);
    OrefSet(this, this->executableName, name);
    // get a source object to generat this from
    RexxSource *_source = new RexxSource(name);
    ProtectedObject p2(_source);
    // generate our code object and make the file hook up.
    RexxCode *codeObj = _source->generateCode(true);
    OrefSet(this, this->code, codeObj);
}


/**
 * Initialize a Routine object using a buffered source.
 *
 * @param name   The name of the routine.
 * @param source the source buffer.
 */
RexxMethod::RexxMethod(RexxString *name, RexxBuffer *buf)
{
    // we need to protect this object until the constructor completes.
    // the code generation step will create lots of new objects, giving a
    // pretty high probability that it will be collected.
    ProtectedObject p(this);
    OrefSet(this, this->executableName, name);
    // get a source object to generat this from
    RexxSource *_source = new RexxSource(name, buf);
    ProtectedObject p2(_source);
    // generate our code object and make the file hook up.
    RexxCode *codeObj = _source->generateCode(true);
    OrefSet(this, this->code, codeObj);
}


/**
 * Initialize a Routine object using directly provided source.
 *
 * @param name   The name of the routine.
 * @param data   The source data buffer pointer.
 * @param length the length of the source buffer.
 */
RexxMethod::RexxMethod(RexxString *name, const char *data, size_t length)
{
    // we need to protect this object until the constructor completes.
    // the code generation step will create lots of new objects, giving a
    // pretty high probability that it will be collected.
    ProtectedObject p(this);
    OrefSet(this, this->executableName, name);
    // get a source object to generat this from
    RexxSource *_source = new RexxSource(name, data, length);
    ProtectedObject p2(_source);
    // generate our code object and make the file hook up.
    RexxCode *codeObj = _source->generateCode(true);
    OrefSet(this, this->code, codeObj);
}


/**
 * Initialize a Routine object using an array source.
 *
 * @param name   The name of the routine.
 * @param source the source buffer.
 */
RexxMethod::RexxMethod(RexxString *name, RexxArray *s)
{
    // we need to protect this object until the constructor completes.
    // the code generation step will create lots of new objects, giving a
    // pretty high probability that it will be collected.
    ProtectedObject p(this);
    OrefSet(this, this->executableName, name);
    // get a source object to generat this from
    RexxSource *_source = new RexxSource(name, s);
    ProtectedObject p2(_source);
    // generate our code object and make the file hook up.
    RexxCode *codeObj = _source->generateCode(true);
    OrefSet(this, this->code, codeObj);
}

void RexxMethod::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
    memory_mark(this->scope);
    memory_mark(this->code);
    memory_mark(this->executableName);
    memory_mark(this->objectVariables);
}

void RexxMethod::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
    memory_mark_general(this->scope);
    memory_mark_general(this->code);
    memory_mark_general(this->executableName);
    memory_mark_general(this->objectVariables);
}

void RexxMethod::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxMethod)

   flatten_reference(newThis->scope, envelope);
   flatten_reference(newThis->code, envelope);
   flatten_reference(newThis->executableName, envelope);
   flatten_reference(newThis->objectVariables, envelope);

  cleanUpFlatten
}


void RexxMethod::run(
    RexxActivity *activity,            /* activity running under            */
    RexxObject *receiver,              /* object receiving the message      */
    RexxString *msgname,               /* message to be run                 */
    RexxObject **argPtr,               /* arguments to the method           */
    size_t count,                      /* count of arguments                */
    ProtectedObject &result)           // the returned result
/******************************************************************************/
/* Function:  Run a method on an object                                       */
/******************************************************************************/
{
    ProtectedObject p(this);           // belt-and-braces to make sure this is protected
    // save this as the most recently executed method
    ActivityManager::currentActivity->setLastMethod(msgname, this);
    // just forward this to the code object
    code->run(activity, this, receiver, msgname, argPtr, count, result);
}


RexxMethod *RexxMethod::newScope(
    RexxClass  *_scope)                 /* new method scope                  */
/******************************************************************************/
/* Function:  Create a new method with a given scope                          */
/******************************************************************************/
{
    // if this doesn't have a scope yet, we can just override what's here
    if (this->scope == OREF_NULL)
    {
        OrefSet(this, this->scope, _scope); /* just set it directly              */
        return this;                       /* and pass back unchanged           */
    }
    else
    {
        /* copy the method                   */
        RexxMethod *newMethod= (RexxMethod *)this->copy();
        /* give the method the new scope     */
        OrefSet(newMethod, newMethod->scope, _scope);
        return newMethod;                  /* and return it                     */
    }
}


RexxObject *RexxMethod::setSecurityManager(
    RexxObject *manager)               /* supplied security manager         */
/******************************************************************************/
/* Function:  Associate a security manager with a method's source             */
/******************************************************************************/
{
    return code->setSecurityManager(manager);
}

void RexxMethod::setScope(
    RexxClass  *_scope)                /* scope for the method              */
/******************************************************************************/
/* Function:  Set a scope for a method without making a copy of the method    */
/*            object.                                                         */
/******************************************************************************/
{
  OrefSet(this, this->scope, _scope);  /* just set the scope                */
}

RexxObject *RexxMethod::setUnguardedRexx()
/******************************************************************************/
/* Function:  Flag a method as being an unguarded method                      */
/******************************************************************************/
{
  this->setUnguarded();                /* turn on the UNGUARDED state     */
  return OREF_NULL;                    /* return nothing                    */
}

RexxObject *RexxMethod::setGuardedRexx( )
/******************************************************************************/
/* Function:  Flag a method as being a guarded method (the default)           */
/******************************************************************************/
{
  this->setGuarded();                  /* flip back to the GUARDED state    */
  return OREF_NULL;                    /* return nothing                    */
}

RexxObject *RexxMethod::setPrivateRexx()
/******************************************************************************/
/* Function:  Flag a method as being a private method                         */
/******************************************************************************/
{
  this->setPrivate();                  /* turn on the PRIVATE flag          */
  return OREF_NULL;                    /* always return nothing             */
}

RexxObject *RexxMethod::setProtectedRexx()
/******************************************************************************/
/* Function:  Flag a method as being a private method                         */
/******************************************************************************/
{
  this->setProtected();                /* turn on the PROTECTED flag        */
  return OREF_NULL;                    /* always return nothing             */
}

/**
 * Return the Guarded setting for a method object.
 *
 * @return .true if the method is guarded.  .false otherwise.
 */
RexxObject *RexxMethod::isGuardedRexx( )
{
    return isGuarded() ? TheTrueObject : TheFalseObject;
}

/**
 * Return the Private setting for a method object.
 *
 * @return .true if the method is private.  .false otherwise.
 */
RexxObject *RexxMethod::isPrivateRexx( )
{
    return isPrivate() ? TheTrueObject : TheFalseObject;
}

/**
 * Return the Protected setting for a method object.
 *
 * @return .true if the method is protected.  .false otherwise.
 */
RexxObject *RexxMethod::isProtectedRexx( )
{
    return isProtected() ? TheTrueObject : TheFalseObject;
}


/**
 * Set the entire set of method attributes with one call.  Used
 * during source compilation.
 *
 * @param _private   The private setting.
 * @param _protected The protected setting.
 * @param _guarded   The guarded setting.
 */
void RexxMethod::setAttributes(bool _private, bool _protected, bool _guarded)
{
    if (_private)
    {
        setPrivate();
    }
    if (_protected)
    {
        setProtected();
    }
    // guarded is the default, so we need to reverse this
    if (!_guarded)
    {
        setUnguarded();
    }
}


RexxSmartBuffer *RexxMethod::saveMethod()
/******************************************************************************/
/* Function: Flatten translated method into a buffer for storage into EA's etc*/
/******************************************************************************/
{
  RexxEnvelope *envelope;              /* envelope for flattening           */
                                       /* Get new envelope object           */
  envelope = new RexxEnvelope;
  ProtectedObject p(envelope);
                                       /* now pack up the envelope for      */
                                       /* saving.                           */
  envelope->pack(this);
  return envelope->getBuffer();        /* return the buffer                 */
}

void *RexxMethod::operator new (size_t size)
/******************************************************************************/
/* Function:  create a new method instance                                    */
/******************************************************************************/
{
                                         /* get a new method object           */
    RexxObject *newObj = new_object(size, T_Method);
    return newObj;                       /* Initialize this new method        */
}


/**
 * Static method used for constructing new method objects in
 * various contexts (such as the define method on the Class class).
 *
 * @param pgmname  The name of the method we're creating.
 * @param source   The method source (either a string or an array).
 * @param position The position used for reporting errors.  This is the position
 *                 of the source argument for the calling method context.
 * @param parentScope
 *                 The parent code we inherit routine scope from.  This overrides
 *                 anything that might be defined in single method code.
 *
 * @return The constructed method object.
 */
RexxMethod *RexxMethod::newMethodObject(RexxString *pgmname, RexxObject *source, RexxObject *position, RexxSource *parentSource)
{
    // request this as an array.  If not convertable, then we'll use it as a string
    RexxArray *newSourceArray = source->requestArray();
    /* couldn't convert?                 */
    if (newSourceArray == (RexxArray *)TheNilObject)
    {
        /* get the string representation     */
        RexxString *sourceString = source->makeString();
        /* got back .nil?                    */
        if (sourceString == (RexxString *)TheNilObject)
        {
            /* raise an error                    */
            reportException(Error_Incorrect_method_no_method, position);
        }
        /* wrap an array around the value    */
        newSourceArray = new_array(sourceString);
    }
    else                                 /* have an array, make sure all      */
    {
        /* is it single dimensional?         */
        if (newSourceArray->getDimension() != 1)
        {
            /* raise an error                    */
            reportException(Error_Incorrect_method_noarray, position);
        }
        /*  element are strings.             */
        /* Make a source array safe.         */
        ProtectedObject p(newSourceArray);
        /* Make sure all elements in array   */
        for (size_t counter = 1; counter <= newSourceArray->size(); counter++)
        {
            /* Get element as string object      */
            RexxString *sourceString = newSourceArray ->get(counter)->makeString();
            /* Did it convert?                   */
            if (sourceString == (RexxString *)TheNilObject)
            {
                /* and report the error.             */
                reportException(Error_Incorrect_method_nostring_inarray, IntegerTwo);
            }
            else
            {
                /* itsa string add to source array   */
                newSourceArray ->put(sourceString, counter);
            }
        }
    }

    RexxMethod *result = new RexxMethod(pgmname, newSourceArray);

    // if we've been provided with a scope, use it
    if (parentSource == OREF_NULL)
    {
        // see if we have an active context and use the current source as the basis for the lookup
        RexxActivation *currentContext = ActivityManager::currentActivity->getCurrentRexxFrame();
        if (currentContext != OREF_NULL)
        {
            parentSource = currentContext->getSourceObject();
        }
    }

    // if there is a parent source, then merge in the scope information
    if (parentSource != OREF_NULL)
    {
        result->getSourceObject()->inheritSourceContext(parentSource);
    }

    return result;
}


RexxMethod *RexxMethod::newRexx(
    RexxObject **init_args,            /* subclass init arguments           */
    size_t       argCount)             /* number of arguments passed        */
/******************************************************************************/
/* Function:  Create a new method from REXX code contained in a string or an  */
/*            array                                                           */
/******************************************************************************/
{
    RexxObject *pgmname;                 /* method name                       */
    RexxObject *_source;                 /* Array or string object            */
    RexxMethod *newMethod;               /* newly created method object       */
    RexxObject *option = OREF_NULL;
    size_t initCount = 0;                /* count of arguments we pass along  */

                                         /* break up the arguments            */

    RexxClass::processNewArgs(init_args, argCount, &init_args, &initCount, 2, (RexxObject **)&pgmname, (RexxObject **)&_source);
    /* get the method name as a string   */
    RexxString *nameString = stringArgument(pgmname, ARG_ONE);
    requiredArgument(_source, ARG_TWO);          /* make sure we have the second too  */

    RexxSource *sourceContext = OREF_NULL;
    // retrieve extra parameter if exists
    if (initCount != 0)
    {
        RexxClass::processNewArgs(init_args, initCount, &init_args, &initCount, 1, (RexxObject **)&option, NULL);
        if (isOfClass(Method, option))
        {
            sourceContext = ((RexxMethod *)option)->getSourceObject();
        }
        else if (isOfClass(Routine, option))
        {
            sourceContext = ((RoutineClass *)option)->getSourceObject();
        }
        else if (isOfClass(Package, option))
        {
            sourceContext = ((PackageClass *)option)->getSourceObject();
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
        }
    }
    /* go create a method                */
    newMethod = RexxMethod::newMethodObject(nameString, _source, IntegerTwo, sourceContext);
    ProtectedObject p(newMethod);
    /* Give new object its behaviour     */
    newMethod->setBehaviour(((RexxClass *)this)->getInstanceBehaviour());
    if (((RexxClass *)this)->hasUninitDefined())          /* does object have an UNINT method  */
    {
        newMethod->hasUninit();              /* Make sure everyone is notified.   */
    }
    /* now send an INIT message          */
    newMethod->sendMessage(OREF_INIT, init_args, initCount);
    return newMethod;                    /* return the new method             */
}


RexxMethod *RexxMethod::newFileRexx(RexxString *filename)
/******************************************************************************/
/* Function:  Create a method from a fully resolved file name                 */
/******************************************************************************/
{
    /* get the method name as a string   */
    filename = stringArgument(filename, ARG_ONE);
    /* create a source object            */
    RexxMethod *newMethod = new RexxMethod(filename);
    ProtectedObject p(newMethod);
    newMethod->setScope((RexxClass *)TheNilObject);
    /* Give new object its behaviour     */
    newMethod->setBehaviour(((RexxClass *)this)->getInstanceBehaviour());
    if (((RexxClass *)this)->hasUninitDefined())    /* does object have an UNINT method  */
    {
        newMethod->hasUninit();           /* Make sure everyone is notified.   */
    }
    /* now send an INIT message          */
    newMethod->sendMessage(OREF_INIT);
    return newMethod;
}


RexxMethod *RexxMethod::restore(
    RexxBuffer *buffer,                /* buffer containing the method      */
    char *startPointer,                /* first character of the method     */
    size_t length)                     // length of the data to restore
/******************************************************************************/
/* Function: Unflatten a translated method.  Passed a buffer object containing*/
/*           the method                                                       */
/******************************************************************************/
{
                                       /* Get new envelope object           */
  RexxEnvelope *envelope  = new RexxEnvelope;
  ProtectedObject p(envelope);
                                       /* now puff up the method object     */
  envelope->puff(buffer, startPointer, length);
                                       /* The receiver object is an envelope*/
                                       /* whose receiver is the actual      */
                                       /* method object we're restoring     */
  return (RexxMethod *)envelope->getReceiver();
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
void BaseCode::run(RexxActivity *activity, RexxMethod *method, RexxObject *receiver, RexxString *msgname, RexxObject **arguments, size_t argCount, ProtectedObject &result)
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
void BaseCode::call(RexxActivity *activity, RoutineClass *routine, RexxString *msgname, RexxObject **arguments, size_t argcount, RexxString *ct, RexxString *env, int context, ProtectedObject &result)
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
void BaseCode::call(RexxActivity *activity, RoutineClass *routine, RexxString *msgname, RexxObject **arguments, size_t argcount, ProtectedObject &result)
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
RexxArray *BaseCode::getSource()
{
                                       /* this is always a null array       */
    return (RexxArray *)TheNullArray->copy();
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
/******************************************************************************/
/* Function:  Associate a security manager with a method's source             */
/******************************************************************************/
{
    // the default is just to return a failure
    return TheFalseObject;
}


/**
 * Retrieve the source object associated with a code object.
 *
 * @return
 */
RexxSource *BaseCode::getSourceObject()
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
BaseCode *BaseCode::setSourceObject(RexxSource *s)
{
    return this;         // this is just a nop
}


/**
 * Retrieve the package associated with a code object.  Returns
 * OREF_NULL if this code object doesn't have a source.
 *
 * @return The associated package, or OREF_NULL.
 */
PackageClass *BaseCode::getPackage()
{
    RexxSource *source = getSourceObject();
    if (source != OREF_NULL)
    {
        return source->getPackage();
    }

    return OREF_NULL;
}


/**
 * Create a method from an external library source.
 *
 * @param name   The method name.
 *
 * @return The resolved method object, or OREF_NULL if unable to
 *         load the routine.
 */
RexxMethod *RexxMethod::loadExternalMethod(RexxString *name, RexxString *descriptor)
{
    name = stringArgument(name, "name");
    descriptor = stringArgument(descriptor, "descriptor");
    /* convert external into words       */
    RexxArray *_words = StringUtil::words(descriptor->getStringData(), descriptor->getLength());
    ProtectedObject p(_words);
    // "LIBRARY libbar [foo]"
    if (((RexxString *)(_words->get(1)))->strCompare(CHAR_LIBRARY))
    {
        RexxString *library = OREF_NULL;
        // the default entry point name is the internal name
        RexxString *entry = name;

        // full library with entry name version?
        if (_words->size() == 3)
        {
            library = (RexxString *)_words->get(2);
            entry = (RexxString *)_words->get(3);
        }
        else if (_words->size() == 2)
        {
            library = (RexxString *)_words->get(2);
        }
        else  // wrong number of tokens
        {
            /* this is an error                  */
            reportException(Error_Translation_bad_external, descriptor);
        }
                                     /* create a new native method        */
        RexxNativeCode *nmethod = PackageManager::loadMethod(library, entry);
        // raise an exception if this entry point is not found.
        if (nmethod == OREF_NULL)
        {
            return (RexxMethod *)TheNilObject;
        }
        /* turn into a real method object    */
        return new RexxMethod(name, nmethod);
    }
    else
    {
        /* unknown external type             */
        reportException(Error_Translation_bad_external, descriptor);
    }
    return OREF_NULL;
}

