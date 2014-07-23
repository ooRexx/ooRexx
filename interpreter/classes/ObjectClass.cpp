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
/* Object REXX Kernel                                                         */
/*                                                                            */
/* The main REXX object definitions                                           */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "ObjectClass.hpp"
#include "StringClass.hpp"
#include "BufferClass.hpp"
#include "SmartBuffer.hpp"
#include "DirectoryClass.hpp"
#include "VariableDictionary.hpp"
#include "ArrayClass.hpp"
#include "Activity.hpp"
#include "RexxActivation.hpp"
#include "MessageClass.hpp"
#include "MethodClass.hpp"
#include "ExpressionBaseVariable.hpp"
#include "SourceFile.hpp"
#include "ProtectedObject.hpp"
#include "PointerClass.hpp"
#include "MethodArguments.hpp"


// singleton class instance
RexxClass *RexxObject::classInstance = OREF_NULL;
RexxObject *RexxNilObject::nilObject = OREF_NULL;


/**
 * Create initial class object at bootstrap time.
 */
void RexxObject::createInstance()
{
    CLASS_CREATE(Object, "Object", RexxClass);
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
    return new_object(size, T_Method);
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
RexxObject * RexxInternalObject::makeProxy(Envelope *envelope)
{
    // we are generally only called if the class is marked as a proxy class.
    // we recognize .nil, but don't handle any other special objects here.
    if (this == TheNilObject)
    {
        return(RexxObject *)new_proxy("NIL");
    }
    else
    {
        return(RexxObject *)this;
    }
}


/**
 * Set the object virtual function table and behaviour
 * to a particular type designation.
 *
 * @param type   The internal type number.
 */
void setObjectType(size_t type)
{
    setVirtualFunctions(memoryObject.virtualFunctionTable[type]);
    setBehaviour(RexxBehaviour::getPrimitiveBehaviour(type));
}


/**
 * primitive level equality method used by the hash collection
 * classes for determining equality.  The comparison is
 * done based on object identity (pointer value).
 *
 * @param other  The comparison object.
 *
 * @return true if the objects are equal, false otherwise.
 */
bool RexxInternalObject::isEqual(RexxObject *other )
{
    return ((RexxObject *)this) == other;/* simple identity equality          */
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
bool RexxObject::isEqual(RexxObject *other)
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
        sendMessage(OREF_STRICT_EQUAL, other, result);
        return ((RexxObject *)result)->truthValue(Error_Logical_value_method);
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
wholenumber_t RexxObject::compareTo(RexxObject *other )
{
    ProtectedObject result;

    sendMessage(OREF_COMPARETO, other, result);
    // the result is required
    if ((RexxObject *)result == OREF_NULL)
    {
        reportException(Error_No_result_object_message, OREF_COMPARETO);
    }
    wholenumber_t comparison;

    // the comparison value is a signed number, it has to convert
    if (!((RexxObject *)result)->numberValue(comparison))
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
    requiredArgument(other, ARG_ONE);
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
 * @param method_name
 *               The method name.
 *
 * @return The method object that implements the object method.
 */
MethodClass *RexxObject::instanceMethod(RexxString *method_name)
{
    // the name must be a string...and we use it in upper case
    method_name = stringArgument(method_name, ARG_ONE)->upper();
    // retrieve the method from the dictionary
    MethodClass *method_object = (MethodClass *)behaviour->getMethodDictionary()->stringGet(method_name);
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
    return (RexxObject *)new_string((char *)&h, sizeof(HashCode));
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
        sendMessage(OREF_HASHCODE, result);
        // TODO:  probably need to have a value check here.

        // the default version sends us a string containing binary data.
        // if the string is long enough for that, we reverse the process.  Otherwise,
        // we'll just take the hash code from the string object.
        return ((RexxObject *)result)->stringValue()->stringToHashCode();
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
bool RexxInternalObject::truthValue(int errorCode)
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
RexxObject * RexxInternalObject::copy()
{
    // Instead of calling new_object and memcpy, ask the memory object to make
    // a copy of ourself.  This way, any header information can be correctly
    // initialized by memory.
    return (RexxObject *)clone();
}


/**
 * Copy an object that is visible to the Rexx programmer.
 * This copy ensures that all object variables are copied,
 * as well as ensuring that the new instance has the
 * same behaviour as the initial object.
 *
 * @return A new instance of this object.
 */
RexxObject *RexxObject::copy()
{
    // Instead of calling new_object and memcpy, ask the memory object to make
    // a copy of ourself.  This way, any header information can be correctly
    // initialized by memory.
    Protected<RexxObject> *newObj = clone();

    // do we have object variables?  We need to give that opject
    // a copy of the variables
    copyObjectVariables(newObj);

    // have instance methods?
    if (behaviour->hasInstanceMethods())
    {
        // need to copy the behaviour
        newObj->setBehaviour((RexxBehaviour *)newObj->behaviour->copy());
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
 *
 * @return An executable method, or OREF_NULL if this cannot be called.
 */
MethodClass * RexxObject::checkPrivate(MethodClass *method )
{
    // TODO:  are there places where the activation context can be passed in?
    // get the calling activaiton context
    RexxActivationBase *activation = ActivityManager::currentActivity->getTopStackFrame();
    if (activation != OREF_NULL)
    {
        // if the sending and receiving object are the same, this is allowed.
        RexxObject *sender = activation->getReceiver();
        if (sender == (RexxObject *)this)
        {
            return method;
        }
        // no sender means this is a routine or program context.  Definitely not allowed.
        if (sender == OREF_NULL)
        {
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
    return OREF_NULL;
}


/**
 * Send a message to an object.
 *
 * @param message The name of the message.
 * @param args    The message arguments.
 *
 * @return The message result.
 */
RexxObject *RexxObject::sendMessage(RexxString *message, ArrayClass *args)
{
    ProtectedObject r;
    sendMessage(message, args, r);
    return (RexxObject *)r;
}


/**
 * Another method signature for sending a message.
 *
 * @param message The message name.
 *
 * @return The message result.
 */
RexxObject *RexxObject::sendMessage(RexxString *message)
{
    ProtectedObject r;
    sendMessage(message, r);
    return (RexxObject *)r;
}


/**
 * Another convenience method for sending a message.
 *
 * @param message  The message name.
 * @param args     Pointer to a variable size argument list.
 * @param argCount The cound of arguments.
 *
 * @return The message result.
 */
RexxObject *RexxObject::sendMessage(RexxString *message, RexxObject **args, size_t argCount)
{
    ProtectedObject r;
    sendMessage(message, args, argCount, r);
    return (RexxObject *)r;
}


/**
 * Set a message with a single argument.
 *
 * @param message   The message name.
 * @param argument1 The message argument.
 *
 * @return The message result.
 */
RexxObject *RexxObject::sendMessage(RexxString *message, RexxObject *argument1)
{
    ProtectedObject r;
    sendMessage(message, argument1, r);
    return (RexxObject *)r;
}


/**
 * Send a message with two arguments.
 *
 * @param message   The message name.
 * @param argument1 The first argument.
 * @param argument2 The second argument.
 *
 * @return The message result.
 */
RexxObject *RexxObject::sendMessage(RexxString *message, RexxObject *argument1, RexxObject *argument2)
{
    ProtectedObject r;
    sendMessage(message, argument1, argument2, r);
    return (RexxObject *)r;
}


/**
 * Send a message with three arguments.
 *
 * @param message   The message name.
 * @param argument1 The first argument.
 * @param argument2 The second argument.
 * @param argument3 The third argument.
 *
 * @return The message result.
 */
RexxObject *RexxObject::sendMessage(RexxString *message, RexxObject *argument1, RexxObject *argument2, RexxObject *argument3)
{
    ProtectedObject r;
    sendMessage(message, argument1, argument2, argument3, r);
    return (RexxObject *)r;
}


/**
 * Send a message with four arguments.
 *
 * @param message   The message name.
 * @param argument1 The first argument.
 * @param argument2 The second argument.
 * @param argument3 The third argument.
 * @param argument4 The fourth argument.
 *
 * @return The message result.
 */
RexxObject *RexxObject::sendMessage(RexxString *message, RexxObject *argument1, RexxObject *argument2, RexxObject *argument3, RexxObject *argument4)
{
    ProtectedObject r;
    sendMessage(message, argument1, argument2, argument3, argument4, r);
    return (RexxObject *)r;
}


/**
 * Send a message with five arguments.
 *
 * @param message   The message name.
 * @param argument1 The first argument.
 * @param argument2 The second argument.
 * @param argument3 The third argument.
 * @param argument4 The fifth argument.
 *
 * @return The message result.
 */
RexxObject *RexxObject::sendMessage(RexxString *message, RexxObject *argument1, RexxObject *argument2, RexxObject *argument3, RexxObject *argument4, RexxObject *argument5)
{
    ProtectedObject r;
    sendMessage(message, argument1, argument2, argument3, argument4, argument5, r);
    return (RexxObject *)r;
}


/**
 * Send a message to an object.
 *
 * @param message   The message name.
 * @param arguments An array of the arguments.
 * @param result    A ProtectedObject used for returning a result and
 *                  protecting it from garbage collection.
 */
void RexxObject::sendMessage(RexxString *message, ArrayClass  *arguments, ProtectedObject &result)
{
    messageSend(message, arguments->data(), arguments->size(), result);
}


/**
 * Send a message with two arguments.
 *
 * @param message   The message name.
 * @param argument1 The first argument.
 * @param argument2 The second argument.
 * @param result    A protected object for returning a result.
 */
void RexxObject::sendMessage(RexxString *message, RexxObject *argument1, RexxObject *argument2, ProtectedObject &result)
{
    // copy the objects in to an array so they can be passed as a group.
    RexxObject *arguments[2];

    arguments[0] = argument1;
    arguments[1] = argument2;

    messageSend(message, arguments, 2, result);
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
void RexxObject::sendMessage(RexxString *message, RexxObject *argument1, RexxObject *argument2, RexxObject *argument3, ProtectedObject &result)
{
    RexxObject *arguments[3];

    arguments[0] = argument1;
    arguments[1] = argument2;
    arguments[2] = argument3;

    messageSend(message, arguments, 3, result);
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
void RexxObject::sendMessage(RexxString *message, RexxObject *argument1, RexxObject *argument2,
    RexxObject *argument3, RexxObject *argument4, ProtectedObject &result)
{
    RexxObject *arguments[4];

    arguments[0] = argument1;
    arguments[1] = argument2;
    arguments[2] = argument3;
    arguments[3] = argument4;

    messageSend(message, arguments, 4, result);
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
void RexxObject::sendMessage(RexxString *message, RexxObject *argument1, RexxObject *argument2, RexxObject *argument3,
    RexxObject *argument4, RexxObject *argument5, ProtectedObject &result)
{
    RexxObject *arguments[5];

    arguments[0] = argument1;
    arguments[1] = argument2;
    arguments[2] = argument3;
    arguments[3] = argument4;
    arguments[4] = argument5;

    messageSend(message, arguments, 5, result);
}


/**
 * The real message sending method.
 *
 * @param msgname   The message name.
 * @param arguments Pointer to an array of message arguments.
 * @param count     The count of arguments.
 * @param result    A protected object for returning the message result.
 */
void RexxObject::messageSend(RexxString *msgname, RexxObject **arguments, size_t  count, ProtectedObject &result)
/******************************************************************************/
/* Function:    send a message (with message lookup) to an object.            */
/*              All types of methods are handled and dispatched               */
/******************************************************************************/
{
    // check for a control stack condition
    ActivityManager::currentActivity->checkStackSpace();
    // see if we have a method defined
    MethodClass *method_save = behaviour->methodLookup(msgname);

    // method exists, but is is protected or private?
    if (method_save != OREF_NULL && method_save->isSpecial())
    {
        // a private method, do the privacy check.  This will return
        // OREF_NULL if not allowed
        if (method_save->isPrivate())
        {
            method_save = checkPrivate(method_save);
        }
        // a protected method...this gets special send handling
        if (method_save != OREF_NULL && method_save->isProtected())
        {
            processProtectedMethod(msgname, method_save, arguments, count, result);
            return;
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
        processUnknown(msgname, arguments, count, result);
    }
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
void RexxObject::messageSend(RexxString *msgname, RexxObject **arguments, size_t count,
    RexxObject *startscope, ProtectedObject &result)
{
    // perform a stack space check
    ActivityManager::currentActivity->checkStackSpace();
    // do the lookup using the starting scope
    MethodClass *method_save = superMethod(msgname, startscope);

    // perform the same private/protected checks as the normal case
    if (method_save != OREF_NULL && method_save->isProtected())
    {
        if (method_save->isPrivate())
        {
            method_save = checkPrivate(method_save);
        }
        else
        {
            processProtectedMethod(msgname, method_save, arguments, count, result);
            return;
        }
    }
    // invoke the method if we have one, call unknown otherwise
    if (method_save != OREF_NULL)
    {
        method_save->run(ActivityManager::currentActivity, this, msgname, arguments, count, result);
    }
    else
    {
        processUnknown(msgname, arguments, count, result);
    }
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
/******************************************************************************/
/* Function:  Process an unknown message, uncluding looking for an UNKNOWN    */
/*            method and raising a NOMETHOD condition                         */
/******************************************************************************/
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
 * @param messageName
 *                  The target message name.
 * @param arguments The message arguments.
 * @param count     The count of arguments.
 * @param result    The return result protected object.
 */
void RexxObject::processUnknown(RexxString *messageName, RexxObject **arguments, size_t count, ProtectedObject &result)
{
    // first check to see if there is an unknown method on this object.
    MethodClass *method_save = behaviour->methodLookup(OREF_UNKNOWN);
    // if it does not exist, then this is a NOMETHOD situation.  Need to
    // check for a condition handler before issuing the syntax error.
    if (method_save == OREF_NULL)
    {
        reportNomethod(messageName, this);
    }

    // we need to pass the arguments to the array as real arguments
    Protected<ArrayClass> argumentArray = new_array(arguments, count);

    // we need the actual arguments in a C array.  First argument is
    // the message name, second is the array of arguments
    RexxObject *unknown_arguments[2];
    unknown_arguments[0] = messageName;
    unknown_arguments[1] = (RexxObject *)argumentArray;
    // and go invoke the method.
    method_save->run(ActivityManager::currentActivity, this, OREF_UNKNOWN, unknown_arguments, 2, result);
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
bool RexxInternalObject::numberValue(wholenumber_t &result, stringsize_t digits)
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
bool RexxInternalObject::unsignedNumberValue(stringsize_t &result, stringsize_t digits)
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
bool RexxInternalObject::unsignedNumberValue(stringsize_t &result)
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
RexxInteger * RexxInternalObject::integerValue(size_t precision)
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
    return OREF_NULLSTRING;
}


/**
 * Convert an exported object to a string value.
 *
 * @return The default string value for an object, which is
 *         derived from the object name.
 */
RexxString *RexxObject::stringValue()
{
    return (RexxString *)sendMessage(OREF_OBJECTNAME);
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
        return (RexxString *)resultOrNil(((RexxObject *)this)->sendMessage(OREF_REQUEST, OREF_STRINGSYM));
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
        return (ArrayClass *)resultOrNil(((RexxObject *this)->sendMessage(OREF_REQUEST, OREF_ARRAYSYM));
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
            ActivityManager::currentActivity->raiseCondition(OREF_NOSTRING, OREF_NULL, string_value, (RexxObject *)this, OREF_NULL);
        }
        return string_value;
    }
    else
    {
        // we need to do this via a real request message.
        ProtectedObject string_value;

        ((RexxObject *)this)->sendMessage(OREF_REQUEST, OREF_STRINGSYM, string_value);
        // The returned value might be an Integer or NumberString value.  We need to
        // force this to be a real string value.
        string_value = ((RexxObject *)string_value)->primitiveMakeString();

        // if this did not convert, we send the STRING message to get a value
        if ((RexxObject *)string_value == TheNilObject)
        {
            ((RexxObject *)this)->sendMessage(OREF_STRINGSYM, string_value);
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
                    reportException(Error_No_result_object_message, OREF_STRINGSYM);
                }
            }
            // The returned value might be an Integer or NumberString value.  We need to
            // force this to be a real string value.
            string_value = ((RexxObject *)string_value)->primitiveMakeString();
            // raise a NOSTRING condition
            ActivityManager::currentActivity->raiseCondition(OREF_NOSTRING, OREF_NULL, (RexxString *)string_value, (RexxObject *)this, OREF_NULL);
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
        ((RexxObject *)this)->sendMessage(OREF_REQUEST, OREF_STRINGSYM, string_value);
        if ((RexxObject *)string_value == TheNilObject)
        {
            ((RexxObject *)this)->sendMessage(OREF_STRINGSYM, string_value);
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
RexxObject *RexxInternalObject::requiredString()
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
        RexxObject *string_value = resultOrNil(((RexxObject *)this)->sendMessage(OREF_REQUEST, OREF_STRINGSYM));
        if (string_value != TheNilObject)
        {
            // The returned value might be an Integer or NumberString value.  We need to
            // force this to be a real string value.
            string_value = ((RexxObject *)string_value)->primitiveMakeString();
        }
        return string_value;
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
        reportException(Error_Incorrect_method_nostring, position);
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
// TODO:  Convert more error checking to this form.
RexxString *RexxInternalObject::requiredString(const char *name)
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
 * Request an integer value from an object.  If this is not a
 * primitive object, the object will be converted to a string,
 * and then the string integer value will be returned.
 *
 * @param precision The required precision for the conversion.
 *
 * @return An integer object version of this object, if possible.
 */
RexxInteger *RexxInternalObject::requestInteger(size_t precision )
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
RexxInteger *RexxInternalObject::requiredInteger(size_t position, size_t precision)
{
    // do the common conversion
    RexxInteger *result = requiredInteger(precision);

    // if didn't convert, this is an error
    if (result == (RexxInteger *)TheNilObject)
    {
        reportException(Error_Incorrect_method_whole, position, (RexxObject *)this);
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
bool RexxInternalObject::requestNumber(wholenumber_t &result, size_t precision)
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
bool RexxInternalObject::requestUnsignedNumber(stringsize_t &result, size_t precision)
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
wholenumber_t RexxInternalObject::requiredNumber(size_t position, size_t precision)
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
 * Request a positive whole number value from an object in a
 * situation where a value is required.
 *
 * @param position  The position of the argument used for error reporting.
 * @param precision The conversion precision.
 *
 * @return The converted whole number.
 */
stringsize_t RexxInternalObject::requiredPositive(size_t position, size_t precision)
{
    stringsize_t result;

    // convert and validate the result
    if (!unsignedNumberValue(result, precision) || result == 0)
    {
        reportException(Error_Incorrect_method_positive, position, (RexxObject *)this);
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
// TODO:  should we have versions of this that take a name?
stringsize_t RexxInternalObject::requiredNonNegative(size_t position, size_t precision)
{
    stringsize_t result;

    if (!unsignedNumberValue(result, precision))
    {
        reportException(Error_Incorrect_method_nonnegative, position, (RexxObject *)this);
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
        if (isOfClass(Array, this))
        {
            return(ArrayClass *)this;
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
        return(ArrayClass *)resultOrNil(((RexxObject *)this)->sendMessage(OREF_REQUEST, OREF_ARRAYSYM));
    }
}


/**
 * Retrieve the object name for an object.
 *
 * @return An explicitly set object name or the default object name value.
 */
RexxString *RexxObject::objectName()
{
    ProtectedObject string_value;

    // this is always stored in the object class scope
    string_value = getObjectVariable(OREF_NAME, TheObjectClass);
    // if not found, we fall back to default means.
    if ((RexxObject *)string_value == OREF_NULL)
    {
        // if still a primitive class, we construct a default name
        if (isBaseClass())
        {
            return defaultName();
        }

        // send the default name message...
        sendMessage(OREF_DEFAULTNAME, string_value);
        // it is possible we got nothing back from this method.  Prevent
        // potential crashes by returning the default default.
        if ((RexxObject *)string_value == OREF_NULL)
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
    name = requiredStringArgument(name, ARG_ONE);
    // set the name in the object class scope
    setObjectVariable(OREF_NAME, name, TheObjectClass);
    return OREF_NULL;
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

    return booleanObject(behaviour->methodLookup(msgname) != OREF_NULL);
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
 * @param msgname The name of the new method.
 * @param methobj The method object.
 * @param option  The scope option.
 *
 * @return Returns nothing.
 */
RexxObject  *RexxObject::setMethod(RexxString *msgname, MethodClass *methobj, RexxString *option)
{
    // get the message name as a string
    msgname = stringArgument(msgname, ARG_ONE)->upper();
    if (option != OREF_NULL)
    {
        option = stringArgument(option, ARG_THREE);
        if (!Utilities::strCaselessCompare("OBJECT", option->getStringData()))
        {
            // do nothing if OBJECT
        }
        else if (!Utilities::strCaselessCompare("FLOAT",option->getStringData()))
        {
            // "FLOAT" makes option a NULL pointer, causing the old default behaviour on setMethod...
            option = OREF_NULL;
        }
        else
        {
            reportException(Error_Incorrect_call_list, CHAR_SETMETHOD, IntegerThree, "\"FLOAT\", \"OBJECT\"", option);
        }
    }

    // if not passed a method, we're hiding methods of this name, so use .nil for
    // the method object.
    if (methobj == OREF_NULL)
    {
        methobj = (MethodClass *)TheNilObject;
    }
    else
    {
        // make one from a string or array, setting the scope to .nil
        methobj = MethodClass::newMethodObject(msgname, (RexxObject *)methobj, ((RexxClass *)TheNilObject, IntegerTwo);
    }
    // define the new method
    defineMethod(msgname, methobj, option);
    return OREF_NULL;
}


/**
 * Remove a method from an object instance.
 *
 * @param msgname The method we're removing.
 *
 * @return Returns nothing.
 */
RexxObject  *RexxObject::unsetMethod(RexxString *msgname)
{
    msgname = stringArgument(msgname, ARG_ONE)->upper();
    // the behaviour does the heavy lifting here.
    behaviour->removeMethod(msgname);
    return OREF_NULL;
}


/**
 * Externalized version of the REQUEST method.  This tries to
 * convert one class of object into another.
 *
 * @param className The class name to convert.
 *
 * @return The converted object, or .nil of can't be converted.
 */
RexxObject  *RexxObject::requestRexx(RexxString *className)
{
    // we need this in uppercase to search for a method name.
    className = stringArgument(className, ARG_ONE)->upper();
    RexxString *class_id = id()->upper();
    // if the existing class id and the target name are the same, we are there already.
    if (className->strictEqual(class_id) == TheTrueObject)
    {
        return this;
    }
    // Get "MAKE"||class methodname
    RexxString *make_method = className->concatToCstring(CHAR_MAKE);
    // find the MAKExxxx method
    MethodClass *method = behaviour->methodLookup(make_method);
    // have this method?
    if (method != OREF_NULL)
    {
        // go invoke the method and return the result
        return resultOrNil(sendMessage(make_method));
    }
    else
    {
        return TheNilObject;
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
RexxObject *RexxObject::sendWith(RexxObject *message, ArrayClass *arguments)
{
    RexxString *messageName;
    RexxObject *startScope;
    // decode and validate the message input
    decodeMessageName(this, message, messageName, startScope);
    arguments = arrayArgument(arguments, ARG_TWO);

    ProtectedObject r;
    if (startScope == OREF_NULL)
    {
        messageSend(messageName, arguments->data(), arguments->size(), r);
    }
    else
    {
        messageSend(messageName, arguments->data(), arguments->size(), startScope, r);
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
    if (argCount < 1 )                   /* no arguments?                     */
    {
        missingArgument(ARG_ONE);         /* Yes, this is an error.            */
    }

    RexxString *messageName;
    RexxObject *startScope;
    // decode and validate the message input
    decodeMessageName(this, arguments[0], messageName, startScope);

    ProtectedObject r;
    if (startScope == OREF_NULL)
    {
        messageSend(messageName, arguments + 1, argCount - 1, r);
    }
    else
    {
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
MessageClass *RexxObject::startWith(RexxObject *message, ArrayClass *arguments)
{
    // the message is required
    requiredArgument(message, ARG_ONE);
    // this is required and must be an array
    arguments = arrayArgument(arguments, ARG_TWO);
    // the rest is handled by code common to startWith();
    return startCommon(message, arguments->data(), arguments->size());
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
    if (argCount < 1 )                   /* no arguments?                     */
    {
        missingArgument(ARG_ONE);         /* Yes, this is an error.            */
    }
    /* Get the message name.             */
    RexxObject *message = arguments[0];  /* get the message .                 */
                                         /* Did we receive a message name     */
    requiredArgument(message, ARG_ONE);
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
    RexxString *messageName;
    RexxObject *startScope;
    // decode and validate the message input
    decodeMessageName(this, message, messageName, startScope);

    // creeate the new message object and start it.
    Protected<MessageClass> newMessage = new MessageClass(this, messageName, startScope, new_array(argCount, arguments));
    newMessage->start(OREF_NULL);
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
void RexxObject::decodeMessageName(RexxObject *target, RexxObject *message, RexxString *&messageName, RexxObject *&startScope)
{
    // clear the starting scope
    startScope = OREF_NULL;

    // if 1st arg is a string, we can do this quickly
    if (!isOfClass(String, message))
    {
        // this must be an array
        ArrayClass *messageArray = arrayArgument(message, ARG_ONE);

        // must be single dimension with two elements
        if (messageArray->getDimension() != 1 || messageArray->size() != 2)
        {
            reportException(Error_Incorrect_method_message);
        }
        // get the message as a string in uppercase.
        messageName = stringArgument(messageArray->get(1), ARG_ONE)->upper();
        startScope = messageArray->get(2);
        requiredArgument(startScope, ARG_TWO);

        // validate the message creator now
        RexxActivationBase *activation = ActivityManager::currentActivity->getTopStackFrame();
        // have an activation?
        if (activation != OREF_NULL)
        {
            // get the receiving object
            RexxObject *sender = activation->getReceiver();
            if (sender != target)
            {
                reportException(Error_Execution_super);
            }
        }
        else
        {
            reportException(Error_Execution_super);
        }
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
void RexxInternalObject::hasUninit()
{
    memoryObject.addUninitObject((RexxObject *)this);
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
    ProtectedObject<ArrayClass> arglist;
    RexxObject **argumentPtr = NULL;
    size_t argcount = 0;

    // get the method object
    Protected<MethodClass> methobj = (MethodClass *)arguments[0];
    requiredArgument(methobj, ARG_ONE);
    // make sure we have a method object, including creating one from source if necessary
    methobj = MethodClass::newMethodObject(OREF_RUN, (RexxObject *)methobj, (RexxClass *)TheNilObject, IntegerOne);

    // if we have arguments, decode how we are supposed to handle method arguments.
    if (argCount > 1)
    {
        // get the 1st one, its the option
        RexxString *option = (RexxString *)arguments[1];
        // this is now required
        option = stringArgument(option, ARG_TWO);
        /* process the different options     */
        switch (toupper(option->getChar(0)))
        {
            // arguments are in an array.
            case 'A':
                {
                    // we must have just one additional argument if this is an array
                    if (argCount < 3)
                    {
                        missingArgument(ARG_THREE);
                    }
                    if (argCount > 3)
                    {
                        reportException(Error_Incorrect_method_maxarg, IntegerThree);
                    }
                    // get the argument array and make sure we have a good array
                    arglist = arrayArgument(arguments[2], ARG_THREE);
                    // get the array specifics to pass along
                    argumentPtr = arglist->data();
                    argcount = arglist->size();
                    break;
                }

            // individual arguments
            case 'I':
                // point to the array data for the third value onward
                argumentPtr = arguments + 2;
                argcount = argCount - 2;
                break;

            // bad option
            default:
                reportException(Error_Incorrect_method_option, "AI", option);
                break;
        }
    }
    ProtectedObject result;
    // run the method and return the result
    methobj->run(ActivityManager::currentActivity, this, OREF_NONE, argumentPtr, argcount, result);
    return (RexxObject *)result;
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
RexxObject *RexxObject::defineMethods(DirectoryClass *methods)
{
    // use a copy of the behaviour
    setField(behaviour, (RexxBehaviour *)behaviour->copy());
    // loop through the list of methods
    for (HashContents::TableIterator iterator = methods->iterator(); iterator.isAvailable(); iterator.next())
    {
        MethodClass *method = (MethodClass *)iterator.value();
        if (method != TheNilObject)        /* not a removal?                    */
        {
            // need validation here?
            method = method->newScope((RexxClass *)this);
        }
        else
        {
            method = OREF_NULL;       // this is a method removal
        }
        // Get the name for this method, in uppercase
        RexxString *name = (RexxString *)iterator.index();
        name = name->upper();
        behaviour->define(name, method);
    }
    return OREF_NULL;
}


/**
 * Add a method to an object's behaviour.  Used internally
 * during image build.
 *
 * @param msgname The method name.
 * @param methobj The target method object.
 *
 * @return Returns nothing.
 */
RexxObject *RexxObject::defineMethod(RexxString *msgname, MethodClass *methobj)
{
    // get the method name in uppercase.
    msgname = msgname->upper();
    if (methobj != TheNilObject)
    {
        // set a new scope on this of the object's class scope
        methobj = methobj->newScope(classObject());
    }

    /* copy primitive behaviour object and define the method, a copy is made to */
    /* ensure that we don't update the behaviour of any other object, since they*/
    /* may have been sharing the mvd.                                           */
    setField(behaviour, (RexxBehaviour *)behaviour->copy());
    // add this to the behaviour
    behaviour->addMethod(msgname, methobj);
    // adding an UNINIT method to obj?
    if (methobj != TheNilObject && msgname->strCompare(CHAR_UNINIT))
    {
        hasUninit();
    }
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
        RexxObject *val = dictionary->realValue(name);
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
    return ovd->realValue(name);
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
 * Invoke an object's UNINIT method, if it has one.
 */
void RexxObject::uninit()
{
    if (hasMethod(OREF_UNINIT))
    {
        sendMessage(OREF_UNINIT);
    }
}


/**
 * Check to see if an object has an UNINIT method.
 *
 * @return true if the object has an uninit, false otherwise.
 */
bool RexxObject::hasUninitMethod()
{
    return hasMethod(OREF_UNINIT);
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
RexxObject *RexxInternalObject::clone()
{
    // we need an identically sized object
    size_t size = getObjectSize();
    RexxObject *cloneObj = new_object(size);
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


// macros for defining the standard operator methods.  These are
// essentially the same except for the names and the message target
#undef operatorMethod
#define operatorMethod(name, message) RexxObject * RexxObject::name(RexxObject *operand) \
{\
    ProtectedObject result;              /* returned result                   */\
                                         /* do a real message send            */\
    messageSend(OREF_##message, &operand, 1, result);                      \
    if ((RexxObject *)result == OREF_NULL)   /* in an expression and need a result*/ \
    {  \
                                         /* need to raise an exception        */ \
        reportException(Error_No_result_object_message, OREF_##message); \
    }  \
    return (RexxObject *)result;         /* return the final result           */ \
}\


#undef prefixOperatorMethod
#define prefixOperatorMethod(name, message) RexxObject * RexxObject::name(RexxObject *operand) \
{\
    ProtectedObject result;              /* returned result                   */\
                                         /* do a real message send            */\
    messageSend(OREF_##message, &operand, operand == OREF_NULL ? 0 : 1, result); \
    if ((RexxObject *)result == OREF_NULL)             /* in an expression and need a result*/ \
    {  \
                                         /* need to raise an exception        */ \
        reportException(Error_No_result_object_message, OREF_##message); \
    }  \
    return (RexxObject *)result;         /* return the final result           */ \
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
    return copy();
}

/**
 * Exported access to an object virtual function
 *
 * @param message   The message target.
 * @param arguments The message arguments.
 *
 * @return The message result.
 */
RexxObject *RexxObject::unknownRexx(RexxString *message, ArrayClass  *arguments)
{
  return unknown(message, arguments);
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
    message = stringArgument(message, ARG_ONE)->upper();
    return booleanObject(hasMethod(message));
}


/**
 * Create the NIL object instance.
 */
RexxNilObject::RexxNilObject()
{
    // use the initial identify hash and save this.
    hashValue = identityHash();
    // TODO:  Should memory have a special table of proxied objects?  Not
    // really relevant currently, since we don't really use proxies.
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
    RexxObject *C_self = getObjectVariable(OREF_CSELF);
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
    return NULL;                     /* no object available               */
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
void *RexxObject::getCSelf(RexxObject *scope)
{
    while (scope != TheNilObject)
    {
        // try for the variable value
        RexxObject *C_self = getObjectVariable(OREF_CSELF, scope);
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
    return NULL;                     /* no object available               */
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
    // At this point, this will be an instance of object.  After we've removed
    // some of the methods during setup but before the image save, we'll update the
    // behaviour type information so that it will restore with the correct virtual
    // function table pointer.
    RexxObject *newObj = new_object(size, T_Object);
    // we need to switch the virtual method table pointer new.
    newObj->setVirtualFunctions(MemoryObject::virtualFunctionTable[T_NilObject]);
    return newObj;
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

