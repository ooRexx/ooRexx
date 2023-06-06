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
/* Object REXX Kernel                                                         */
/*                                                                            */
/* The main REXX object definitions                                           */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "ObjectClass.hpp"
#include "StringClass.hpp"
#include "BufferClass.hpp"
#include "DirectoryClass.hpp"
#include "VariableDictionary.hpp"
#include "ArrayClass.hpp"
#include "Activity.hpp"
#include "RexxActivation.hpp"
#include "MessageClass.hpp"
#include "MethodClass.hpp"
#include "ExpressionBaseVariable.hpp"
#include "ProtectedObject.hpp"
#include "PointerClass.hpp"
#include "MethodArguments.hpp"
#include "MethodDictionary.hpp"
#include "PointerTable.hpp"


// singleton class instance
RexxClass *RexxObject::classInstance = OREF_NULL;
RexxObject *RexxNilObject::nilObject = OREF_NULL;


/**
 * Create initial class object at bootstrap time.
 */
void RexxObject::createInstance()
{
    CLASS_CREATE(Object);
}


/**
 * Allocate a new object.
 *
 * @param size   The base size of the method object.
 *
 * @return The storage for a method object.
 */
void *RexxObject::operator new (size_t size)
{
    return new_object(size, T_Object);
}


/**
 * Default object live stack marking.  Since Object is
 * an exported object, it needs to mark the object variables.
 * Other exported classes that inherit from Object are
 * also expected to mark this field.
 *
 * @param liveMark The current live mark.
 */
void RexxObject::live(size_t liveMark)
{
    memory_mark(objectVariables);
}


/**
 * Default object general live stack marking.  Since Object is
 * an exported object, it needs to mark the object variables.
 * Other exported classes that inherit from Object are
 * also expected to mark this field.
 *
 * @param reason The reason for the mark being performed.
 */
void RexxObject::liveGeneral(MarkReason reason)
{
    memory_mark_general(objectVariables);
}


/**
 * Default object flatten processing..  Since Object is an
 * exported object, it needs to mark the object variables. Other
 * exported classes that inherit from Object are also expected
 * to mark this field.
 *
 * @param envelope the target envelope we're flatting in.
 */
void RexxObject::flatten(Envelope *envelope)
{
    setUpFlatten(RexxObject)

    flattenRef(objectVariables);

    cleanUpFlatten
}


/**
 * Create a proxy object for a "special" REXX object.
 *
 * @param envelope The target envelope.
 *
 * @return The proxy object, which is usually a name.
 */
RexxObject *RexxInternalObject::makeProxy(Envelope *envelope)
{
    // we are generally only called if the class is marked as a proxy class.
    // we recognize .nil, but don't handle any other special objects here.
    if (this == TheNilObject)
    {
        return (RexxObject *)new_proxy("NIL");
    }
    else
    {
        return (RexxObject *)this;
    }
}


/**
 * Set the object virtual function table and behaviour
 * to a particular type designation.
 *
 * @param type   The internal type number.
 */
void RexxInternalObject::setObjectType(size_t type)
{
    setVirtualFunctions(memoryObject.virtualFunctionTable[type]);
    setBehaviour(RexxBehaviour::getPrimitiveBehaviour(type));
}


/**
 * The rexx object override for the isequal method.  This will
 * either compare using the pointers or for non-primitive objects,
 * will call the == method to compare.
 *
 * @param other  The object to compare against.
 *
 * @return true if the objects compare equal, false otherwise.
 */
bool RexxInternalObject::isEqual(RexxInternalObject *other)
{
    // if this is not a subclass, we can just directly compare the pointers.
    if (isBaseClass())
    {
        return this == other;
    }
    // we need to invoke the "==" method and process the true value
    // of the result
    else
    {
        ProtectedObject result;
        ((RexxObject *)this)->sendMessage(GlobalNames::STRICT_EQUAL, (RexxObject *)other, result);
        return result->truthValue(Error_Logical_value_method);
    }
}


/**
 * Test if an object instance is an enhanced version of a
 * primitive class or a subclass of the primitive class.
 *
 * @return true if the object is a subclass instance or an enhanced one-off.
 */
bool RexxInternalObject::isSubClassOrEnhanced()
{
    return behaviour->isNonPrimitive();
}


/**
 * Test if an object instance is a true instance of a primitive
 * class.
 *
 * @return true if the object is not a subclass instance or an enhanced one-off.
 */
bool RexxInternalObject::isBaseClass()
{
    return behaviour->isPrimitive();
}


/**
 * Wrapper around the compareTo() method that validates and
 * extracts integer value.
 *
 * @param other  The other comparison object
 *
 * @return -1, 0, 1 depending on the comparison result.
 */
wholenumber_t RexxInternalObject::compareTo(RexxInternalObject *other )
{
    ProtectedObject result;

    ((RexxObject *)this)->sendMessage(GlobalNames::COMPARETO, (RexxObject *)other, result);
    // the result is required
    if (result.isNull())
    {
        reportException(Error_No_result_object_message, GlobalNames::COMPARETO);
    }
    wholenumber_t comparison;

    // the comparison value is a signed number, it has to convert
    if (!result->numberValue(comparison))
    {
        reportException(Error_Invalid_whole_number_compareto, (RexxObject *)result);
    }
    return comparison;
}


/**
 * Test if an internal object is an instance of another class.
 *
 * @param other  The test class.
 *
 * @return Always returns false.
 */
bool RexxInternalObject::isInstanceOf(RexxClass *other)
{
    // internal classes always fail this
    return false;
}


/**
 * Test if a Rexx object is an instance of a given class.
 *
 * @param other  The other test class.
 *
 * @return True if this object is an instance of the target class, false otherwise.
 */
bool RexxObject::isInstanceOf(RexxClass *other)
{
    return classObject()->isCompatibleWith(other);
}


/**
 * The Rexx external version of the instance of.
 *
 * @param other  The other test class.
 *
 * @return .true if this class is an instance of the target class. .false
 *         otherwise.
 */
RexxObject *RexxObject::isInstanceOfRexx(RexxClass *other)
{
    // verify we have a valid class object to check
    classArgument(other, TheClassClass, "class");
    return booleanObject(isInstanceOf(other));
}


/**
 * Retrieve the method instance for an object's defined method.
 *
 * @param method_name
 *               The method name.
 *
 * @return The method object that implements the object method.
 */
MethodClass *RexxInternalObject::instanceMethod(RexxString  *method_name)
{
    // Internal objects have no instance methods, so this will always fail.
    return (MethodClass *)TheNilObject;
}


/**
 * Retrieve the method instance for an object's defined method.
 *
 * @param name The method name.
 *
 * @return The method object that implements the object method.
 */
MethodClass *RexxObject::instanceMethod(RexxString *name)
{
    // the name must be a string...and we use it in upper case
    Protected<RexxString> method_name = stringArgument(name, ARG_ONE)->upper();
    // retrieve the method from the dictionary
    MethodClass *method_object = behaviour->methodLookup(method_name);
    return (MethodClass *)resultOrNil(method_object);
}


/**
 * Return a supplier containing the methods implemented by an
 * object.  Depending on the argument, this is either A) all of
 * the methods, B) just the explicitly set instance methods, or
 * C) the methods provided by a given class.
 *
 * @param class_object
 *               The target class object (optional).
 *
 * @return A supplier with the appropriate method set.
 */
SupplierClass *RexxInternalObject::instanceMethods(RexxClass *class_object)
{
    // internal objects have none, so we always return NULL.
    return OREF_NULL;
}


/**
 * Return a supplier containing the methods implemented by an
 * object.  Depending on the argument, this is either A) all of
 * the methods, B) just the explicitly set instance methods, or
 * C) the methods provided by a given class.
 *
 * @param class_object
 *               The target class object (optional).
 *
 * @return A supplier with the appropriate method set.
 */
SupplierClass *RexxObject::instanceMethods(RexxClass *class_object)
{
    // the behaviour handles all of this
    return behaviour->getMethods(class_object);
}


/**
 * Retrieve the method instance for an object's defined method.
 *
 * @param method_name
 *               The method name.
 *
 * @return The method object that implements the object method.
 */
MethodClass *RexxObject::instanceMethodRexx(RexxString *method_name)
{
    return instanceMethod(method_name);
}


/**
 * Return a supplier containing the methods implemented by an
 * object.  Depending on the argument, this is either A) all of
 * the methods, B) just the explicitly set instance methods, or
 * C) the methods provided by a given class.
 *
 * @param class_object
 *               The target class object (optional).
 *
 * @return A supplier with the appropriate method set.
 */
SupplierClass *RexxObject::instanceMethodsRexx(RexxClass *class_object)
{
    return instanceMethods(class_object);
}


/**
 * Default implementation of the HASHCODE method.
 *
 * @return The object's hash code value.
 */
RexxObject *RexxObject::hashCode()
{
    // get the hash value directly, then turn it into a binary string value
    HashCode h = getHashValue();
    return new_string((char *)&h, sizeof(HashCode));
}


/**
 * Hash an exported object.  If we're a non-primitive one, this
 * will require us to send the HASHCODE message to request a
 * hash value.
 *
 * @return A "hashed hash" that can be used by the map collections.
 */
HashCode RexxObject::hash()
{
    // if this is a primitive object, we can just return the primitive hash code.
    if (isBaseClass())
    {
        return getHashValue();
    }
    else
    {
        ProtectedObject result;
        // we have some other type of object, so we need to request a hash code
        // by sending the HASHCODE() message.
        sendMessage(GlobalNames::HASHCODE, result);

        // we need to have a return value for this.
        if (result.isNull())
        {
            reportException(Error_No_result_object_message, GlobalNames::HASHCODE);
        }

        // the default version sends us a string containing binary data.
        // if the string is long enough for that, we reverse the process.  Otherwise,
        // we'll just take the hash code from the string object.
        return result->stringValue()->getObjectHashCode();
    }
}


/**
 * Process the default "==" strict comparison operator
 *
 * @param other  The other object for comparison.
 *
 * @return .true or .false to indicate the comparison result.
 */
RexxObject *RexxObject::strictEqual(RexxObject * other)
{
    requiredArgument(other, ARG_ONE);
    // this is direct object equality
    return booleanObject(this == other);
}


/**
 * Normal "=" type comparison.  This only returns true if the
 * two objects are the same object
 *
 * @param other  The other object for comparison.
 *
 * @return .true or .false reflecting the comparison result.
 */
RexxObject * RexxObject::equal(RexxObject * other)
{
  requiredArgument(other, ARG_ONE);

  return booleanObject(this == other);
}

/**
 * Return the strict inequality of two objects
 *
 * @param other  The other object for comparison.
 *
 * @return .true or .false reflecting the comparison result.
 */
RexxObject *RexxObject::strictNotEqual(RexxObject *other)
{
   requiredArgument(other, ARG_ONE);
   return booleanObject(this != other);
}


/**
 * Return the inequality of two objects
 *
 * @param other  The other object for comparison.
 *
 * @return .true or .false reflecting the comparison result.
 */
RexxObject *RexxObject::notEqual(RexxObject *other)
{
   requiredArgument(other, ARG_ONE);
   return booleanObject(this != other);
}


/**
 * Convert an object to a logical value without raising an
 * error.
 *
 * @param result The converted value.
 *
 * @return true if this converted ok, false for an invalid logical.
 */
bool RexxInternalObject::logicalValue(logical_t &result)
{
    // always take the logical value from the string value.  For internal objects,
    // this will most likely be false.
    return requestString()->logicalValue(result);
}


/**
 * test the truth value of a primitive object.  Raises an
 * error if this does not have a valid truth value.
 *
 * @param errorCode The error code to issue.
 *
 * @return The logical value of the object.
 */
bool RexxInternalObject::truthValue(RexxErrorCodes errorCode)
{
    // process the string value for this and get the truth value for that.  This will
    // be an error for internal objects...which should never happen, really.
    return requestString()->truthValue(errorCode);
}


/**
 * First level primitive copy of an object.  This just copies
 * the object storage, and nothing else.
 *
 * @return A copy of the target internal object.
 */
RexxInternalObject *RexxInternalObject::copy()
{
    // Instead of calling new_object and memcpy, ask the memory object to make
    // a copy of ourself.  This way, any header information can be correctly
    // initialized by memory.
    return clone();
}


/**
 * Copy an object that is visible to the Rexx programmer.
 * This copy ensures that all object variables are copied,
 * as well as ensuring that the new instance has the
 * same behaviour as the initial object.
 *
 * @return A new instance of this object.
 */
RexxInternalObject *RexxObject::copy()
{
    // Instead of calling new_object and memcpy, ask the memory object to make
    // a copy of ourself.  This way, any header information can be correctly
    // initialized by memory.
    RexxObject *newObj = (RexxObject *)clone();
    ProtectedObject p(newObj);

    // do we have object variables?  We need to give that object
    // a copy of the variables
    copyObjectVariables(newObj);

    // have instance methods?
    if (behaviour->hasInstanceMethods())
    {
        // need to copy the behaviour
        newObj->setBehaviour((RexxBehaviour *)newObj->behaviour->copy());
    }

    // if the source object has an uninit method, then so does the copy.  Make
    // sure it is added to the table.
    if (hasUninit())
    {
        newObj->requiresUninit();
    }
    return newObj;
}


/**
 * Copy an object's object variable dictionaries into another obj.
 * The variable dictionaries are copied so that they
 * are different variable trees, but none of the objects
 * stored in the dictionary are copied.
 *
 * @param newObj The target new object.
 */
void RexxObject::copyObjectVariables(RexxObject *newObj)
{
    // copy the complete variable set if we have something
    if (objectVariables != OREF_NULL)
    {
        objectVariables = objectVariables->deepCopy();
    }
}


/**
 * Check a private method for accessibility.
 *
 * @param method The method object to check
 * @param error  An error to be raised if this is not permitted.
 *
 * @return An executable method, or OREF_NULL if this cannot be called.
 */
MethodClass *RexxObject::checkPrivate(MethodClass *method, RexxErrorCodes &error)
{
    // get the calling activation context
    ActivationBase *activation = ActivityManager::currentActivity->getTopStackFrame();
    if (activation != OREF_NULL)
    {
        // if the sending and receiving object are the same, this is allowed.
        RexxObject *sender = activation->getReceiver();
        if (sender == this)
        {
            return method;
        }
        // no sender means this is a routine or program context.  Definitely not allowed.
        if (sender == OREF_NULL)
        {
            error = Error_No_method_private;
            return OREF_NULL;
        }
        // ok, now we check the various scope possibilities
        RexxClass *scope = method->getScope();
        // 1) Another instance of the same class that defined the method?
        if (sender->isInstanceOf(scope) )
        {
            return method;       // ok, we'll allow this
        }
        // if the sender is a class object, check the class for compatibility with the
        // method scope
        if (isOfClassType(Class, sender))
        {
            // if this class is part of the compatible hierarchy, this is also permitted
            if (((RexxClass *)sender)->isCompatibleWith(scope))
            {
                return method;
            }
        }
    }
    // can't touch this...
    error = Error_No_method_private;
    return OREF_NULL;
}


/**
 * Check a package method for accessibility.
 *
 * @param method The method object to check
 * @param error  The error to be raised if this is not permitted.
 *
 * @return An executable method, or OREF_NULL if this cannot be called.
 */
MethodClass *RexxObject::checkPackage(MethodClass *method, RexxErrorCodes &error )
{
    // get the calling activation context
    ActivationBase *activation = ActivityManager::currentActivity->getTopStackFrame();

    // likely a topLevel call via SendMessage() API which is contextless.
    if (activation == OREF_NULL)
    {
        error = Error_No_method_package;
        return OREF_NULL;
    }
    // get the package from that frame.
    PackageClass *callerPackage = activation->getPackage();

    // it is possible this is a special native activation, which means there
    // is no caller context. This is a no-go.
    if (callerPackage == OREF_NULL)
    {
        return OREF_NULL;
    }

    // defined in the same package, this is good.
    if (method->isSamePackage(callerPackage))
    {
        return method;
    }
    // can't touch this...
    error = Error_No_method_package;
    return OREF_NULL;
}


/**
 * Check that methods like RUN, SETMETHOD, and UNSETMETHOD are
 * issued from a method on the same object instance.
 *
 * @return An executable method, or OREF_NULL if this cannot be called.
 */
void RexxObject::checkRestrictedMethod(const char *methodName)
{
    // get the calling activation context
    ActivationBase *activation = ActivityManager::currentActivity->getTopStackFrame();
    if (activation != OREF_NULL)
    {
        // Unlike private methods, this is only allowed from the instance, or from
        // the class.  Because RUN, SETMETHOD, and UNSETMETHOD are defined by Object,
        // this essentially means the only restrictions on these private methods under
        // the new rules is that it must be called from another method.  Seriously
        // NOT the intent of making private methods interact a little more.  Therefore,
        // we add an additional check for any private methods defined by .object
        RexxObject *sender = activation->getReceiver();
        if (sender == this)
        {
            return;
        }

        // no sender means this is a routine or program context.  Definitely not allowed.
        if (sender == OREF_NULL)
        {
            reportException(Error_Execution_private_access, methodName);
        }

        // if the sender is a class object, check the class for compatibility with the
        // method scope
        if (isOfClassType(Class, sender))
        {
            // if this class is part of the object's hierarchy, this is also permitted
            if (isInstanceOf((RexxClass *)sender))
            {
                return;
            }
        }
        // we have this issued from an invalid context.
        reportException(Error_Execution_private_access, methodName);
    }
}


/**
 * Send a message to an object.
 *
 * @param message   The message name.
 * @param arguments An array of the arguments.
 * @param result    A ProtectedObject used for returning a result and
 *                  protecting it from garbage collection.
 */
RexxObject* RexxObject::sendMessage(RexxString *message, ArrayClass  *arguments, ProtectedObject &result)
{
    return messageSend(message, arguments->messageArgs(), arguments->messageArgCount(), result);
}


/**
 * Send a message to an object.
 *
 * @param message   The message name.
 * @param scope     The scope where to start searching.
 * @param arguments An array of the arguments.
 * @param result    A ProtectedObject used for returning a result and
 *                  protecting it from garbage collection.
 */
RexxObject* RexxObject::sendMessage(RexxString *message, RexxClass *scope, ArrayClass  *arguments, ProtectedObject &result)
{
    return messageSend(message, arguments->messageArgs(), arguments->messageArgCount(), scope, result);
}



/**
 * Send a message with two arguments.
 *
 * @param message   The message name.
 * @param argument1 The first argument.
 * @param argument2 The second argument.
 * @param result    A protected object for returning a result.
 */
RexxObject *RexxObject::sendMessage(RexxString *message, RexxObject *argument1, RexxObject *argument2, ProtectedObject &result)
{
    // copy the objects in to an array so they can be passed as a group.
    RexxObject *arguments[2];

    arguments[0] = argument1;
    arguments[1] = argument2;

    return messageSend(message, arguments, 2, result);
}


/**
 * Send a message with three arguments.
 *
 * @param message   The message name.
 * @param argument1 The first argument.
 * @param argument2 The second argument.
 * @param argument3 The third argument.
 * @param result    A protected object for returning a result.
 */
RexxObject *RexxObject::sendMessage(RexxString *message, RexxObject *argument1, RexxObject *argument2, RexxObject *argument3, ProtectedObject &result)
{
    RexxObject *arguments[3];

    arguments[0] = argument1;
    arguments[1] = argument2;
    arguments[2] = argument3;

    return messageSend(message, arguments, 3, result);
}


/**
 * Send a message with four arguments.
 *
 * @param message   The message name.
 * @param argument1 The first argument.
 * @param argument2 The second argument.
 * @param argument3 The third argument.
 * @param argument4 The fourth argument.
 * @param result    A protected object for returning a result.
 */
RexxObject *RexxObject::sendMessage(RexxString *message, RexxObject *argument1, RexxObject *argument2,
    RexxObject *argument3, RexxObject *argument4, ProtectedObject &result)
{
    RexxObject *arguments[4];

    arguments[0] = argument1;
    arguments[1] = argument2;
    arguments[2] = argument3;
    arguments[3] = argument4;

    return messageSend(message, arguments, 4, result);
}


/**
 * Send a message with five arguments.
 *
 * @param message   The message name.
 * @param argument1 The first argument.
 * @param argument2 The second argument.
 * @param argument3 The third argument.
 * @param argument4 The fourth argument.
 * @param argument5 The fifth argument.
 * @param result    A protected object for returning a result.
 */
RexxObject *RexxObject::sendMessage(RexxString *message, RexxObject *argument1, RexxObject *argument2, RexxObject *argument3,
    RexxObject *argument4, RexxObject *argument5, ProtectedObject &result)
{
    RexxObject *arguments[5];

    arguments[0] = argument1;
    arguments[1] = argument2;
    arguments[2] = argument3;
    arguments[3] = argument4;
    arguments[4] = argument5;

    return messageSend(message, arguments, 5, result);
}


/**
 * The real message sending method.
 *
 * @param msgname   The message name.
 * @param arguments Pointer to an array of message arguments.
 * @param count     The count of arguments.
 * @param result    A protected object for returning the message result.
 */
RexxObject *RexxObject::messageSend(RexxString *msgname, RexxObject **arguments, size_t  count, ProtectedObject &result)
{
    // check for a control stack condition
    ActivityManager::currentActivity->checkStackSpace();
    // see if we have a method defined
    MethodClass *method_save = behaviour->methodLookup(msgname);

    RexxErrorCodes error = Error_No_method_name;

    // method exists, but is is protected or private?
    if (method_save != OREF_NULL && method_save->isSpecial())
    {
        // a private method, do the privacy check.  This will return
        // OREF_NULL if not allowed
        if (method_save->isPrivate())
        {
            method_save = checkPrivate(method_save, error);
        }
        else if (method_save->isPackageScope())
        {
            method_save = checkPackage(method_save, error);
        }
        // a protected method...this gets special send handling
        if (method_save != OREF_NULL && method_save->isProtected())
        {
            processProtectedMethod(msgname, method_save, arguments, count, result);
            return result;
        }
    }

    // we have a method, go run it against this object
    if (method_save != OREF_NULL)
    {
        method_save->run(ActivityManager::currentActivity, this, msgname, arguments, count, result);
    }
    else
    {
        // not found, so check for an unknown method
        processUnknown(error, msgname, arguments, count, result);
    }
    return result;
}


/**
 * Send a message with a scope override.
 *
 * @param msgname    The target message name.
 * @param arguments  The array of arguments.
 * @param count      The count of arguments.
 * @param startscope The starting scope.
 * @param result     A protected object for returning the result.
 */
RexxObject *RexxObject::messageSend(RexxString *msgname, RexxObject **arguments, size_t count,
    RexxClass *startscope, ProtectedObject &result)
{
    // validate that the scope override is valid (FORWARD uses this method, this way no need to check TO option)
    validateScopeOverride(startscope);

    // perform a stack space check
    ActivityManager::currentActivity->checkStackSpace();

    // do the lookup using the starting scope
    MethodClass *method_save = superMethod(msgname, startscope);

    RexxErrorCodes error = Error_No_method_name;

    // perform the same private/protected checks as the normal case
    if (method_save != OREF_NULL && method_save->isSpecial())
    {
        if (method_save->isPrivate())
        {
            method_save = checkPrivate(method_save, error);
        }
        else if (method_save->isPackageScope())
        {
            method_save = checkPackage(method_save, error);
        }

        // a protected method...this gets special send handling
        if (method_save != OREF_NULL && method_save->isProtected())
        {
            processProtectedMethod(msgname, method_save, arguments, count, result);
            return result;
        }
    }
    // invoke the method if we have one, call unknown otherwise
    if (method_save != OREF_NULL)
    {
        method_save->run(ActivityManager::currentActivity, this, msgname, arguments, count, result);
    }
    else
    {
        processUnknown(error, msgname, arguments, count, result);
    }
    return result;
}


/**
 * Request permission for issuing a protected method.
 *
 * @param messageName
 *                  The name this method was invoked under.
 * @param targetMethod
 *                  The target method object.
 * @param arguments The method arguments.
 * @param count     The argument cound.
 * @param result    A protected object for returning a result.
 */
void RexxObject::processProtectedMethod(RexxString *messageName, MethodClass *targetMethod,
    RexxObject  **arguments, size_t count, ProtectedObject &result)
{
    // get the current security manager
    SecurityManager *manager = ActivityManager::currentActivity->getEffectiveSecurityManager();
    // the security manager can provide a new result
    if (manager->checkProtectedMethod(this, messageName, count, arguments, result))
    {
        return;
    }

    // we have permission from the security manager, go run this.
    targetMethod->run(ActivityManager::currentActivity, this, messageName, arguments, count, result);
}


/**
 * Process an unknown message condition on an object.
 *
 * @param error     The error number to raise.
 * @param messageName
 *                  The target message name.
 * @param arguments The message arguments.
 * @param count     The count of arguments.
 * @param result    The return result protected object.
 */
void RexxObject::processUnknown(RexxErrorCodes error, RexxString *messageName, RexxObject **arguments, size_t count, ProtectedObject &result)
{
    // first check to see if there is an unknown method on this object.
    MethodClass *method_save = behaviour->methodLookup(GlobalNames::UNKNOWN);
    // if it does not exist, then this is a NOMETHOD situation.  Need to
    // check for a condition handler before issuing the syntax error.
    if (method_save == OREF_NULL)
    {
        reportNomethod(error, messageName, this);
    }
    // we need to pass the arguments to the array as real arguments
    Protected<ArrayClass> argumentArray = new_array(count, arguments);

    // we need the actual arguments in a C array.  First argument is
    // the message name, second is the array of arguments
    RexxObject *unknown_arguments[2];
    unknown_arguments[0] = messageName;
    unknown_arguments[1] = (RexxObject *)argumentArray;
    // and go invoke the method.
    method_save->run(ActivityManager::currentActivity, this, GlobalNames::UNKNOWN, unknown_arguments, 2, result);
}


/**
 * Perform method lookup on an object instance.
 *
 * @param msgname The target message name.
 *
 * @return A method object, or OREF_NULL if the object does not have
 *         the method.
 */
MethodClass * RexxObject::methodLookup(RexxString *msgname)
{
    return behaviour->methodLookup(msgname);
}


/**
 * Convert a REXX object to a whole number value
 *
 * @param result the returned converted number.
 * @param digits The digits under which the conversion needs to be performed.
 *
 * @return true if the number converted, false otherwise.
 */
bool RexxInternalObject::numberValue(wholenumber_t &result, wholenumber_t digits)
{
    return requestString()->numberValue(result, digits);
}


/**
 * Convert a REXX object to a whole number value
 *
 * @param result the returned converted number.
 *
 * @return true if the number converted, false otherwise.
 */
bool RexxInternalObject::numberValue(wholenumber_t &result)
{
    return requestString()->numberValue(result);
}


/**
 * Convert a REXX object to an unsigned number
 *
 * @param result the returned converted number.
 * @param digits The digits under which the conversion needs to be performed.
 *
 * @return true if the number converted, false otherwise.
 */
bool RexxInternalObject::unsignedNumberValue(size_t &result, wholenumber_t digits)
{

    return requestString()->unsignedNumberValue(result, digits);
}


/**
 * Convert a REXX object to an unsigned number
 *
 * @param result the returned converted number.
 *
 * @return true if the number converted, false otherwise.
 */
bool RexxInternalObject::unsignedNumberValue(size_t &result)
{
    return requestString()->unsignedNumberValue(result);
}


/**
 * Convert a REXX object to a double value.
 *
 * @param result the returned converted number.
 *
 * @return true if the number converted, false otherwise.
 */
bool RexxInternalObject::doubleValue(double &result)
{

    return requestString()->doubleValue(result);
}


/**
 * Convert a primitive internal object to an integer value
 *
 * @param precision The precision used on the conversion.
 *
 * @return An integer object representation for the object or
 *         .nil
 */
RexxInteger * RexxInternalObject::integerValue(wholenumber_t precision)
{
    return requestString()->integerValue(precision);
}


/**
 * Convert a primitive internal object to a number string
 * equivalent.
 *
 * @return A number string object representation for the object
 *         or .nil
 */
NumberString * RexxInternalObject::numberString()
{
    return requestString()->numberString();
}


/**
 * Convert a primitive internal object to a string value
 *
 * @return The string value of the internal object, which is always
 *         "".
 */
RexxString *RexxInternalObject::stringValue()
{
    // to eliminate a lot of method overrides between RexxObject and RexxInternalObject,
    // we pass a lot of methods on to the string value.  We depend on internal objects
    // returning a reliable string object that will likely raise an error in places where
    // they are used unexpectedly.
    return GlobalNames::NULLSTRING;
}


/**
 * Convert an exported object to a string value.
 *
 * @return The default string value for an object, which is
 *         derived from the object name.
 */
RexxString *RexxObject::stringValue()
{
    ProtectedObject result;
    return (RexxString *)sendMessage(GlobalNames::OBJECTNAME, result);
}


/**
 * Handle a string conversion REQUEST for an internal object
 *
 * @return Always return .nil because default behaviour is to not return anything here.
 */
RexxString *RexxInternalObject::primitiveMakeString()
{
    return (RexxString *)TheNilObject;
}


/**
 * Handle a request to convert this object to a string value.
 *
 * @return Either a string value or .nil.
 */
RexxString *RexxInternalObject::makeString()
{
    // primitive classes that don't override never convert directly
    if (isBaseClass())
    {
        return (RexxString *)TheNilObject;
    }
    // some sort of subclass, so we need to issue the actual REQUEST method.
    else
    {
        ProtectedObject result;
        return (RexxString *)resultOrNil(((RexxObject *)this)->sendMessage(GlobalNames::REQUEST, GlobalNames::STRING, result));
    }
}


/**
 * Copy an object value into a compound variable tail under construction.
 *
 * @param tail   The target tail.
 */
void RexxInternalObject::copyIntoTail(CompoundVariableTail *tail)
{
    // use the string value and copy into the tail buffer.
    RexxString *value = requestString();
    value->copyIntoTail(tail);
}


/**
 * Handle an array conversion for a REXX object.
 *
 * @return The converted array, or TheNilObject for no result returned.
 */
ArrayClass *RexxInternalObject::makeArray()
{
    // base class behaviour is to not convert.
    if (isBaseClass())
    {
        return (ArrayClass *)TheNilObject;
    }
    else
    {
        ProtectedObject result;
        return (ArrayClass *)resultOrNil(((RexxObject *)this)->sendMessage(GlobalNames::REQUEST, GlobalNames::ARRAY, result));
    }
}


/**
 * Handle a string request for a REXX object.  This will go
 * through the whole search order to do the conversion.
 *
 * @return The converted string value or TheNilObject.
 */
RexxString *RexxInternalObject::requestString()
{
    // if we have a primitive object, we can get the string value directly.
    if (isBaseClass())
    {
        RexxString *string_value = primitiveMakeString();
        // if this didn't convert, we need to try to raise the NOSTRING condition.
        if (string_value == TheNilObject)
        {
            // get the ultimate, really needs to return a string string value for raising the condition.
            string_value = stringValue();
            // and raise nostring.  If not trapped, this returns here and we return the final string value as a result.
            ActivityManager::currentActivity->raiseCondition(GlobalNames::NOSTRING, OREF_NULL, string_value, (RexxObject *)this, OREF_NULL);
        }
        return string_value;
    }
    else
    {
        // we need to do this via a real request message.
        ProtectedObject string_value;

        ((RexxObject *)this)->sendMessage(GlobalNames::REQUEST, GlobalNames::STRING, string_value);
        // The returned value might be an Integer or NumberString value.  We need to
        // force this to be a real string value.
        string_value = ((RexxObject *)string_value)->primitiveMakeString();

        // if this did not convert, we send the STRING message to get a value
        if ((RexxObject *)string_value == TheNilObject)
        {
            ((RexxObject *)this)->sendMessage(GlobalNames::STRING, string_value);
            // we're really dependent upon the program respecting the protocol
            // here and returning a value.  It is possible there is a
            // problem, so how to handle this.  We could just raise an error, but this
            // isn't the most ideal message since the error is raised at the
            // line where the string value is required, but this is a rare
            // situation.  As a fallback, use the default object STRING method,
            // then raise an error if we still don't get anything.  This at least
            // keeps the interpreter from crashing, there's a good chance the
            // program will run.  Frankly, there's something seriously wrong
            // if this error ever gets issued.
            if (((RexxObject *)string_value) == OREF_NULL)
            {
                string_value = stringValue();
                if (((RexxObject *)string_value) == OREF_NULL)
                {
                    reportException(Error_No_result_object_message, GlobalNames::STRING);
                }
            }
            // The returned value might be an Integer or NumberString value.  We need to
            // force this to be a real string value.
            string_value = ((RexxObject *)string_value)->primitiveMakeString();
            // raise a NOSTRING condition
            ActivityManager::currentActivity->raiseCondition(GlobalNames::NOSTRING, OREF_NULL, (RexxString *)string_value, (RexxObject *)this, OREF_NULL);
        }
        // we finally have a string value of some sort.
        return (RexxString *)string_value;
    }
}


/**
 * Handle a string request for a REXX object.  This will go
 * through the whole search order to do the conversion, but will
 * not raise a NOSTRING condition.
 *
 * @return The converted string object.
 */
RexxString *RexxInternalObject::requestStringNoNOSTRING()
{
    // primitive classes have methods to do this directly
    if (isBaseClass())
    {
        RexxString *string_value;
        // this is for classes that handle string converstion
        string_value = primitiveMakeString();
        // if the class does not convert, then return the absolute default string value
        if (string_value == TheNilObject)
        {
            string_value = stringValue();
        }
        return string_value;
    }
    else
    {
        // have to do this via message sends
        ProtectedObject string_value;
        ((RexxObject *)this)->sendMessage(GlobalNames::REQUEST, GlobalNames::STRING, string_value);
        if ((RexxObject *)string_value == TheNilObject)
        {
            ((RexxObject *)this)->sendMessage(GlobalNames::STRING, string_value);
        }

        // we better get something here.
        return (RexxString *)string_value;
    }
}


/**
 * Base method for converting a Rexx object into a required
 * string value.  This takes the conversion as far as it can
 * without reporting an error.
 *
 * @return The converted string value or the .nil if this did
 *         not convert.
 */
RexxString *RexxInternalObject::requiredString()
{
    // base classes can handle directly
    if (isBaseClass())
    {
        return makeString();
    }
    else
    {
        // do via a message send with some no return value protection to keep us from
        // crashing.
        ProtectedObject result;
        RexxObject *string_value = resultOrNil(((RexxObject *)this)->sendMessage(GlobalNames::REQUEST, GlobalNames::STRING, result));
        if (string_value != TheNilObject)
        {
            // The returned value might be an Integer or NumberString value.  We need to
            // force this to be a real string value.
            string_value = ((RexxObject *)string_value)->primitiveMakeString();
        }
        return (RexxString *)string_value;
    }
}


/**
 * Handle a string request for a REXX object in a context where
 * the object MUST have a string value.
 *
 * @param position The argument position for error reporting.
 *
 * @return The converted string value.
 */
RexxString *RexxInternalObject::requiredString(size_t position )
{
    // try to convert first
    RexxObject *string_value = requiredString();

    // if this did not convert, give the error message
    if (string_value == TheNilObject)
    {
        reportException(Error_Invalid_argument_string, position);
    }

    // we should have a real string object here.
    return (RexxString *)string_value;
}


/**
 * Handle a string request for a REXX object in a context where
 * the object MUST have a string value.
 *
 * @param name   The name of the argument.
 *
 * @return The converted string value.
 */
RexxString *RexxInternalObject::requiredString(const char *name)
{
    // try to convert first
    RexxObject *string_value = requiredString();

    // if this did not convert, give the error message
    if (string_value == TheNilObject)
    {
        reportException(Error_Invalid_argument_string, name);
    }

    // we should have a real string object here.
    return (RexxString *)string_value;
}


/**
 * Request an integer value from an object.  If this is not a
 * primitive object, the object will be converted to a string,
 * and then the string integer value will be returned.
 *
 * @param precision The required precision for the conversion.
 *
 * @return An integer object version of this object, if possible.
 */
RexxInteger *RexxInternalObject::requestInteger(wholenumber_t precision )
{
    // primitive objects can optimize their own conversion.
    if (isBaseClass())
    {
        return integerValue(precision);
    }
    // try to convert the string value.
    else
    {
        return requestString()->integerValue(precision);
    }
}


/**
 * Get an integer value in a situation where a value is requied.
 *
 * @param position  The argument position for error messages.
 * @param precision The precision for the conversion.
 *
 * @return The converted object value.
 */
RexxInteger *RexxInternalObject::requiredInteger(size_t position, wholenumber_t precision)
{
    // do the common conversion
    RexxInteger *result = integerValue(precision);

    // if didn't convert, this is an error
    if (result == (RexxInteger *)TheNilObject)
    {
        reportException(Error_Incorrect_method_whole, position, (RexxObject *)this);
    }
    return result;
}


/**
 * Get an integer value in a situation where a value is requied.
 *
 * @param position  The argument name for error messages.
 * @param precision The precision for the conversion.
 *
 * @return The converted object value.
 */
RexxInteger *RexxInternalObject::requiredInteger(const char *position, wholenumber_t precision)
{
    // do the common conversion
    RexxInteger *result = integerValue(precision);

    // if didn't convert, this is an error
    if (result == (RexxInteger *)TheNilObject)
    {
        reportException(Error_Invalid_argument_whole, position, (RexxObject *)this);
    }
    return result;
}


/**
 * Request an object to convert itself into a number value.
 *
 * @param result    The numeric result value.
 * @param precision The precision used for the conversion.
 *
 * @return true if the object converted ok, false for a conversion failure.
 */
bool RexxInternalObject::requestNumber(wholenumber_t &result, wholenumber_t precision)
{
    // numberValue takes care of baseclass/subclass issues, so we just forward
    return numberValue(result, precision);
}


/**
 * Request an object to convert itself into a number value.
 *
 * @param result    The numeric result value.
 * @param precision The precision used for the conversion.
 *
 * @return true if the object converted ok, false for a conversion failure.
 */
bool RexxInternalObject::requestUnsignedNumber(size_t &result, wholenumber_t precision)
{
    // numberValue takes care of baseclass/subclass issues, so we just forward
    return unsignedNumberValue(result, precision);
}


/**
 * Request a whole number value from an object in a
 * situation where a value is required.
 *
 * @param position  The position of the argument used for error reporting.
 * @param precision The conversion precision.
 *
 * @return The converted whole number.
 */
wholenumber_t RexxInternalObject::requiredNumber(size_t position, wholenumber_t precision)
{
    wholenumber_t  result;

    // just convert via the appropriate means and raise the error
    if (!numberValue(result, precision))
    {
        reportException(Error_Incorrect_method_whole, position, (RexxObject *)this);
    }
    return result;
}


/**
 * Request a whole number value from an object in a
 * situation where a value is required.
 *
 * @param position  The position of the argument used for error reporting.
 * @param precision The conversion precision.
 *
 * @return The converted whole number.
 */
wholenumber_t RexxInternalObject::requiredNumber(const char *position, wholenumber_t precision)
{
    wholenumber_t  result;

    // just convert via the appropriate means and raise the error
    if (!numberValue(result, precision))
    {
        reportException(Error_Invalid_argument_whole, position, (RexxObject *)this);
    }
    return result;
}


/**
 * Request a positive whole number value from an object in a
 * situation where a value is required.
 *
 * @param position  The position of the argument used for error reporting.
 * @param precision The conversion precision.
 *
 * @return The converted whole number.
 */
size_t RexxInternalObject::requiredPositive(size_t position, wholenumber_t precision)
{
    size_t result;

    // convert and validate the result
    if (!unsignedNumberValue(result, precision) || result == 0)
    {
        reportException(Error_Incorrect_method_positive, position, (RexxObject *)this);
    }
    return result;
}


/**
 * Request a positive whole number value from an object in a
 * situation where a value is required.
 *
 * @param position  The position of the argument used for error reporting.
 * @param precision The conversion precision.
 *
 * @return The converted whole number.
 */
size_t RexxInternalObject::requiredPositive(const char *position, wholenumber_t precision)
{
    size_t result;

    // convert and validate the result
    if (!unsignedNumberValue(result, precision) || result == 0)
    {
        reportException(Error_Invalid_argument_positive, position, (RexxObject *)this);
    }
    return result;
}


/**
 * Request a non-negative whole number value from an object in a
 * situation where a value is required.
 *
 * @param position  The position of the argument used for error reporting.
 * @param precision The conversion precision.
 *
 * @return The converted whole number.
 */
size_t RexxInternalObject::requiredNonNegative(size_t position, wholenumber_t precision)
{
    size_t result;

    if (!unsignedNumberValue(result, precision))
    {
        reportException(Error_Incorrect_method_nonnegative, position, (RexxObject *)this);
    }
    return result;
}


/**
 * Request a non-negative whole number value from an object in a
 * situation where a value is required.
 *
 * @param position  The position of the argument used for error reporting.
 * @param precision The conversion precision.
 *
 * @return The converted whole number.
 */
size_t RexxInternalObject::requiredNonNegative(const char *position, wholenumber_t precision)
{
    size_t result;

    if (!unsignedNumberValue(result, precision))
    {
        reportException(Error_Invalid_argument_nonnegative, position, (RexxObject *)this);
    }
    return result;
}


/**
 * Request an array value from an object.
 *
 * @return The converted array value, or .nil if it did not convert.
 */
ArrayClass *RexxInternalObject::requestArray()
{
    // if this is a primitive object, apply a special fast-path for a real array object.
    if (isBaseClass())
    {
        if (isArray(this))
        {
            return (ArrayClass *)this;
        }
        // other primitive classes handle directly
        else
        {
            return makeArray();
        }
    }
    // for subclasses, this needs to go through the REQUEST method.
    else
    {
        ProtectedObject result;
        return(ArrayClass *)resultOrNil(((RexxObject *)this)->sendMessage(GlobalNames::REQUEST, GlobalNames::ARRAY, result));
    }
}


/**
 * Request a double floating point number value from an object in a
 * situation where a value is required.
 *
 * @param position  The position of the argument used for error reporting.
 * @param precision The conversion precision.
 *
 * @return The double floating point number.
 */
double RexxInternalObject::requiredFloat(const char *position)
{
    double result;

    if (!doubleValue(result))
    {
        reportException(Error_Invalid_argument_number, position, (RexxObject *)this);
    }
    return result;
}


/**
 * Retrieve the object name for an object.
 *
 * @return An explicitly set object name or the default object name value.
 */
RexxString *RexxObject::objectName()
{
    Protected<RexxString> string_value;

    // this is always stored in the object class scope
    string_value = (RexxString *)getObjectVariable(GlobalNames::OBJECTNAME, TheObjectClass);
    // if not found, we fall back to default means.
    if (string_value.isNull())
    {
        // if still a primitive class, we construct a default name
        if (isBaseClass())
        {
            return defaultName();
        }

        ProtectedObject result;
        // send the default name message...
        string_value = (RexxString *)sendMessage(GlobalNames::DEFAULTNAME, result);
        // it is possible we got nothing back from this method.  Prevent
        // potential crashes by returning the default default.
        if (string_value.isNull())
        {
            return defaultName();
        }
    }
    // we need to make sure this is a real string value.
    return string_value->stringValue();
}


/**
 * Assign a name to the the object.
 *
 * @param name   The name of the object.
 *
 * @return Returns nothing.
 */
RexxObject  *RexxObject::objectNameEquals(RexxObject *name)
{
    Protected<RexxString> objectName = stringArgument(name, ARG_ONE);
    // set the name in the object class scope
    setObjectVariable(GlobalNames::OBJECTNAME, objectName, TheObjectClass);
    return OREF_NULL;
}


/**
 * Handle "final" string coercion level
 *
 * @return The default string value for an object...generally just
 *         constructed from the class id.
 */
RexxString  *RexxInternalObject::defaultName()
{
    return new_string("Unknown class object");
}


/**
 * Handle "final" string coercion level
 *
 * @return The default string value for an object...generally just
 *         constructed from the class id.
 */
RexxString  *RexxObject::defaultName()
{
    RexxString *defaultname = behaviour->getOwningClass()->getId();
    // start with the ID.  If this is an enhanced object, add the modifier
    if (behaviour->isEnhanced())
    {
        return defaultname->concatToCstring("enhanced ");
    }
    // grammar counts!
    switch (defaultname->getChar(0))
    {
        // vowels get "an"
        case 'a':
        case 'A':
        case 'e':
        case 'E':
        case 'i':
        case 'I':
        case 'o':
        case 'O':
        case 'u':
        case 'U':

            defaultname = defaultname->concatToCstring("an ");
            break;

        // all others are "a"
        default:
            defaultname = defaultname->concatToCstring("a ");
            break;
    }
    return defaultname;
}


/**
 * Check for the presence of a method on an object.
 *
 * @param msgname The target message name.
 *
 * @return true if the object has the method, false otherwise.
 */
bool RexxObject::hasMethod(RexxString *msgname)
{

    return behaviour->hasMethod(msgname);
}


/**
 * Return the class object associated with an object
 *
 * @return The associated class object.
 */
RexxClass *RexxObject::classObject()
{
    return behaviour->getOwningClass();
}


/**
 * Add a new method to an object instance
 *
 * @param name    The name of the new method.
 * @param methobj The method object.
 * @param option  The scope option.
 *
 * @return Returns nothing.
 */
RexxObject *RexxObject::setMethod(RexxString *name, MethodClass *methobj, RexxString *option)
{
    // get the message name as a string
    Protected<RexxString> msgname = stringArgument(name, "method name")->upper();

    // by default, the added scope is .nil, which is the object scope.
    RexxClass *targetScope = (RexxClass *)TheNilObject;
    Protected<MethodClass> newMethod;

    // if not passed a method, we're hiding methods of this name, so use .nil for
    // the method object.
    if (methobj == OREF_NULL)
    {
        methobj = (MethodClass *)TheNilObject;
    }
    else
    {
        // make one from a string or array, setting the scope to .nil
        newMethod = MethodClass::newMethodObject(msgname, (RexxObject *)methobj, (RexxClass *)TheNilObject, "method");
        methobj = newMethod;
    }

    // By default, we add this method using the floating scope, which is a
    // non-specific scope shared by all set methods.  This can also be defined
    // using "OBJECT" scope, which really should be "CLASS" scope.  "OBJECT" scope
    // uses the
    if (option != OREF_NULL)
    {
        option = stringArgument(option, "scope option");

        // OBJECT scope means we attach this as a method of the defining class (top-level
        // of the hierarchy.
        if (Utilities::strCaselessCompare("OBJECT", option->getStringData()) == 0)
        {
            // define this scope on the class object, not the object level.
            targetScope = classObject();
        }
        // FLOAT is the only other possibility, which is the default.  This is
        // the .nil scope.
        else if (Utilities::strCaselessCompare("FLOAT",option->getStringData()) != 0)
        {
            reportException(Error_Invalid_argument_list, IntegerThree, new_string("\"FLOAT\" or \"OBJECT\""), option);

        }
    }

    // this has restrictions on how it can be used, check this context is valid.
    checkRestrictedMethod("SETMETHOD");

    // define the new method
    defineInstanceMethod(msgname, methobj, targetScope);
    return OREF_NULL;
}


/**
 * Remove a method from an object instance.
 *
 * @param name    The method we're removing.
 *
 * @return Returns nothing.
 */
RexxObject  *RexxObject::unsetMethod(RexxString *name)
{
    Protected<RexxString> msgname = stringArgument(name, "method name")->upper();

    // this has restrictions on how it can be used, check this context is valid.
    checkRestrictedMethod("UNSETMETHOD");

    // go remove the instance method
    deleteInstanceMethod(msgname);
    return OREF_NULL;
}


/**
 * Externalized version of the REQUEST method.  This tries to
 * convert one class of object into another.
 *
 * @param name      The class name to convert.
 *
 * @return The converted object, or .nil of can't be converted.
 */
RexxObject* RexxObject::requestRexx(RexxString *name)
{
    // we need this in uppercase to search for a method name.
    Protected<RexxString> className = stringArgument(name, ARG_ONE)->upper();
    Protected<RexxString> class_id = behaviour->getOwningClass()->getId()->upper();
    // Get "MAKE"||class methodname
    Protected<RexxString>make_method = className->concatToCstring("MAKE");
    // find the MAKExxxx method
    MethodClass *method = behaviour->methodLookup(make_method);
    // have this method?
    if (method != OREF_NULL)
    {
        ProtectedObject result;
        // go invoke the method and return the result
        return resultOrNil(sendMessage(make_method, result));
    }
    else
    {
        // Bug #1904: This test was orignally done first, however the MAKExxxx method might do more
        // than just return the same object. For example, MAKEARRAY for the array class returns
        // a new non-sparse array object. We only accept a class name match when there is not
        // corresponding MAKE method.

        // if the existing class id and the target name are the same, we will accept that
        if (className->strictEqual(class_id) == TheTrueObject)
        {
            return this;
        }
        return TheNilObject;
    }
}


/**
 * Validate a scope override on a message send.
 *
 * @param scope  The scope we're checking.
 */
void RexxObject::validateScopeOverride(RexxClass *scope)
{
    // validate the starting scope if we've been given one
    if (scope != OREF_NULL)
    {
        if (! this->behaviour->hasScope((RexxClass *)scope))
        {
            reportException(Error_Incorrect_method_array_noclass, this, scope);
        }
    }
}


/**
 * Do a dynamic invocation of an object method.
 *
 * @param message   The target message.  This can be either a string message
 *                  name or a string/scope pair to do a qualified invocation.
 * @param arguments An array of arguments to used with the message invocation.
 *
 * @return The method result.
 */
RexxObject *RexxObject::sendWith(RexxObject *message, ArrayClass *args)
{
    ProtectedObject messageName;
    ProtectedObject startScope;
    // decode and validate the message input
    decodeMessageName(this, message, messageName, startScope);

    // this needs to be protected in case this was converted to an array via makeArray
    Protected<ArrayClass> arguments = arrayArgument(args, "message arguments");
    ProtectedObject r;
    if (startScope == OREF_NULL)
    {
        messageSend(messageName, arguments->messageArgs(), arguments->messageArgCount(), r);
    }
    else
    {
        // validate that the scope override is valid
        validateScopeOverride(startScope);
        messageSend(messageName, arguments->messageArgs(), arguments->messageArgCount(), startScope, r);
    }
    return (RexxObject *)r;
}


/**
 * Do a dynamic invocation of an object method.
 *
 * @param arguments The variable arguments passed to the method.  The first
 *                  argument is a required message target, which can be either
 *                  a string method name or an array containing a name/scope
 *                  pair.  The remainder of the arguments are the message
 *                  arguments.
 * @param argCount
 *
 * @return The method result.
 */
RexxObject *RexxObject::send(RexxObject **arguments, size_t argCount)
{
    // we must have a message name argument
    if (argCount < 1 )
    {
        missingArgument("message name");
    }

    ProtectedObject messageName;
    ProtectedObject startScope;
    // decode and validate the message input
    decodeMessageName(this, arguments[0], messageName, startScope);

    ProtectedObject r;
    if (startScope == OREF_NULL)
    {
        messageSend(messageName, arguments + 1, argCount - 1, r);
    }
    else
    {
        // validate that the scope override is valid
        validateScopeOverride(startScope);
        messageSend(messageName, arguments + 1, argCount - 1, startScope, r);
    }
    return (RexxObject *)r;
}


/**
 * Perform a start() using arguments provided in an
 * array wrapper.
 *
 * @param message   The target message.  This can be either a string, or an
 *                  array containing a string/scope coupling.
 * @param arguments The message arguments.
 *
 * @return The message object.
 */
MessageClass *RexxObject::startWith(RexxObject *message, ArrayClass *args)
{
    // the message is required
    requiredArgument(message, "message name");
    // this is required and must be an array
    Protected<ArrayClass> arguments = arrayArgument(args, ARG_TWO);
    // the rest is handled by code common to startWith();
    return startCommon(message, arguments->messageArgs(), arguments->messageArgCount());
}


/**
 * Run a message send in another thread.
 *
 * @param arguments The list of arguments.  This is an open-ended argument
 *                  list.  The first argument is the message, the remaining
 *                  arguments are the message arguments.
 * @param argCount  The number of arguments we were invoked with.
 *
 * @return The count of arguments.
 */
MessageClass *RexxObject::start(RexxObject **arguments, size_t argCount)
{
    // we must have a message name argument
    if (argCount < 1 )
    {
        missingArgument("message name");
    }

    /* Get the message name.             */
    RexxObject *message = arguments[0];  /* get the message .                 */
                                         /* Did we receive a message name     */
    requiredArgument(message, "message name");
    // the rest is handled by code common to startWith();
    return startCommon(message, arguments + 1, argCount - 1);
}


/**
 * A common method to process either a start() or a
 * startWith() method call.
 *
 * @param message   The message name (which might be an array form)
 * @param arguments The array of arguments.
 * @param argCount  The number of passed arguments.
 *
 * @return The message object spun off to process this message.
 */
MessageClass *RexxObject::startCommon(RexxObject *message, RexxObject **arguments, size_t argCount)
{
    ProtectedObject messageName;
    ProtectedObject startScope;
    // decode and validate the message input
    decodeMessageName(this, message, messageName, startScope);

    // validate the starting scope now, if specified.  We'll validate this in this
    // thread first.
    validateScopeOverride(startScope);

    // creeate the new message object and start it.
    Protected<ArrayClass> argArray = new_array(argCount, arguments);

    Protected<MessageClass> newMessage = new MessageClass(this, messageName, startScope, argArray);
    newMessage->start();
    // the message object is our return value.
    return newMessage;
}


/**
 * A static method that can be used to decode the
 * various message argument varieties used with start(),
 * startWith(), and the Message class new.
 *
 * @param message    The input message.  This can be a message name or an
 *                   array containing a message name/startscope pairing.
 * @param messageName
 * @param startScope
 */
void RexxObject::decodeMessageName(RexxObject *target, RexxObject *message, ProtectedObject &messageName, ProtectedObject &startScope)
{
    // clear the starting scope
    startScope = OREF_NULL;

    requiredArgument(message, "message name");

    // if 1st arg is a string, we can do this quickly
    if (!isString(message))
    {
        // get the array form and verify we got a single-dimension array back.
        Protected<ArrayClass> messageArray = message->requestArray();
        if (messageArray == (ArrayClass *)TheNilObject)
        {
            reportException(Error_Incorrect_method_message_name, message);
        }

        // must be single dimension with two elements
        if (messageArray->isMultiDimensional() || messageArray->messageArgCount() != 2)
        {
            reportException(Error_Incorrect_method_message);
        }

        // get the message as a string in uppercase.
        messageName = stringArgument((RexxObject *)messageArray->get(1), "message name")->upper();
        startScope = (RexxClass *)messageArray->get(2);
        classArgument(startScope, TheClassClass, "SCOPE");

        // we only break this up into the component parts here.  The calling context
        // takes care of validating whether this is a valid call.
    }
    else
    {
        // force to a string value
        messageName = stringArgument(message, ARG_ONE)->upper();
    }
}


/**
 * Tag an object as having an UNINIT method
 */
void RexxInternalObject::requiresUninit()
{
    // mark this object as having an uninit method
    setHasUninit();
    memoryObject.addUninitObject(this);
}


/**
 * Run a method on an object as if it was part of the object's
 * behaviour.
 *
 * @param arguments Pointer to variable arguments.  First must be the
 *                  method object, all others are arguments.
 * @param argCount  The count of arguments.
 *
 * @return The method result.
 */
RexxObject *RexxObject::run(RexxObject **arguments, size_t argCount)
{
    Protected<ArrayClass> arglist;
    RexxObject **argumentPtr = NULL;
    size_t argcount = 0;

    // must have at least a first argument
    if (argCount == 0)
    {
        missingArgument("method");
    }

    // get the method object
    Protected<MethodClass> methobj = (MethodClass *)arguments[0];
    requiredArgument(methobj, "method");
    // make sure we have a method object, including creating one from source if necessary
    methobj = MethodClass::newMethodObject(GlobalNames::RUN, (RexxObject *)methobj, (RexxClass *)TheNilObject, "method");

    // if we have arguments, decode how we are supposed to handle method arguments.
    if (argCount > 1)
    {
        // this is now required
        char optionChar = optionArgument(arguments[1], "AI", "argument style");
        /* process the different options     */
        switch (optionChar)
        {
            // arguments are in an array.
            case 'A':
                {
                    // we must have just one additional argument if this is an array
                    if (argCount < 3)
                    {
                        missingArgument("argument array");
                    }
                    if (argCount > 3)
                    {
                        reportException(Error_Incorrect_method_maxarg, IntegerThree);
                    }
                    // get the argument array and make sure we have a good array
                    arglist = arrayArgument(arguments[2], "argument array");
                    // get the array specifics to pass along
                    argumentPtr = arglist->messageArgs();
                    argcount = arglist->messageArgCount();
                    break;
                }

            // individual arguments
            case 'I':
                // point to the array data for the third value onward
                argumentPtr = arguments + 2;
                argcount = argCount - 2;
                break;
        }
    }

    // this has restrictions on how it can be used, check this context is valid.
    checkRestrictedMethod("RUN");

    ProtectedObject result;
    // run the method and return the result
    methobj->run(ActivityManager::currentActivity, this, GlobalNames::UNNAMED_METHOD, argumentPtr, argcount, result);
    return result;
}


/**
 * Define a whole collection of methods on a class.  This is a
 * method that is only exposed during the image build, thus it
 * has fairly limited error checking.  The methods must be real
 * method objects.
 *
 * @param methods The directory of methods to add.
 *
 * @return returns nothing.
 */
RexxObject *RexxObject::defineInstanceMethods(DirectoryClass *methods)
{
    // use a copy of the behaviour
    setField(behaviour, (RexxBehaviour *)behaviour->copy());
    // loop through the list of methods
    for (HashContents::TableIterator iterator = methods->iterator(); iterator.isAvailable(); iterator.next())
    {
        MethodClass *method = (MethodClass *)iterator.value();
        // if not a method removal, change the scope
        if (method != TheNilObject)
        {
            // need validation here?
            method = method->newScope((RexxClass *)this);
        }
        else
        {
            method = OREF_NULL;       // this is a method removal
        }
        // Get the name for this method, in uppercase
        Protected<RexxString> name = (RexxString *)iterator.index();
        name = name->upper();
        behaviour->defineMethod(name, method);
    }
    return OREF_NULL;
}


/**
 * Add a method to an object's behaviour.  Used internally
 * during image build and for the Object setMethod() method.
 *
 * @param msgname The method name.
 * @param methobj The target method object.
 * @param scope   The scope the new method is defined with.
 *
 * @return Returns nothing.
 */
RexxObject *RexxObject::defineInstanceMethod(RexxString *msgname, MethodClass *methobj, RexxClass *scope)
{
    if (methobj != TheNilObject)
    {
        // set a new scope on this of the target (either .nil, or the object class)
        methobj = methobj->newScope(scope);
    }

    // copy primitive behaviour object and define the method, a copy is made to
    // ensure that we don't update the behaviour of any other object, since they
    // may have been sharing the mvd.
    setField(behaviour, (RexxBehaviour *)behaviour->copy());
    // add this to the behaviour
    behaviour->addInstanceMethod(msgname, methobj);

    // adding an UNINIT method to obj?
    checkUninit();
    return OREF_NULL;
}


/**
 * Remove a method to an object's instance behaviour with
 * unsetMethod.
 *
 * @param msgname The method name.
 * @param methobj The target method object.
 * @param scope   The scope the new method is defined with.
 *
 * @return Returns nothing.
 */
RexxObject *RexxObject::deleteInstanceMethod(RexxString *msgname)
{
    // copy primitive behaviour object and define the method, a copy is made to
    // ensure that we don't update the behaviour of any other object, since they
    // may have been sharing the mvd.
    setField(behaviour, (RexxBehaviour *)behaviour->copy());
    // add this to the behaviour
    behaviour->removeInstanceMethod(msgname);
    // we might have removed an uninit method, so check the status
    checkUninit();
    return OREF_NULL;
}


/**
 * Return the object's primitive type number
 *
 * @return The object type number.
 */
size_t RexxInternalObject::getObjectTypeNumber()
{
    return behaviour->getClassType();
}


/**
 * Remove an UNINIT method from an object
 */
void RexxInternalObject::removedUninit()
{
    // mark this object as not having an uninit method
    clearHasUninit();
    memoryObject.removeUninitObject(this);
}

/**
 * Search through all of the scopes looking for a variable
 * of the given name.  This will return the first match.
 *
 * @param name   The target name.
 *
 * @return The value associated with the variable or OREF_NULL if
 *         no matching variable is found.
 */
RexxObject *RexxObject::getObjectVariable(RexxString *name)
{
    VariableDictionary *dictionary = objectVariables;
    while (dictionary != OREF_NULL)
    {
        // see if this dictionary has the variable
        RexxObject *val =(RexxObject *)dictionary->realValue(name);
        // return this if it exists
        if (val != OREF_NULL)
        {
            return val;
        }
        // step to the next dictionary in the chain
        dictionary = dictionary->getNextDictionary();
    }
    return OREF_NULL;      // no variable found
}


/**
 * retrieve the value of an object variable.  This name
 * must be a name object, and only simple variables are supported.
 * If the variable has not been assigned a value, then OREF_NULL
 * is returned.
 *
 * @param name   The name of the variable.
 * @param scope  The target scope.
 *
 * @return The value of the variable, or OREF_NULL if not found.
 */
RexxObject *RexxObject::getObjectVariable(RexxString *name, RexxClass *scope)
{
    // if no scope give, use .nil as the scope to look at the object scope level.
    if (OREF_NULL == scope)
    {
        scope = (RexxClass *)TheNilObject;
    }

    // get the variable dictionary for the target scope level.  This will create a new
    // dictionary if we don't have one.
    VariableDictionary *ovd = getObjectVariables(scope);
    // get the variable value
    return (RexxObject *)ovd->realValue(name);
}


/**
 * assign a new value to a object variable.  This name
 * must be a name object, and only simple variables are supported.
 *
 * @param name   The target variable name.
 * @param value  The new variable value.
 * @param scope  The target dictionary scope.
 */
void RexxObject::setObjectVariable(RexxString *name, RexxObject *value, RexxClass *scope)
{
    // if no scope give, use .nil as the scope to look at the object scope level.
    if (OREF_NULL == scope)
    {
        scope = (RexxClass *)TheNilObject;
    }

    // get the variable dictionary for the target scope level.  This will create a new
    // dictionary if we don't have one.
    VariableDictionary *ovd = getObjectVariables(scope);
    ovd->set(name, value);
}


/**
 * Add a new variable dictionary to an object with a given scope
 *
 * @param dictionary The new dictionary.
 */
void RexxObject::addObjectVariables(VariableDictionary *dictionary)
{
    // chain any existing dictionaries off of the new one
    dictionary->setNextDictionary(objectVariables);
    // make this the new head of the chain
    setField(objectVariables, dictionary);
}


/**
 * Find the scope of a method's super class
 *
 * @param startScope The starting scope
 *
 * @return The next scope in the chain.
 */
RexxClass *RexxObject::superScope(RexxClass *startScope)
{
  return behaviour->superScope(startScope);
}

/**
 * Locate a method when qualified by a starting scope value.
 *
 * @param msgName    The target message name.
 * @param startScope The starting scope.
 *
 * @return The located method object, if any.
 */
MethodClass * RexxObject::superMethod(RexxString *msgName, RexxClass *startScope)
{
    return behaviour->superMethod(msgName, startScope);
}


/**
 * Retrieve an object dictionary for a given scope
 *
 * @param scope  The target scope.
 *
 * @return The associated variable dictionary.  This will create
 *         a new dictionary if an existing one is not found.
 */
VariableDictionary * RexxObject::getObjectVariables(RexxClass *scope)
{
    // run the dictionary chain looking for one created with that scope.
    VariableDictionary *dictionary = objectVariables;
    while (dictionary != OREF_NULL)
    {
        if (dictionary->isScope(scope))
        {
            return dictionary;
        }
        dictionary = dictionary->getNextDictionary();
    }

    // we don't have one with this scope level yet, so create a new one.
    dictionary = new_objectVariableDictionary(scope);
    // chain any existing dictionaries off of the new one
    dictionary->setNextDictionary(objectVariables);
    // make this the new head of the chain
    setField(objectVariables, dictionary);
    // we need to flip on the references field.
    setHasReferences();
    // return the new variable dictionary
    return dictionary;
}


/**
 * Obtain a guard lock on the target object at the given scope.
 *
 * @param activity The activity we're running on.
 * @param scope    The scope that needs to be locked.
 */
void RexxObject::guardOn(Activity *activity, RexxClass *scope)
{
    VariableDictionary *vdict = getObjectVariables(scope);
    vdict->reserve(activity);
}


/**
 * Release a guard lock on the target object at the given scope.
 *
 * @param activity The activity we're running on.
 * @param scope    The scope that needs to be released.
 */
void RexxObject::guardOff(Activity *activity, RexxClass *scope)
{
    VariableDictionary *vdict = getObjectVariables(scope);
    vdict->release(activity);
}


/**
 * Default Object init method, which is really a NOP.
 *
 * @return Returns nothing.
 */
RexxObject *RexxObject::initRexx()
{
    return OREF_NULL;
}


/**
 * Return a unique identity hash value for this object.  This
 * hash will be unique among the set of currently active Rexx
 * objects.
 *
 * @return The identity hash value as an integer object.
 */
RexxInteger *RexxObject::identityHashRexx()
{
    return new_integer(identityHash());
}


/**
 * Check if this is the Nil object.
 *
 * @return true if Nil, false otherwise.
 */
RexxObject *RexxObject::isNilRexx()
{
    return booleanObject(this == TheNilObject);
}


/**
 * Invoke an object's UNINIT method, if it has one.
 */
void RexxObject::uninit()
{
    if (hasMethod(GlobalNames::UNINIT))
    {
        ProtectedObject result;
        sendMessage(GlobalNames::UNINIT, result);
    }
}


/**
 * Check to see if an object has an UNINIT method.
 *
 * @return true if the object has an uninit, false otherwise.
 */
bool RexxObject::hasUninitMethod()
{
    return hasMethod(GlobalNames::UNINIT);
}


/**
 * Check to see if an object has an uninit method after
 * a change to the instance methods.
 */
void RexxObject::checkUninit()
{
    if (hasMethod(GlobalNames::UNINIT))
    {
        requiresUninit();
    }
    // this might be in the uninit table, remove it now.
    else
    {
        removedUninit();
    }
}


/**
 * Default new method for creating an object from Rexx
 * code.  This is the version inherited by all subclasses
 * of the Object class.
 *
 * @param arguments Pointer to the arguments of the new() method.  These
 *                  are passed on to the init method.
 * @param argCount  The count of the arguments.
 *
 * @return An initialized object instance.  If this is a subclass,
 *         the object will have all of the subclass behaviour.
 */
RexxObject *RexxObject::newRexx(RexxObject **arguments, size_t argCount)
{
    // this class is defined on the object class, but this is actually attached
    // to a class object instance.  Therefore, any use of the this pointer
    // will be touching the wrong data.  Use the classThis pointer for calling
    // any methods on this object from this method.
    RexxClass *classThis = (RexxClass *)this;

    RexxObject *newObj =  new RexxObject;
    ProtectedObject p(newObj);

    // handle Rexx class completion
    classThis->completeNewObject(newObj, arguments, argCount);

    return newObj;
}


/**
 * Copy the storage of an object and set up its header.
 * This method should be called by other copy() methods
 * instead ov using new_object and memcpy so that the
 * memory can properly initialize the new object's header
 * will be garbage collected properly.
 *
 * @return A new object copied for the source object.
 */
RexxInternalObject *RexxInternalObject::clone()
{
    // we need an identically sized object
    size_t size = getObjectSize();
    RexxInternalObject *cloneObj = new_object(size);
    // copy the object header.  That's the only piece of this we're not going to keep from
    // the old object.
    ObjectHeader newHeader = cloneObj->header;
    // copy everything but the object header over from the source object.
    memcpy((char *)cloneObj, (char *)this, size);
    // restore the new header to the cloned object
    cloneObj->header = newHeader;
    // we might be copying an oldspace object.  The new object, by definition,
    // will not be oldspace.
    cloneObj->setNewSpace();
    return cloneObj;
}


/**
 * Provide a dump of the header part of an object. This is generally
 * used when an invalid heap object is detected during garbage collection.
 */
void RexxInternalObject::dumpObject()
{
    printf("GC detected invalid object size=%zd (type=%zd, min=%zd, grain=%zd)" line_end, getObjectSize(), getObjectTypeNumber(), Memory::MinimumObjectSize, Memory::ObjectGrain);
    // hexdump the first 64 bytes
    unsigned char *s = (unsigned char *)this;
    for (int lines = 1; lines <= 2; lines++)
    {
        for (int blocks = 1; blocks <= 8; blocks++)
        {
            printf("%02x%02x%02x%02x ", *s, *(s + 1), *(s + 2), *(s + 3));
            s += 4;
        }
        printf(line_end);
    }
}


/**
 * Perform a simple validation on an object so we can detect corrupted
 * objects during garbage collection
 *
 * @return True if the object is value, false if it fails any of the validity tests.
 */
bool RexxInternalObject::isValid()
{
    // Test #1, is the size valid
    if (!Memory::isValidSize(getObjectSize()))
    {
        return false;
    }

#if 0
    // The following tests seem line a good idea, but unfortunately
    // it only works with live objects. A dead object on the pool
    // has neither a valid bahaviour pointer nor a valid virtual function pointer.
    // I'm leaving this in here just in case we figure out how to make these tests work.

    size_t typeNumber = getObjectTypeNumber();
    // Test #2 is the type indicator correct?
    if (typeNumber > T_Last_Class_Type)
    {
        return false;
    }
    // Test #3, is the object's virtual function pointer the correct one for the type.
    // this can detect problems where the front part of the object has been overlayed.
    if (!checkVirtualFunctions(memoryObject.virtualFunctionTable[typeNumber]))
    {
        return false;
    }
#endif

    // the object is at least consistent
    return true;
}


// macros for defining the standard operator methods.  These are
// essentially the same except for the names and the message target
#undef operatorMethod
#define operatorMethod(name, message) RexxObject * RexxObject::name(RexxObject *operand) \
{\
    ProtectedObject result;                                                     \
    messageSend(GlobalNames::message, &operand, 1, result);                      \
    if (result.isNull())                                                         \
    {  \
        reportException(Error_No_result_object_message, GlobalNames::message); \
    }  \
    return result;                                                               \
}\


#undef prefixOperatorMethod
#define prefixOperatorMethod(name, message) RexxObject * RexxObject::name(RexxObject *operand) \
{\
    ProtectedObject result;                                                     \
    messageSend(GlobalNames::message, &operand, operand == OREF_NULL ? 0 : 1, result); \
    if (result.isNull())                                                                        \
    {  \
        reportException(Error_No_result_object_message, GlobalNames::message); \
    }  \
    return result;                                                               \
}\


prefixOperatorMethod(operator_plus                , PLUS)
prefixOperatorMethod(operator_minus               , SUBTRACT)
operatorMethod(operator_multiply                  , MULTIPLY)
operatorMethod(operator_divide                    , DIVIDE)
operatorMethod(operator_integerDivide             , INTDIV)
operatorMethod(operator_remainder                 , REMAINDER)
operatorMethod(operator_power                     , POWER)
operatorMethod(operator_abuttal                   , NULLSTRING)
operatorMethod(operator_concat                    , CONCATENATE)
operatorMethod(operator_concatBlank               , BLANK)
operatorMethod(operator_equal                     , EQUAL)
operatorMethod(operator_notEqual                  , BACKSLASH_EQUAL)
operatorMethod(operator_isGreaterThan             , GREATERTHAN)
operatorMethod(operator_isBackslashGreaterThan    , BACKSLASH_GREATERTHAN)
operatorMethod(operator_isLessThan                , LESSTHAN)
operatorMethod(operator_isBackslashLessThan       , BACKSLASH_LESSTHAN)
operatorMethod(operator_isGreaterOrEqual          , GREATERTHAN_EQUAL)
operatorMethod(operator_isLessOrEqual             , LESSTHAN_EQUAL)
operatorMethod(operator_strictEqual               , STRICT_EQUAL)
operatorMethod(operator_strictNotEqual            , STRICT_BACKSLASH_EQUAL)
operatorMethod(operator_strictGreaterThan         , STRICT_GREATERTHAN)
operatorMethod(operator_strictBackslashGreaterThan, STRICT_BACKSLASH_GREATERTHAN)
operatorMethod(operator_strictLessThan            , STRICT_LESSTHAN)
operatorMethod(operator_strictBackslashLessThan   , STRICT_BACKSLASH_LESSTHAN)
operatorMethod(operator_strictGreaterOrEqual      , STRICT_GREATERTHAN_EQUAL)
operatorMethod(operator_strictLessOrEqual         , STRICT_LESSTHAN_EQUAL)
operatorMethod(operator_lessThanGreaterThan       , LESSTHAN_GREATERTHAN)
operatorMethod(operator_greaterThanLessThan       , GREATERTHAN_LESSTHAN)
operatorMethod(operator_and                       , AND)
operatorMethod(operator_or                        , OR)
operatorMethod(operator_xor                       , XOR)
prefixOperatorMethod(operator_not                 , BACKSLASH)


/**
 * Concatentation operation supported by the Object class.  This
 * converts the object into a string form, then asks the
 * converted object to perform the concatenaton.
 *
 * @param otherObj The object to concatenate.
 *
 * @return The concatenation result.
 */
RexxString *RexxObject::concatRexx(RexxObject *otherObj)
{
    RexxString *alias = requestString();
    return alias->concatRexx(otherObj);
}


/**
 * Blank concatentation operation supported by the Object class.
 * This converts the object into a string form, then asks the
 * converted object to perform the concatenaton.
 *
 * @param otherObj The object to concatenate.
 *
 * @return The concatenation result.
 */
RexxString *RexxObject::concatBlank(RexxObject *otherObj)
{
    RexxString *alias = requestString();
    return alias->concatBlank(otherObj);
}


/**
 * Exported access to an object virtual function
 *
 * @return The object string value.
 */
RexxString *RexxObject::stringRexx()
{
    return stringValue();
}


/**
 * Exported access to an object virtual function
 *
 * @return The object makestring result.
 */
RexxObject *RexxObject::makeStringRexx()
{
    return makeString();
}


/**
 * Exported access to an object virtual function
 *
 * @return The makeArray result.
 */
RexxObject *RexxObject::makeArrayRexx()
{
    return makeArray();
}


/**
 * Exported access to an object virtual function
 *
 * @return the object's default name.
 */
RexxString *RexxObject::defaultNameRexx()
{
    return defaultName();
}


/**
 * Exported access to an object virtual function
 *
 * @return A copy of the target object.
 */
RexxObject *RexxObject::copyRexx()
{
    return (RexxObject *)copy();
}


/**
 * Exported access to an object virtual function
 *
 * @param message The target message name.
 *
 * @return .true if the object has the method, .false otherwise.
 */
RexxObject *RexxObject::hasMethodRexx(RexxString *message )
{
    Protected<RexxString>messageName = stringArgument(message, ARG_ONE)->upper();
    return booleanObject(hasMethod(messageName));
}


/**
 * Create the NIL object instance.
 */
RexxNilObject::RexxNilObject()
{
    // The nil object needs to use a static hashcode so it remains
    // the same after an image install. We use an arbitrary hardcoded value
    // so that we get a constant value in the saved image file.
    hashValue = 0xDEADBEEF;
    // we are a special proxy object.
    makeProxiedObject();
}


/**
 * Override of the default hash value method.
 */
HashCode RexxNilObject::getHashValue()
{
    return hashValue;
}


/**
 * Attempt to get a CSELF value from an object instance
 * for a native context.
 *
 * @return An unwrapperd CSELF value, if one can be found.
 */
void *RexxObject::getCSelf()
{
    // try for the variable value
    RexxObject *C_self = getObjectVariable(GlobalNames::CSELF);
    // if we found one, validate for unwrappering
    if (C_self != OREF_NULL)
    {
        // if this is a pointer, then unwrapper the value
        if (C_self->isInstanceOf(ThePointerClass))
        {
            return ((PointerClass *)C_self)->pointer();
        }
        // this could be a containing buffer instance as well
        else if (C_self->isInstanceOf(TheBufferClass))
        {
            // return a pointer to the buffer beginning
            return(void *)((BufferClass *)C_self)->getData();
        }
    }
    return NULL;
}


/**
 * Attempt to get a CSELF value from an object instance,
 * starting from a given scope value and checking each of the
 * super scopes for the class
 *
 * @param scope  The starting scope for the search.
 *
 * @return An unwrappered CSELF value, if one can be found.
 */
void *RexxObject::getCSelf(RexxClass *scope)
{
    while (scope != TheNilObject)
    {
        // try for the variable value
        RexxObject *C_self = getObjectVariable(GlobalNames::CSELF, scope);
        // if we found one, validate for unwrappering
        if (C_self != OREF_NULL)
        {
            // if this is a pointer, then unwrapper the value
            if (C_self->isInstanceOf(ThePointerClass))
            {
                return ((PointerClass *)C_self)->pointer();
            }
            // this could be a containing buffer instance as well
            else if (C_self->isInstanceOf(TheBufferClass))
            {
                // return a pointer to the buffer beginning
                return(void *)((BufferClass *)C_self)->getData();
            }
        }
        // step to the next scope
        scope = superScope(scope);
    }
    return NULL;                     // no object available
}


/**
 * Obtain the special object memory table, creating one if
 * this is the first request.
 *
 * @return The memory table associated with this object.
 */
PointerTable *RexxObject::getMemoryTable()
{
    // this is always stored in the object class scope, using
    // a nullstring as a special name.
    PointerTable *table = (PointerTable *)getObjectVariable(GlobalNames::NULLSTRING, TheObjectClass);
    // if not found, we create a new one and set the variable
    if (table == OREF_NULL)
    {
        table = new PointerTable();
        setObjectVariable(GlobalNames::NULLSTRING, (RexxObject *)table, TheObjectClass);
    }

    return table;
}


/**
 * Allocate some memory associated with this object.
 *
 * @param size   The required memory size.
 *
 * @return A pointer to the allocated memory.
 */
void *RexxObject::allocateObjectMemory(size_t size)
{
    PointerTable *memoryTable = getMemoryTable();
    BufferClass *buffer = new_buffer(size);
    void *dataPointer = buffer->getData();
    memoryTable->put(buffer, dataPointer);
    return dataPointer;
}


/**
 * Free a buffer of objet-allocated memory.
 *
 * @param pointer The buffer pointer to release.
 */
void RexxObject::freeObjectMemory(void *pointer)
{
    // just remove the pointer from the table so that the buffer can
    // be garbage collected.
    PointerTable *memoryTable = getMemoryTable();
    memoryTable->remove(pointer);
}


/**
 * Reallocate a buffer of object memory, copying the
 * data into the new buffer.
 *
 * @param pointer The pointer for the existing memory.
 * @param newSize The new data size.
 *
 * @return The new data pointer.
 */
void *RexxObject::reallocateObjectMemory(void *pointer, size_t newSize)
{
    // get our table and access the old buffer object so we know what size
    // this was allocated to.
    PointerTable *memoryTable = getMemoryTable();
    BufferClass *oldBuffer = (BufferClass *)memoryTable->get(pointer);

    // if we don't find this in the table, this is an invalid
    // reallocate call.
    if (oldBuffer == OREF_NULL)
    {
        return NULL;
    }

    // we need the old size so we can copy the data to the new buffer
    size_t oldSize = oldBuffer->getBufferSize();

    // this is a nop if the size is not larger than the existing allocation.
    if (newSize <= oldSize)
    {
        return pointer;
    }

    // allocate a new object and copy the data over
    void * newPointer = allocateObjectMemory(newSize);
    memcpy(newPointer, pointer, std::min(oldSize, newSize));

    // remove the old pointer from the table
    memoryTable->remove(pointer);
    return newPointer;
}



/**
 * Call a method in the indexed object operator table.
 *
 * @param methodOffset
 *                 The index of the target operator.
 * @param argument The argument to the operator
 *
 * @return The operator method result.
 */
RexxObject *RexxObject::callOperatorMethod(size_t methodOffset, RexxObject *argument)
{
    // The behavior manages the operator tables.
    PCPPM cppEntry = behaviour->getOperatorMethod(methodOffset);
    // Go issue the method directly.
    return (this->*((PCPPM1)cppEntry))(argument);
}


/**
 * new operator for creating a RexxNilObject
 */
void *RexxNilObject::operator new(size_t size)
{
    // NOTE:  We create this using the Object behaviour, but
    // will switch the identifier after this has been customized.
    return new_object(size, T_Object);
}


/**
 * This is the default table of methods that implement
 * the operators.  The order of this table must match
 * the order of the OPERATOR_* definitions in the TokenSubclass
 * enum defined by the RexxToken class.
 */
PCPPM RexxObject::operatorMethods[] =
{
   NULL,
   (PCPPM)&RexxObject::operator_plus,
   (PCPPM)&RexxObject::operator_minus,
   (PCPPM)&RexxObject::operator_multiply,
   (PCPPM)&RexxObject::operator_divide,
   (PCPPM)&RexxObject::operator_integerDivide,
   (PCPPM)&RexxObject::operator_remainder,
   (PCPPM)&RexxObject::operator_power,
   (PCPPM)&RexxObject::operator_abuttal,
   (PCPPM)&RexxObject::operator_concat,
   (PCPPM)&RexxObject::operator_concatBlank,
   (PCPPM)&RexxObject::operator_equal,
   (PCPPM)&RexxObject::operator_notEqual,
   (PCPPM)&RexxObject::operator_isGreaterThan,
   (PCPPM)&RexxObject::operator_isBackslashGreaterThan,
   (PCPPM)&RexxObject::operator_isLessThan,
   (PCPPM)&RexxObject::operator_isBackslashLessThan,
   (PCPPM)&RexxObject::operator_isGreaterOrEqual,
   (PCPPM)&RexxObject::operator_isLessOrEqual,
   (PCPPM)&RexxObject::operator_strictEqual,
   (PCPPM)&RexxObject::operator_strictNotEqual,
   (PCPPM)&RexxObject::operator_strictGreaterThan,
   (PCPPM)&RexxObject::operator_strictBackslashGreaterThan,
   (PCPPM)&RexxObject::operator_strictLessThan,
   (PCPPM)&RexxObject::operator_strictBackslashLessThan,
   (PCPPM)&RexxObject::operator_strictGreaterOrEqual,
   (PCPPM)&RexxObject::operator_strictLessOrEqual,
   (PCPPM)&RexxObject::operator_lessThanGreaterThan,
   (PCPPM)&RexxObject::operator_greaterThanLessThan,
   (PCPPM)&RexxObject::operator_and,
   (PCPPM)&RexxObject::operator_or,
   (PCPPM)&RexxObject::operator_xor,
   (PCPPM)&RexxObject::operator_not,
};

