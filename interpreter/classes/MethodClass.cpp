/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2020 Rexx Language Association. All rights reserved.    */
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
/* Primitive Method Class                                                     */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "RexxCode.hpp"
#include "NativeCode.hpp"
#include "Activity.hpp"
#include "RexxActivation.hpp"
#include "NativeActivation.hpp"
#include "MethodClass.hpp"
#include "DirectoryClass.hpp"
#include "ProtectedObject.hpp"
#include "BufferClass.hpp"
#include "ProgramMetaData.hpp"
#include "RexxInternalApis.h"
#include "RoutineClass.hpp"
#include "PackageClass.hpp"
#include "Interpreter.hpp"
#include "RexxCode.hpp"
#include "PackageManager.hpp"
#include "LanguageParser.hpp"
#include "MethodArguments.hpp"

// singleton class instance
RexxClass *MethodClass::classInstance = OREF_NULL;


/**
 * Create initial class object at bootstrap time.
 */
void MethodClass::createInstance()
{
    CLASS_CREATE(Method);
}


/**
 * Allocate a new Method object.
 *
 * @param size   The base size of the method object.
 *
 * @return The storage for a method object.
 */
void *MethodClass::operator new (size_t size)
{
    return new_object(size, T_Method);
}


/**
 * Initialize a Routine object from a generated code object. Generally
 * used for routines generated from ::METHOD directives.
 *
 * @param name    The routine name.
 * @param codeObj The associated code object.
 */
MethodClass::MethodClass(RexxString *name, BaseCode *codeObj)
{
    executableName = name;
    code = codeObj;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void MethodClass::live(size_t liveMark)
{
    memory_mark(scope);
    memory_mark(code);
    memory_mark(executableName);
    memory_mark(objectVariables);
    memory_mark(annotations);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void MethodClass::liveGeneral(MarkReason reason)
{
    memory_mark_general(scope);
    memory_mark_general(code);
    memory_mark_general(executableName);
    memory_mark_general(objectVariables);
    memory_mark_general(annotations);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void MethodClass::flatten(Envelope *envelope)
{
    setUpFlatten(MethodClass)

     flattenRef(scope);
     flattenRef(code);
     flattenRef(executableName);
     flattenRef(objectVariables);
     flattenRef(annotations);

    cleanUpFlatten
}


/**
 * Invoke this method on an object.
 *
 * @param activity The current activity.
 * @param receiver The receiver object.
 * @param msgname  The message name.
 * @param argPtr   The argument pointer.
 * @param count    The argument count.
 * @param result   The result object used to pass a result back.
 */
void MethodClass::run(Activity *activity, RexxObject *receiver, RexxString *msgname,
    RexxObject **argPtr, size_t count, ProtectedObject &result)
{
    // just forward this to the code object
    code->run(activity, this, receiver, msgname, argPtr, count, result);
}


/**
 * Change the scope of a method object.  This also returns
 * a new method object.
 *
 * @param _scope The new scope.
 *
 * @return The new method object with the new scope.
 */
MethodClass *MethodClass::newScope(RexxClass *newClass)
{
    // if this doesn't have a scope yet, we can just override what's here
    // and return the same method instance.
    if (scope == OREF_NULL)
    {
        setField(scope, newClass);
        return this;
    }
    else
    {
        // we need to return a copy of the method with the scope set
        MethodClass *newMethod= (MethodClass *)copy();
        setOtherField(newMethod, scope, newClass);
        return newMethod;
    }
}


/**
 * Associate a security manager with a method's source.
 *
 * @param manager The security manager instance.
 *
 * @return The return code from setting this on the code object.
 */
RexxObject *MethodClass::setSecurityManager(RexxObject *manager)
{
    return code->setSecurityManager(manager);
}

/**
 * Internal method to just set the scope for the method.
 *
 * @param _scope The new scope.
 */
void MethodClass::setScope(RexxClass  *_scope)
{
    setField(scope, _scope);
}


/**
 * Change the guarded state of the method from Rexx code.
 *
 * @return No return value.
 */
RexxObject *MethodClass::setUnguardedRexx()
{
    setUnguarded();
    return OREF_NULL;
}


/**
 * Change the guarded state of the method from Rexx code.
 *
 * @return No return value.
 */
RexxObject *MethodClass::setGuardedRexx( )
{
    setGuarded();
    return OREF_NULL;
}


/**
 * Flag the method as being private from Rexx code.
 *
 * @return No return value.
 */
RexxObject *MethodClass::setPrivateRexx()
{
    setPrivate();
    return OREF_NULL;
}


/**
 * Turn on a method's protected state from Rexx code.
 *
 * @return No return value.
 */
RexxObject *MethodClass::setProtectedRexx()
{
    setProtected();
    return OREF_NULL;
}


/**
 * Return the Guarded setting for a method object.
 *
 * @return .true if the method is guarded.  .false otherwise.
 */
RexxObject *MethodClass::isGuardedRexx( )
{
    return booleanObject(isGuarded());
}


/**
 * Return the Private setting for a method object.
 *
 * @return .true if the method is private.  .false otherwise.
 */
RexxObject *MethodClass::isPrivateRexx( )
{
    return booleanObject(isPrivate());
}


/**
 * Return the Package setting for a method object.
 *
 * @return .true if the method is package scope.  .false
 *         otherwise.
 */
RexxObject *MethodClass::isPackageRexx( )
{
    return booleanObject(isPackageScope());
}


/**
 * Return the Protected setting for a method object.
 *
 * @return .true if the method is protected.  .false otherwise.
 */
RexxObject *MethodClass::isProtectedRexx( )
{
    return booleanObject(isProtected());
}


/**
 * Indicate if this is an abstract method
 *
 * @return .true if the method is abstract.  .false otherwise.
 */
RexxObject *MethodClass::isAbstractRexx( )
{
    return booleanObject(isAbstract());
}


/**
 * Indicate if this method was defined as an attribute method
 * (using ::attribute or ::method attribute)
 *
 * @return .true if the method is defined as an attribute.
 *         .false otherwise.
 */
RexxObject *MethodClass::isAttributeRexx( )
{
    return booleanObject(isAttribute());
}


/**
 * Indicate if this method was defined as a constant method
 * (using ::constant)
 *
 * @return .true if the method is defined as an attribute.
 *         .false otherwise.
 */
RexxObject *MethodClass::isConstantRexx( )
{
    return booleanObject(isConstant());
}


/**
 * Returns the defining class scope for a method.  Returns .nil
 * for any method not defined by a class scope.
 *
 * @return The scope class or .nil.
 */
RexxObject *MethodClass::getScopeRexx()
{
    return resultOrNil(getScope());
}


/**
 * Retrieve a string name for the method scope. If the scope is .nil,
 * then ".NIL" is returned, otherwise the class name is used.
 *
 * @return A string name for the scope.
 */
RexxString *MethodClass::getScopeName()
{
    return (RexxObject *)scope == TheNilObject ? GlobalNames::DOTNIL : scope->getId();
}


/**
 * Set the entire set of method attributes with one call.  Used
 * during source compilation.
 *
 * @param _access   The access setting.
 * @param _protected The protected setting.
 * @param _guarded   The guarded setting.
 */
void MethodClass::setAttributes(AccessFlag _access, ProtectedFlag _protected, GuardFlag _guarded)
{
    switch (_access)
    {
        case PRIVATE_SCOPE:
            setPrivate();
            break;

        case PACKAGE_SCOPE:
            setPackageScope();
            break;

        // covers PUBLIC and DEFAULT
        default:
            break;
    }

    if (_protected == PROTECTED_METHOD)
    {
        setProtected();
    }

    // both GUARDED and DEFAULT are guarded, so check for the reverse.
    if (_guarded == UNGUARDED_METHOD)
    {
        setUnguarded();
    }
}



/**
 * Restore a program from a simple buffer.
 *
 * @param fileName The file name of the program we're restoring from.
 * @param buffer   The source buffer.  This contains all of the saved metadata
 *                 ahead of the the flattened object.
 *
 * @return The inflated Method object, if valid.
 */
MethodClass* MethodClass::restore(RexxString *fileName, BufferClass *buffer)
{
    // try to restore the routine object from the compiled file, ProgramMetaData handles all of the details here
    Protected<RoutineClass> routine = ProgramMetaData::restore(fileName, buffer);
    if (routine != (RoutineClass *)OREF_NULL)
    {
        // create and return a new method, use the restored compiled code for it
        return new MethodClass(fileName, routine->getCode());
    }
    return OREF_NULL;
}



/**
 * Static method used for constructing new method objects in
 * various contexts (such as the define method on the Class class).
 *
 * @param pgmname  The name of the method we're creating.
 * @param source   The method source (either a string or an array).
 * @param scope    The scope that the new method object will be given.
 * @param position The position used for reporting errors.  This is the position
 *                 of the source argument for the calling method context.
 *
 * @return The constructed method object.
 */
MethodClass *MethodClass::newMethodObject(RexxString *pgmname, RexxObject *source, RexxClass *scope, const char *position)
{
    // this is used in contexts where an existing method object is allowed...perform this
    // check here and just return the original object if it is already a method.
    if (isMethod(source))
    {
        return ((MethodClass *)source)->newScope(scope);
    }

    // validate, and potentially transform, the method source object.
    ArrayClass *newSourceArray = processExecutableSource(source, position);

    // if not a valid source, give an error
    if (newSourceArray == OREF_NULL)
    {
        reportException(Error_Incorrect_method_no_method_type, position);
    }

    // this method is called when methods are added to class, object, directory, etc.
    // we want to inherit from the current execution source context if we can.

    PackageClass *sourceContext = OREF_NULL;

    // see if we have an active context and use the current source as the basis for the lookup
    RexxActivation *currentContext = ActivityManager::currentActivity->getCurrentRexxFrame();
    if (currentContext != OREF_NULL)
    {
        sourceContext = currentContext->getPackage();
    }

    // create a method and give it the target scope
    MethodClass *method = LanguageParser::createMethod(pgmname, newSourceArray, sourceContext);
    method->setScope(scope);
    return method;
}


/**
 * Create a new method from REXX code contained in a string or an
 * array
 *
 * @param init_args The array of arguments to the new method.
 * @param argCount  The argument count.
 *
 * @return A new method object.
 */
MethodClass *MethodClass::newRexx(RexxObject **init_args, size_t argCount)
{
    // this method is defined as an instance method, but this is actually attached
    // to a class object instance.  Therefore, any use of the this pointer
    // will be touching the wrong data.  Use the classThis pointer for calling
    // any methods on this object from this method.
    RexxClass *classThis = (RexxClass *)this;

    RexxString *programName;
    Protected<ArrayClass> sourceArray;
    PackageClass *sourceContext;

    // parse all of the options
    processNewExecutableArgs(init_args, argCount, programName, sourceArray, sourceContext);

    // go create a method from whatever we were given for source.
    ProtectedObject newMethod = LanguageParser::createMethod(programName, sourceArray, sourceContext);

    // finish up the object creation.  Set the correct instance behavior (this could
    // be a subclass), check for uninit methods, and finally, send an init message using any
    // left over arguments.
    classThis->completeNewObject(newMethod, init_args, argCount);
    return newMethod;
}


/**
 * Create a method object from a file.
 *
 * @param filename The target file name.
 * @param scope    The scope that the new method object will be given.
 *
 * @return The created method object.
 */
MethodClass *MethodClass::newFileRexx(RexxString *filename, PackageClass *sourceContext)
{
    // this class is defined on the object class, but this is actually attached
    // to a class object instance.  Therefore, any use of the this pointer
    // will be touching the wrong data.  Use the classThis pointer for calling
    // any methods on this object from this method.
    RexxClass *classThis = (RexxClass *)this;

    // parse all of the options
    processNewFileExecutableArgs(filename, sourceContext);

    // go create a method from filename
    Protected<MethodClass> newMethod = LanguageParser::createMethod(filename, sourceContext);
    classThis->completeNewObject(newMethod);
    return newMethod;
}


/**
 * Create a method from an external library source.
 *
 * @param methodName The method name.
 * @param libraryDescriptor
 *
 * @return The resolved method object, or OREF_NULL if unable to
 *         load the routine.
 */
MethodClass *MethodClass::loadExternalMethod(RexxString *methodName, RexxString *libraryDescriptor)
{
    Protected<RexxString> name = stringArgument(methodName, "name");
    Protected<RexxString> descriptor = stringArgument(libraryDescriptor, "descriptor");
    // convert external into words
    Protected<ArrayClass> _words = StringUtil::words(descriptor->getStringData(), descriptor->getLength());
    // "LIBRARY libbar [foo]"
    if (_words->size() > 0 && ((RexxString *)(_words->get(1)))->strCaselessCompare("LIBRARY"))
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
            reportException(Error_Translation_bad_external, descriptor);
        }
        // get the native code for this library
        NativeCode *nmethod = PackageManager::loadMethod(library, entry);
        // raise an exception if this entry point is not found.
        if (nmethod == OREF_NULL)
        {
            return (MethodClass *)TheNilObject;
        }
        // turn into a real method object
        return new MethodClass(name, nmethod);
    }
    else
    {
        // unknown external type
        reportException(Error_Translation_bad_external, descriptor);
    }
    return OREF_NULL;
}

