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
/* Object REXX Kernel                                                         */
/*                                                                            */
/* The main REXX object definitions                                           */
/*                                                                            */
/******************************************************************************/
#include <ctype.h>
#include <string.h>
#include "RexxCore.h"
#include "ObjectClass.hpp"
#include "StringClass.hpp"
#include "BufferClass.hpp"
#include "RexxSmartBuffer.hpp"
#include "DirectoryClass.hpp"
#include "RexxVariableDictionary.hpp"
#include "ArrayClass.hpp"
#include "RexxActivity.hpp"
#include "RexxActivation.hpp"
#include "MessageClass.hpp"
#include "MethodClass.hpp"
#include "ExpressionBaseVariable.hpp"
#include "SourceFile.hpp"
#include "ProtectedObject.hpp"
#include "PointerClass.hpp"


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


void RexxObject::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
    memory_mark(this->objectVariables);
}

void RexxObject::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
    memory_mark_general(this->objectVariables);
}

void RexxObject::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxObject)

    flatten_reference(newThis->objectVariables, envelope);

  cleanUpFlatten
}

RexxObject * RexxInternalObject::makeProxy(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Create a proxy object for a "special" REXX object               */
/******************************************************************************/
{
    if (this == TheNilObject)
    {
        return(RexxObject *)new_proxy("NIL");
    }
    else
    {
        return(RexxObject *)this;
    }
}

bool RexxInternalObject::isEqual(
    RexxObject *other )                /* other object for comparison       */
/******************************************************************************/
/* Function:  primitive level equality method used by the hash collection     */
/*            classes for determining equality.                               */
/******************************************************************************/
{
    return ((RexxObject *)this) == other;/* simple identity equality          */
}

bool RexxObject::isEqual(
    RexxObject *other )                /* other object for comparison       */
/******************************************************************************/
/* Function:  primitive level equality method used by the hash collection     */
/*            classes for determining equality.                               */
/******************************************************************************/
{
    if (this->isBaseClass())               /* not a primitive?                  */
    {
                                           /* simple identity equality          */
        return ((RexxObject *)this) == other;
    }
    else                                 /* return truth value of a compare   */
    {
        ProtectedObject result;
        this->sendMessage(OREF_STRICT_EQUAL, other, result);
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
    if ((RexxObject *)result == OREF_NULL)
    {
        reportException(Error_No_result_object_message, OREF_COMPARETO);
    }
    wholenumber_t comparison;

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
    return isInstanceOf(other) ? TheTrueObject : TheFalseObject;
}


/**
 * Retrieve the method instance for an object's defined method.
 *
 * @param method_name
 *               The method name.
 *
 * @return The method object that implements the object method.
 */
RexxMethod *RexxInternalObject::instanceMethod(RexxString  *method_name)
{
    return OREF_NULL;
}


/**
 * Retrieve the method instance for an object's defined method.
 *
 * @param method_name
 *               The method name.
 *
 * @return The method object that implements the object method.
 */
RexxMethod *RexxObject::instanceMethod(RexxString  *method_name)
{
    // the name must be a string...and we use it in upper case
    method_name = stringArgument(method_name, ARG_ONE)->upper();
    // retrieve the method from the dictionary
    RexxMethod *method_object = (RexxMethod *)this->behaviour->getMethodDictionary()->stringGet(method_name);
    // this is an error if it doesn't exist
    if (method_object == OREF_NULL)
    {
        reportException(Error_No_method_name, this, method_name);
    }
    return method_object;    // got a live one
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
RexxSupplier *RexxInternalObject::instanceMethods(RexxClass *class_object)
{
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
RexxSupplier *RexxObject::instanceMethods(RexxClass *class_object)
{
    // the behaviour handles all of this
    return this->behaviour->getMethods(class_object);
}


/**
 * Retrieve the method instance for an object's defined method.
 *
 * @param method_name
 *               The method name.
 *
 * @return The method object that implements the object method.
 */
RexxMethod *RexxObject::instanceMethodRexx(RexxString  *method_name)
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
RexxSupplier *RexxObject::instanceMethodsRexx(RexxClass *class_object)
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
                                         /* create a string value             */
    return (RexxObject *)new_string((char *)&h, sizeof(HashCode));
}


/**
 * Hash an exported object.  Of we're a non-primitive one, this
 * will require us to send the HASHCODE message to request a
 * hash value.
 *
 * @return A "hashed hash" that can be used by the map collections.
 */
HashCode RexxObject::hash()
{
    // if this is a primitive object, we can just return the primitive hash code.
    if (this->isBaseClass())
    {
        return getHashValue();
    }
    else
    {
        ProtectedObject result;
        // we have some other type of object, so we need to request a hash code
        // by sending the HASHCODE() message.
        this->sendMessage(OREF_HASHCODE, result);
        return ((RexxObject *)result)->stringValue()->getObjectHashCode();
    }
}


RexxObject * RexxObject::strictEqual(
    RexxObject * other)                /* other object for comparison       */
/******************************************************************************/
/* Function:  Process the default "==" strict comparison operator             */
/******************************************************************************/
{
    requiredArgument(other, ARG_ONE);            /* must have the other argument      */
                                       /* this is direct object equality    */
    return (RexxObject *)((this == other)? TheTrueObject: TheFalseObject);
}

RexxObject * RexxObject::equal(RexxObject * other)
/******************************************************************************/
/* Function:  Normal "=" type comparison.  This only returns true if the      */
/*            two objects are the same object                                 */
/******************************************************************************/
{
  requiredArgument(other, ARG_ONE);            /* must have the other argument      */
                                       /* this is direct object equality    */
  return (RexxObject *)((this == other)? TheTrueObject: TheFalseObject);
}

RexxObject *RexxObject::strictNotEqual(RexxObject *other)
/******************************************************************************/
/* Function:  Return the strict inequality of two objects                     */
/******************************************************************************/
{
   requiredArgument(other, ARG_ONE);           /* first argument is required        */
   return this != other ? TheTrueObject : TheFalseObject;
}

RexxObject *RexxObject::notEqual(RexxObject *other)
/******************************************************************************/
/* Function:  Return the inequality of two objects                            */
/******************************************************************************/
{
   requiredArgument(other, ARG_ONE);           /* first argument is required        */
   return this != other ? TheTrueObject : TheFalseObject;
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
    return false;
}


/**
 * Convert an object to a logical value without raising an
 * error.
 *
 * @param result The converted value.
 *
 * @return true if this converted ok, false for an invalid logical.
 */
bool RexxObject::logicalValue(logical_t &result)
{
    return REQUEST_STRING(this)->logicalValue(result);
}


bool RexxInternalObject::truthValue(
    int    errorCode )                 /* error to issue for bad conversion */
/******************************************************************************/
/* Function:  test the truth value of a primitive object                      */
/******************************************************************************/
{
                                       /* report the error                  */
  reportException(errorCode, OREF_NULLSTRING);
  return false;                        /* need a return value               */
}

bool RexxObject::truthValue(
    int    errorCode )                 /* error to issue for bad conversion */
/******************************************************************************/
/* Function:  test the truth value of a primitive object                      */
/******************************************************************************/
{
                                       /* just return string truth value    */
   return REQUEST_STRING(this)->truthValue(errorCode);
}

RexxObject * RexxInternalObject::copy()
/******************************************************************************/
/* Function:  First level primitive copy of an object.  This just copies      */
/*            the object storage, and nothing else.                           */
/******************************************************************************/
{
  /* Instead of calling new_object and memcpy, ask the memory object to make  */
  /* a copy of ourself.  This way, any header information can be correctly    */
  /* initialized by memory.                                                   */
  return (RexxObject *)this->clone();
}

void *RexxInternalObject::operator new(size_t size,
    RexxClass *classObject)            /* class of the object               */
/******************************************************************************/
/* Function:  Create a new primitive object                                   */
/******************************************************************************/
{
    /* get storage for a new object      */
    RexxObject *newObject = (RexxObject *)new_object(size);
    /* use the class instance behaviour  */
    newObject->setBehaviour(classObject->getInstanceBehaviour());
    return(void *)newObject;            /* and return the new object         */
}

void *RexxInternalObject::operator new(size_t size,
    RexxClass * classObject,           /* class of the object               */
    RexxObject **arguments,            /* arguments to the new method       */
    size_t       argCount)             /* the count of arguments            */
/******************************************************************************/
/* Function:  Create a new instance of object (with arguments)                */
/******************************************************************************/
{
    /* Get storage for a new object      */
    RexxObject *newObject = (RexxObject *)new_object(size);
    /* use the classes instance behaviour*/
    newObject->setBehaviour(classObject->getInstanceBehaviour());
    return newObject;                    /* and return the object             */
}

RexxObject * RexxObject::copy()
/******************************************************************************/
/* Function:  Copy an object that has an Object Variable Dictionary (OVD)     */
/******************************************************************************/
{
    /* Instead of calling new_object and memcpy, ask the memory object to make  */
    /* a copy of ourself.  This way, any header information can be correctly    */
    /* initialized by memory.                                                   */
    RexxObject *newObj = (RexxObject *)this->clone();
    /* have object variables?            */
    if (this->objectVariables != OREF_NULL)
    {
        ProtectedObject p(newObj);
        copyObjectVariables(newObj);       /* copy the object variables into the new object */
    }
    /* have instance methods?            */
    if (this->behaviour->getInstanceMethodDictionary() != OREF_NULL)
    {
        /* need to copy the behaviour        */
        newObj->setBehaviour((RexxBehaviour *)newObj->behaviour->copy());
    }
    return newObj;                       /* return the copied version         */
}

void RexxObject::copyObjectVariables(RexxObject *newObj)
/******************************************************************************/
/* Function:  Copy an object's object variable dictionaries into another obj. */
/******************************************************************************/
{
    RexxVariableDictionary *dictionary = objectVariables;
    /* clear out the existing object variable pointer */
    newObj->objectVariables = OREF_NULL;

    while (dictionary != OREF_NULL)
    {
        /* copy the dictionary */
        RexxVariableDictionary *newDictionary = (RexxVariableDictionary *)dictionary->copy();
        /* add this to the variable set */
        newObj->addObjectVariables(newDictionary);
        /* now that the dictionary is anchored in the new object, */
        /* copy the variable objects inside. */
        newDictionary->copyValues();
        /* and repeat for each one */
        dictionary = dictionary->getNextDictionary();
    }
}

RexxMethod * RexxObject::checkPrivate(
    RexxMethod       * method )        /* method to check                   */
/******************************************************************************/
/* Function:  Check a private method for accessibility.                       */
/******************************************************************************/
{
    /* get the top activation            */
    RexxActivationBase *activation = ActivityManager::currentActivity->getTopStackFrame();
    /* have an activation?               */
    if (activation != OREF_NULL)
    {
        RexxObject *sender = activation->getReceiver();/* get the receiving object          */
        if (sender == (RexxObject *)this)  /* the same receiver?                */
        {
            return method;                   /* just return the same method       */
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
    return OREF_NULL;                    /* return a failure indicator        */
}

RexxObject *RexxObject::sendMessage(RexxString *message, RexxArray *args)
{
    ProtectedObject r;
    this->sendMessage(message, args, r);
    return (RexxObject *)r;
}

RexxObject *RexxObject::sendMessage(RexxString *message)
{
    ProtectedObject r;
    this->sendMessage(message, r);
    return (RexxObject *)r;
}

RexxObject *RexxObject::sendMessage(RexxString *message, RexxObject **args, size_t argCount)
{
    ProtectedObject r;
    this->sendMessage(message, args, argCount, r);
    return (RexxObject *)r;
}

RexxObject *RexxObject::sendMessage(RexxString *message, RexxObject *argument1)
{
    ProtectedObject r;
    this->sendMessage(message, argument1, r);
    return (RexxObject *)r;
}


RexxObject *RexxObject::sendMessage(RexxString *message, RexxObject *argument1, RexxObject *argument2)
{
    ProtectedObject r;
    this->sendMessage(message, argument1, argument2, r);
    return (RexxObject *)r;
}


RexxObject *RexxObject::sendMessage(RexxString *message, RexxObject *argument1, RexxObject *argument2, RexxObject *argument3)
{
    ProtectedObject r;
    this->sendMessage(message, argument1, argument2, argument3, r);
    return (RexxObject *)r;
}


RexxObject *RexxObject::sendMessage(RexxString *message, RexxObject *argument1, RexxObject *argument2, RexxObject *argument3, RexxObject *argument4)
{
    ProtectedObject r;
    this->sendMessage(message, argument1, argument2, argument3, argument4, r);
    return (RexxObject *)r;
}


RexxObject *RexxObject::sendMessage(RexxString *message, RexxObject *argument1, RexxObject *argument2, RexxObject *argument3, RexxObject *argument4, RexxObject *argument5)
{
    ProtectedObject r;
    this->sendMessage(message, argument1, argument2, argument3, argument4, argument5, r);
    return (RexxObject *)r;
}


void RexxObject::sendMessage(
    RexxString      *message,          /* name of the message to process    */
    RexxArray  *arguments,             /* array of arguments                */
    ProtectedObject &result)
/******************************************************************************/
/* Function:  Issue a using a set of arguments already in an array item       */
/******************************************************************************/
{
    this->messageSend(message, arguments->data(), arguments->size(), result);
}

void RexxObject::sendMessage(
    RexxString *message,               /* name of the message to process    */
    RexxObject *argument1,             /* first argument                    */
    RexxObject *argument2,             /* second argument                   */
    ProtectedObject &result)
/******************************************************************************/
/* Function:  Send a message with two arguments                               */
/******************************************************************************/
{
  RexxObject *arguments[2];            /* argument array                    */

  arguments[0] = argument1;            /* set each argument                 */
  arguments[1] = argument2;
                                       /* just pass on to message send      */
  this->messageSend(message, arguments, 2, result);
}

void RexxObject::sendMessage(
    RexxString *message,               /* name of the message to process    */
    RexxObject *argument1,             /* first argument                    */
    RexxObject *argument2,             /* second argument                   */
    RexxObject *argument3,             /* third argument                    */
    ProtectedObject &result)
/******************************************************************************/
/* Function:  Send a message with three arguments                             */
/******************************************************************************/
{
  RexxObject *arguments[3];            /* argument array                    */

  arguments[0] = argument1;            /* set each argument                 */
  arguments[1] = argument2;
  arguments[2] = argument3;
                                       /* just pass on to message send      */
  this->messageSend(message, arguments, 3, result);
}

void RexxObject::sendMessage(
    RexxString *message,               /* name of the message to process    */
    RexxObject *argument1,             /* first argument                    */
    RexxObject *argument2,             /* second argument                   */
    RexxObject *argument3,             /* third argument                    */
    RexxObject *argument4,             /* fourth argument                   */
    ProtectedObject &result)
/******************************************************************************/
/* Function:  Send a message with four arguments                              */
/******************************************************************************/
{
  RexxObject *arguments[4];            /* argument array                    */

  arguments[0] = argument1;            /* set each argument                 */
  arguments[1] = argument2;
  arguments[2] = argument3;
  arguments[3] = argument4;
                                       /* just pass on to message send      */
  this->messageSend(message, arguments, 4, result);
}

void RexxObject::sendMessage(
    RexxString *message,               /* name of the message to process    */
    RexxObject *argument1,             /* first argument                    */
    RexxObject *argument2,             /* second argument                   */
    RexxObject *argument3,             /* third argument                    */
    RexxObject *argument4,             /* fourth argument                   */
    RexxObject *argument5,             /* fifth argument                    */
    ProtectedObject &result)
/******************************************************************************/
/* Function:  Send a message with five arguments                              */
/******************************************************************************/
{
  RexxObject *arguments[5];            /* argument array                    */

  arguments[0] = argument1;            /* set each argument                 */
  arguments[1] = argument2;
  arguments[2] = argument3;
  arguments[3] = argument4;
  arguments[4] = argument5;
                                       /* just pass on to message send      */
  this->messageSend(message, arguments, 5, result);
}

void RexxObject::messageSend(
    RexxString      *msgname,          /* name of the message to process    */
    RexxObject     **arguments,        /* array of arguments                */
    size_t           count,            /* count of arguments                */
    ProtectedObject &result)           // returned result
/******************************************************************************/
/* Function:    send a message (with message lookup) to an object.            */
/*              All types of methods are handled and dispatched               */
/******************************************************************************/
{
    ActivityManager::currentActivity->checkStackSpace();       /* have enough stack space?          */
    /* grab the method from this level   */
    RexxMethod *method_save = this->behaviour->methodLookup(msgname);
    /* method exists...special processing*/
    if (method_save != OREF_NULL && method_save->isSpecial())
    {
        if (method_save->isPrivate())      /* actually private method?          */
        {
            /* go validate a private method      */
            method_save = this->checkPrivate(method_save);
        }
        /* now process protected methods     */
        if (method_save != OREF_NULL && method_save->isProtected())
        {
            /* really a protected method         */
            this->processProtectedMethod(msgname, method_save, arguments, count, result);
            return;
        }
    }
    /* have a method                     */
    if (method_save != OREF_NULL)
    {
        method_save->run(ActivityManager::currentActivity, this, msgname, arguments, count, result);
    }
    else
    {
        /* go process an unknown method      */
        this->processUnknown(msgname, arguments, count, result);
    }
}

void RexxObject::messageSend(
    RexxString      *msgname,          /* name of the message to process    */
    RexxObject     **arguments,        /* array of arguments                */
    size_t           count,            /* count of arguments                */
    RexxObject      *startscope,       /* starting superclass scope         */
    ProtectedObject &result)           // returned result
/******************************************************************************/
/* Function:    send a message (with message lookup) to an object.            */
/*              All types of methods are handled and dispatched               */
/******************************************************************************/
{
    ActivityManager::currentActivity->checkStackSpace();       /* have enough stack space?          */
    /* go to the higher level            */
    RexxMethod *method_save = this->superMethod(msgname, startscope);
    if (method_save != OREF_NULL && method_save->isProtected())
    {
        if (method_save->isPrivate())      /* actually private method?          */
        {
            method_save = this->checkPrivate(method_save);
        }
        /* go validate a private method      */
        else                               /* really a protected method         */
        {
            this->processProtectedMethod(msgname, method_save, arguments, count, result);
            return;
        }
    }
    /* have a method                     */
    if (method_save != OREF_NULL)
    {
        /* run the method                    */
        method_save->run(ActivityManager::currentActivity, this, msgname, arguments, count, result);
    }
    else
    {
        /* go process an unknown method      */
        this->processUnknown(msgname, arguments, count, result);
    }
}

void RexxObject::processProtectedMethod(
    RexxString   * messageName,        /* message to issue                  */
    RexxMethod   * targetMethod,       // the method to run
    RexxObject  ** arguments,          /* actual message arguments          */
    size_t         count,              /* count of arguments                */
    ProtectedObject &result)           // returned result
/******************************************************************************/
/* Function:  Process an unknown message, uncluding looking for an UNKNOWN    */
/*            method and raising a NOMETHOD condition                         */
/******************************************************************************/
{
    // get the current security manager
    SecurityManager *manager = ActivityManager::currentActivity->getEffectiveSecurityManager();
    // the security manager can replace provide a new result
    if (manager->checkProtectedMethod(this, messageName, count, arguments, result))
    {
        return;
    }
    /* run the method                    */
    targetMethod->run(ActivityManager::currentActivity, this, messageName, arguments, count, result);
}

void RexxObject::processUnknown(
    RexxString   * messageName,        /* message to issue                  */
    RexxObject  ** arguments,          /* actual message arguments          */
    size_t         count,              /* count of arguments                */
    ProtectedObject &result)           // returned result
/******************************************************************************/
/* Function:  Process an unknown message, uncluding looking for an UNKNOWN    */
/*            method and raising a NOMETHOD condition                         */
/******************************************************************************/
{
    /* no method for this msgname        */
    /* find the unknown method           */
    RexxMethod *method_save = this->behaviour->methodLookup(OREF_UNKNOWN);
    if (method_save == OREF_NULL)        /* "unknown" method exists?          */
    /* no unknown method - try to raise  */
    /* a NOMETHOD condition, and if that */
    {
        reportNomethod(messageName, this); /* fails, it is an error message     */
    }
    RexxArray *argumentArray = new_array(count);    /* get an array for the arguments    */
    ProtectedObject p(argumentArray);

    for (size_t i = 1; i <= count; i++)         /* copy the arguments into an array  */
    {
        argumentArray->put(arguments[i - 1], i);
    }

    RexxObject     *unknown_arguments[2];/* arguments to the unknown method   */
    unknown_arguments[0] = messageName;  /* method name is first argument     */
                                         /* second argument is array of       */
    unknown_arguments[1] = argumentArray;/* arguments for the original call   */
                                         /* run the unknown method            */
    method_save->run(ActivityManager::currentActivity, this, OREF_UNKNOWN, unknown_arguments, 2, result);
}

RexxMethod * RexxObject::methodLookup(
    RexxString *msgname)               /* name of the target message        */
/******************************************************************************/
/* Function:  Return the method object associated with a message name         */
/******************************************************************************/
{
  return this->behaviour->methodLookup(msgname);
}

bool RexxInternalObject::unsignedNumberValue(stringsize_t &result, stringsize_t digits)
/******************************************************************************/
/* Function:  Convert a primitive internal object to a long value             */
/******************************************************************************/
{
  return false;                        /* give a "safe" default here        */
}

bool RexxInternalObject::unsignedNumberValue(stringsize_t &result)
/******************************************************************************/
/* Function:  Convert a primitive internal object to a long value             */
/******************************************************************************/
{
  return false;                        /* give a "safe" default here        */
}

bool RexxInternalObject::numberValue(wholenumber_t &result, stringsize_t digits)
/******************************************************************************/
/* Function:  Convert a primitive internal object to a long value             */
/******************************************************************************/
{
  return false;                        /* give a "safe" default here        */
}

bool RexxInternalObject::numberValue(wholenumber_t &result)
/******************************************************************************/
/* Function:  Convert a primitive internal object to a long value             */
/******************************************************************************/
{
  return false;                        /* give a "safe" default here        */
}

bool RexxInternalObject::doubleValue(double &result)
/******************************************************************************/
/* Function:  Convert a primitive internal object to a double value           */
/******************************************************************************/
{
  return false;                        /* give a "safe" default here        */
}

RexxInteger * RexxInternalObject::integerValue(
    size_t precision)                  /* precision to use                  */
/******************************************************************************/
/* Function:  Convert a primitive internal object to an integer value         */
/******************************************************************************/
{
  return (RexxInteger *)TheNilObject;  /* give a "safe" default here        */
}

RexxNumberString * RexxInternalObject::numberString()
/******************************************************************************/
/* Function:  convert an internal object to a numberstring representation     */
/******************************************************************************/
{
  return OREF_NULL;                    /* this never converts               */
}

bool RexxObject::numberValue(wholenumber_t &result, stringsize_t digits)
/******************************************************************************/
/* Function:  Convert a REXX object to a long value                           */
/******************************************************************************/
{
                                       /* get a string and convert          */
  return REQUEST_STRING(this)->numberValue(result, digits);
}

bool RexxObject::numberValue(wholenumber_t &result)
/******************************************************************************/
/* Function:  Convert a REXX object to a long value                           */
/******************************************************************************/
{
                                       /* get a string and convert          */
  return REQUEST_STRING(this)->numberValue(result);
}

bool RexxObject::unsignedNumberValue(stringsize_t &result, stringsize_t digits)
/******************************************************************************/
/* Function:  Convert a REXX object to a long value                           */
/******************************************************************************/
{
                                       /* get a string and convert          */
  return REQUEST_STRING(this)->unsignedNumberValue(result, digits);
}

bool RexxObject::unsignedNumberValue(stringsize_t &result)
/******************************************************************************/
/* Function:  Convert a REXX object to a long value                           */
/******************************************************************************/
{
                                       /* get a string and convert          */
  return REQUEST_STRING(this)->unsignedNumberValue(result);
}

bool RexxObject::doubleValue(double &result)
/******************************************************************************/
/* Function:  Convert a primitive internal object to a double value           */
/******************************************************************************/
{
                                       /* get a string and convert          */
  return this->requestString()->doubleValue(result);
}

RexxInteger * RexxObject::integerValue(
    size_t precision)                  /* precision to use                  */
/******************************************************************************/
/* Function:  Convert a primitive internal object to an integer value         */
/******************************************************************************/
{
                                       /* get a string and convert          */
  return REQUEST_STRING(this)->integerValue(precision);
}

RexxNumberString * RexxObject::numberString()
/******************************************************************************/
/* Function:  convert a standard object to a numberstring representation      */
/******************************************************************************/
{
                                       /* get the string representation,    */
                                       /* return the numberstring form      */
  return this->requestString()->numberString();
}

RexxString *RexxInternalObject::stringValue()
/******************************************************************************/
/* Function:  Convert a primitive internal object to a string value           */
/******************************************************************************/
{
  return OREF_NULLSTRING;              /* give a "safe" default here        */
}

RexxString *RexxObject::stringValue()
/******************************************************************************/
/* Function:  Convert a primitive object to a string value                    */
/******************************************************************************/
{
                                       /* issue the object name message     */
    return (RexxString *)this->sendMessage(OREF_OBJECTNAME);
}

RexxString *RexxInternalObject::makeString()
/******************************************************************************/
/* Function:  Handle a string conversion REQUEST for an internal object       */
/******************************************************************************/
{
  return (RexxString *)TheNilObject;   /* should never occur                */
}


void RexxInternalObject::copyIntoTail(RexxCompoundTail *tail)
/******************************************************************************/
/* Function:  Handle a tail construction request for an internal object       */
/******************************************************************************/
{
  return;                              /* should never occur                */
}

RexxString *RexxInternalObject::primitiveMakeString()
/******************************************************************************/
/* Function:  Handle a string conversion REQUEST for an internal object       */
/******************************************************************************/
{
  return (RexxString *)TheNilObject;   /* should never occur                */
}

RexxString *RexxObject::makeString()
/******************************************************************************/
/* Function:  Handle a string conversion REQUEST for a REXX object            */
/******************************************************************************/
{
  if (this->isBaseClass())             /* primitive object?                 */
    return (RexxString *)TheNilObject; /* this never converts               */
  else                                 /* process as a string request       */
  {
      return (RexxString *)this->sendMessage(OREF_REQUEST, OREF_STRINGSYM);
  }
}


void RexxObject::copyIntoTail(RexxCompoundTail *tail)
/******************************************************************************/
/* Function:  Handle a tail construction request for an internal object       */
/******************************************************************************/
{
                                       /* get our string value              */
    RexxString *value = REQUEST_STRING(this);
    value->copyIntoTail(tail);         /* pass this on to the string value  */
}


RexxString *RexxObject::primitiveMakeString()
/******************************************************************************/
/* Function:  Handle a string conversion REQUEST for a REXX object            */
/******************************************************************************/
{
  return (RexxString *)TheNilObject;   /* this never converts               */
}

RexxArray *RexxInternalObject::makeArray()
/******************************************************************************/
/* Function:  Handle an array conversion REQUEST for an internal object       */
/******************************************************************************/
{
  return (RexxArray *)TheNilObject;    /* should never occur                */
}

RexxArray *RexxObject::makeArray()
/******************************************************************************/
/* Function:  Handle a string conversion REQUEST for a REXX object            */
/******************************************************************************/
{
  if (this->isBaseClass())             /* primitive object?                 */
    return (RexxArray *)TheNilObject;  /* this never converts               */
  else                                 /* process as a string request       */
  {
      return (RexxArray *)this->sendMessage(OREF_REQUEST, OREF_ARRAYSYM);
  }
}

RexxString *RexxObject::requestString()
/******************************************************************************/
/* Function:  Handle a string request for a REXX object.  This will go        */
/*            through the whole search order to do the conversion.            */
/******************************************************************************/
{

    /* primitive object?                 */
    if (this->isBaseClass())
    {
        RexxString *string_value;            /* converted object                  */
        /* get the string representation     */
        string_value = this->primitiveMakeString();
        if (string_value == TheNilObject)
        {/* didn't convert?                   */
         /* get the final string value        */
            string_value = this->stringValue();
            /* raise a NOSTRING condition        */
            ActivityManager::currentActivity->raiseCondition(OREF_NOSTRING, OREF_NULL, string_value, this, OREF_NULL);
        }
        return string_value;               /* return the converted form         */
    }
    else
    {                               /* do a real request for this        */
        ProtectedObject string_value;

        this->sendMessage(OREF_REQUEST, OREF_STRINGSYM, string_value);
        if (string_value == TheNilObject)
        {/* didn't convert?                   */
         /* get the final string value        */
            this->sendMessage(OREF_STRINGSYM, string_value);
            /* raise a NOSTRING condition        */
            ActivityManager::currentActivity->raiseCondition(OREF_NOSTRING, OREF_NULL, (RexxString *)string_value, this, OREF_NULL);
        }
        return(RexxString *)string_value;   /* return the converted form         */
    }
}

RexxString *RexxObject::requestStringNoNOSTRING()
/******************************************************************************/
/* Function:  Handle a string request for a REXX object.  This will go        */
/*            through the whole search order to do the conversion.            */
/******************************************************************************/
{

    /* primitive object?                 */
    if (this->isBaseClass())
    {
        RexxString *string_value;            /* converted object                  */
        /* get the string representation     */
        string_value = this->primitiveMakeString();
        if (string_value == TheNilObject)
        {/* didn't convert?                   */
         /* get the final string value        */
            string_value = this->stringValue();
        }
        return string_value;
    }
    else
    {                               /* do a real request for this        */
        ProtectedObject string_value;
        this->sendMessage(OREF_REQUEST, OREF_STRINGSYM, string_value);
        if (string_value == TheNilObject)
        {/* didn't convert?                   */
         /* get the final string value        */
            this->sendMessage(OREF_STRINGSYM, string_value);
        }
        return(RexxString *)string_value;  /* return the converted form         */
    }
}


RexxString *RexxObject::requiredString(
    size_t position )                  /* required argument position        */
/******************************************************************************/
/* Function:  Handle a string request for a REXX object in a context where    */
/*            the object MUST have a string value.                            */
/******************************************************************************/
{
    RexxObject *string_value;            /* converted object                  */

    if (this->isBaseClass())             /* primitive object?                 */
    {
        string_value = this->makeString(); /* get the string representation     */
    }
    else                                 /* do a full request for this        */
    {
        string_value = this->sendMessage(OREF_REQUEST, OREF_STRINGSYM);
    }
    /* didn't convert?                   */
    if (string_value == TheNilObject)
    {
        /* this is an error                  */
        reportException(Error_Incorrect_method_nostring, position);
    }
    return(RexxString *)string_value;   /* return the converted form         */
}


RexxString *RexxObject::requiredString(
    const char *name)                  /* required argument position        */
/******************************************************************************/
/* Function:  Handle a string request for a REXX object in a context where    */
/*            the object MUST have a string value.                            */
/******************************************************************************/
{
    RexxObject *string_value;            /* converted object                  */

    if (this->isBaseClass())             /* primitive object?                 */
    {
        string_value = this->makeString(); /* get the string representation     */
    }
    else                                 /* do a full request for this        */
    {
        string_value = this->sendMessage(OREF_REQUEST, OREF_STRINGSYM);
    }
    /* didn't convert?                   */
    if (string_value == TheNilObject)
    {
        /* this is an error                  */
        reportException(Error_Invalid_argument_string, name);
    }
    return(RexxString *)string_value;   /* return the converted form         */
}

/**
 * Handle a string request for a required string value where
 * the caller wishes to handle the error itself.
 *
 * @return The object's string value, or OREF_NULL if this is not a
 *         string.
 */
RexxString *RexxObject::requiredString()
{
    // primitive object?  We have a bypass for this
    if (this->isBaseClass())
    {
        return this->makeString();
    }
    else
    {
        // we have to use REQUEST to get this
        return (RexxString *)this->sendMessage(OREF_REQUEST, OREF_STRINGSYM);
    }
}


RexxInteger *RexxObject::requestInteger(
    size_t precision )                 /* precision to use                  */
/******************************************************************************/
/* Function:  Request an integer value from an object.  If this is not a      */
/*            primitive object, the object will be converted to a string,     */
/*            and then the string integer value will be returned.             */
/******************************************************************************/
{
    if (this->isBaseClass())             /* primitive object?                 */
    {
        /* return the integer value          */
        return this->integerValue(precision);
    }
    else                                 /* return integer value of string    */
    {
        return this->requestString()->integerValue(precision);
    }
}

RexxInteger *RexxObject::requiredInteger(
    size_t position,                   /* precision to use                  */
    size_t precision)                  /* argument position for errors      */
/******************************************************************************/
/* Function:  Request an integer value from an object.  If this is not a      */
/*            primitive object, a REQUEST('STRING') will be performeding,     */
/*            and then the string integer value will be returned.             */
/******************************************************************************/
{
    RexxInteger *result;                 /* returned result                   */

    if (this->isBaseClass())             /* primitive object?                 */
    {
        /* return the integer value          */
        result = this->integerValue(precision);
    }
    else                                 /* return integer value of string    */
    {
        result = this->requiredString(position)->integerValue(precision);
    }
    /* didn't convert well?              */
    if (result == (RexxInteger *)TheNilObject)
    {
        /* raise an error                    */
        reportException(Error_Incorrect_method_whole, position, this);
    }
    return result;                       /* return the new integer            */
}


/**
 * Request an object to convert itself into a number value.
 *
 * @param result    The numeric result value.
 * @param precision The precision used for the conversion.
 *
 * @return true if the object converted ok, false for a conversion failure.
 */
bool RexxObject::requestNumber(wholenumber_t &result, size_t precision)
{
    if (isBaseClass())
    {
        // are we already a base class?
        // the base classes can handle this directly.
        return numberValue(result, precision);
    }
    else
    {
        // we need to perform the operation on the string value
        return numberValue(result, precision);
    }
}


/**
 * Request an object to convert itself into a number value.
 *
 * @param result    The numeric result value.
 * @param precision The precision used for the conversion.
 *
 * @return true if the object converted ok, false for a conversion failure.
 */
bool RexxObject::requestUnsignedNumber(stringsize_t &result, size_t precision)
{
    if (isBaseClass())
    {
        // are we already a base class?
        // the base classes can handle this directly.
        return unsignedNumberValue(result, precision);
    }
    else
    {
        // we need to perform the operation on the string value
        return unsignedNumberValue(result, precision);
    }
}


wholenumber_t RexxObject::requiredNumber(
    size_t position ,                  /* precision to use                  */
    size_t precision)                  /* argument position for errors      */
/******************************************************************************/
/* Function:  Request a long value from an object.  If this is not a          */
/*            primitive object, a REQUEST('STRING') will be performed         */
/*            and then the string long value will be returned.                */
/******************************************************************************/
{
    wholenumber_t  result;               /* returned result                   */

                                         /* primitive object?                 */
    if (this->isBaseClass() && !isOfClass(Object, this))
    {
        if (!numberValue(result, precision))
        {
            /* raise an error                    */
            reportException(Error_Incorrect_method_whole, position, this);
        }
    }
    else                                 /* return integer value of string    */
    {
        if (!requiredString(position)->numberValue(result, precision))
        {
            /* raise an error                    */
            reportException(Error_Incorrect_method_whole, position, this);
        }
    }
    return result;                       /* return the result                 */
}

stringsize_t RexxObject::requiredPositive(
    size_t position,                   /* precision to use                  */
    size_t precision)                  /* argument position for errors      */
/******************************************************************************/
/* Function:  Request a a positive long value from an object.  If this is not */
/*            positive, it will raise an error.                               */
/******************************************************************************/
{
    stringsize_t result;                 /* returned result                   */

    if (!unsignedNumberValue(result, precision) || result == 0)
    {
        /* raise the error                   */
        reportException(Error_Incorrect_method_positive, position, this);
    }
    return result;
}


stringsize_t RexxObject::requiredNonNegative(
    size_t position ,                  /* precision to use                  */
    size_t precision)                  /* argument position for errors      */
/******************************************************************************/
/* Function:  Request a non-negative long value from an object.  If this is   */
/*            less than zero, it will raise an error                          */
/******************************************************************************/
{
    stringsize_t result;                 /* returned result                   */

    if (!unsignedNumberValue(result, precision))
    {
        /* raise the error                   */
        reportException(Error_Incorrect_method_nonnegative, position, this);
    }
    return result;
}


RexxArray *RexxObject::requestArray()
/******************************************************************************/
/* Function:  Request an array value from an object.                          */
/******************************************************************************/
{
  if (this->isBaseClass())             /* primitive object?                 */
  {
    if (isOfClass(Array, this))            /* already an array?                 */
      return (RexxArray *)this;        /* return directly, don't makearray  */
    else
      return this->makeArray();        /* return the array value            */
  }
  else                                 /* return integer value of string    */
  {
      return (RexxArray *)this->sendMessage(OREF_REQUEST, OREF_ARRAYSYM);
  }
}

RexxString *RexxObject::objectName()
/******************************************************************************/
/* Function:  Retrieve the object name for a primitive object                 */
/******************************************************************************/
{
    ProtectedObject string_value;        /* returned string value             */

    RexxObject *scope = lastMethod()->getScope();    /* get the method's scope            */
                                         /* get the object name variable      */
    string_value = (RexxString *)this->getObjectVariable(OREF_NAME, scope);
    if ((RexxObject *)string_value == OREF_NULL)
    {     /* no name?                          */
        if (this->isBaseClass())           /* primitive object?                 */
        {
            /* use fast path to default name     */
            string_value = this->defaultName();
        }
        else                               /* go through the full search        */
        {
            this->sendMessage(OREF_DEFAULTNAME, string_value);
        }
    }
    return(RexxString *)string_value;   /* return the string value           */
}

RexxObject  *RexxObject::objectNameEquals(RexxObject *name)
/******************************************************************************/
/* Function:  retrieve an objects default name value                          */
/******************************************************************************/
{
    RexxObject *scope;                   /* scope of the object               */

    requiredArgument(name, ARG_ONE);             /* must have a name                  */
    scope = lastMethod()->getScope();    /* get the method's scope            */
                                         /* get this as a string              */
    name = (RexxObject *)stringArgument(name, ARG_ONE);
    /* set the name                      */
    this->setObjectVariable(OREF_NAME, name, scope);
    return OREF_NULL;                    /* no return value                   */
}

RexxString  *RexxObject::defaultName()
/******************************************************************************/
/* Function:  Handle "final" string coercion level                            */
/******************************************************************************/
{
    /* use the class id as the default   */
    /* name                              */
    RexxString *defaultname = this->behaviour->getOwningClass()->getId();
    /* check if it is from an enhanced   */
    if (this->behaviour->isEnhanced())
    { /* class                             */
      /* return the 'enhanced' id          */
        return defaultname->concatToCstring("enhanced ");
    }
    switch (defaultname->getChar(0))
    {   /* process based on first character*/
        case 'a':                          /* vowels                          */
        case 'A':
        case 'e':
        case 'E':
        case 'i':
        case 'I':
        case 'o':
        case 'O':
        case 'u':
        case 'U':
            /* prefix with "an"                  */
            defaultname = defaultname->concatToCstring("an ");
            break;

        default:                           /* consonants                        */
            /* prefix with "a"                   */
            defaultname = defaultname->concatToCstring("a ");
            break;
    }
    return defaultname;                  /* return that value                 */
}

RexxInteger *RexxObject::hasMethod(RexxString *msgname)
/******************************************************************************/
/* Function:  Check for the presense of a method on the object                */
/******************************************************************************/
{
                                       /* check the behaviour for the method*/
  return (this->behaviour->methodLookup(msgname) != OREF_NULL) ? TheTrueObject : TheFalseObject;
}

RexxClass   *RexxObject::classObject()
/******************************************************************************/
/* Function:  Return the class object associated with an object               */
/******************************************************************************/
{
                                       /* just return class from behaviour  */
  return this->behaviour->getOwningClass();
}

RexxObject  *RexxObject::setMethod(
    RexxString *msgname,               /* name of the new method            */
    RexxMethod *methobj,               /* associated method object/code     */
    RexxString *option)
/******************************************************************************/
/* Function:  Add a new method to an object instance                          */
/******************************************************************************/
{
    /* get the message name as a string  */
    msgname = stringArgument(msgname, ARG_ONE)->upper();
    if (option)
    {
        option = stringArgument(option, ARG_THREE);
        if (!Utilities::strCaselessCompare("OBJECT",option->getStringData()))
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

    if (methobj == OREF_NULL)            /* we weren't passed a method,       */
    {
        /* add a dummy method                */
        methobj = (RexxMethod *)TheNilObject;
    }
    else if (!isOfClass(Method, methobj))    /* not a method type already?        */
    {
        /* make one from a string or array   */
        methobj = RexxMethod::newMethodObject(msgname, (RexxObject *)methobj, IntegerTwo, OREF_NULL);
    }
    this->defMethod(msgname, methobj, option);   /* defMethod handles all the details */
    return OREF_NULL;                    /* no return value                   */
}

RexxObject  *RexxObject::unsetMethod(
    RexxString *msgname)               /* target message name               */
/******************************************************************************/
/* Function:  Remove a method from an object instance                         */
/******************************************************************************/
{
                                       /* get the message name as a string  */
  msgname = stringArgument(msgname, ARG_ONE)->upper();
                                       /* now just go remove this           */
  this->behaviour->removeMethod(msgname);
  return OREF_NULL;                    /* no return value                   */
}

RexxObject  *RexxObject::requestRexx(
    RexxString *className)             /* target name of the class          */
/******************************************************************************/
/* Function:  Externalized version of the REQUEST method.  This tries to      */
/*            convert one class of object into another.                       */
/******************************************************************************/
{
                                         /* Verify we have a string parm      */
    className = stringArgument(className, ARG_ONE)->upper();
    RexxString *class_id = this->id()->upper();      /* get the class name in uppercase   */
                                         /* of the same class?                */
    if (className->strictEqual(class_id) == TheTrueObject)
    {
        return this;                     /* already converted                 */
    }
    /* Get "MAKE"||class methodname      */
    RexxString *make_method = className->concatToCstring(CHAR_MAKE);
    /* find the MAKExxxx method          */
    RexxMethod *method = this->behaviour->methodLookup(make_method);
    /* have this method?                 */
    if (method != OREF_NULL)
    {
        /* Return its results                */
        return this->sendMessage(make_method);
    }
    else                                 /* No makeclass method               */
    {
        return TheNilObject;               /* Let user handle it                */
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
RexxObject *RexxObject::sendWith(RexxObject *message, RexxArray *arguments)
{
    RexxString *messageName;
    RexxObject *startScope;
    // decode and validate the message input
    decodeMessageName(this, message, messageName, startScope);
    arguments = arrayArgument(arguments, ARG_TWO);

    ProtectedObject r;
    if (startScope == OREF_NULL)
    {
        this->messageSend(messageName, arguments->data(), arguments->size(), r);
    }
    else
    {
        this->messageSend(messageName, arguments->data(), arguments->size(), startScope, r);
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
        this->messageSend(messageName, arguments + 1, argCount - 1, r);
    }
    else
    {
        this->messageSend(messageName, arguments + 1, argCount - 1, startScope, r);
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
RexxMessage *RexxObject::startWith(RexxObject *message, RexxArray *arguments)
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
RexxMessage *RexxObject::start(RexxObject **arguments, size_t argCount)
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
RexxMessage *RexxObject::startCommon(RexxObject *message, RexxObject **arguments, size_t argCount)
{
    RexxString *messageName;
    RexxObject *startScope;
    // decode and validate the message input
    decodeMessageName(this, message, messageName, startScope);

    /* Create the new message object.    */
    RexxMessage *newMessage = new RexxMessage(this, messageName, startScope, new (argCount, arguments) RexxArray);
    ProtectedObject p(newMessage);
    newMessage->start(OREF_NULL);        /* Tell the message object to start  */
    return newMessage;                   /* return the new message object     */
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

    /* if 1st arg is a string, we can do */
    /* this quickly                      */
    if (!isOfClass(String, message))
    {
        // this must be an array
        RexxArray *messageArray = arrayArgument(message, ARG_ONE);

        // must be single dimension with two arguments
        if (messageArray->getDimension() != 1 || messageArray->size() != 2)
        {
            /* raise an error                    */
            reportException(Error_Incorrect_method_message);
        }
        // get the message as a string in uppercase.
        messageName = stringArgument(messageArray->get(1), ARG_ONE)->upper();
        startScope = messageArray->get(2);
        requiredArgument(startScope, ARG_TWO);

        // validate the message creator now
        RexxActivationBase *activation = ActivityManager::currentActivity->getTopStackFrame();
        /* have an activation?               */
        if (activation != OREF_NULL)
        {
            /* get the receiving object          */
            RexxObject *sender = activation->getReceiver();
            if (sender != target)            /* not the same receiver?            */
            {
                /* this is an error                  */
                reportException(Error_Execution_super);
            }
        }
        else
        {
            /* this is an error                  */
            reportException(Error_Execution_super);
        }
    }
    else                                 /* not an array as message.          */
    {
        /* force to a string value           */
        messageName = stringArgument(message, ARG_ONE)->upper();
    }
}


RexxString  *RexxObject::oref()
/****************************************************************************/
/* Function:  Return the objects address as a HEX string (debugging only)   */
/****************************************************************************/
{
    char buffer[20];                     /* buffered address                  */

    sprintf(buffer, "%p", this);         /* format this                       */
                                         /* and return a string               */
    return(RexxString *)new_string(buffer,8);
}

void RexxInternalObject::hasUninit()
/****************************************************************************/
/* Function:  Tag an object as having an UNINIT method                      */
/****************************************************************************/
{
                                       /* tell the activity about this      */
   memoryObject.addUninitObject((RexxObject *)this);
}


RexxObject  *RexxObject::run(
    RexxObject **arguments,            /* method arguments                  */
    size_t       argCount)             /* the number of arguments           */
/****************************************************************************/
/* Function:  Run a method on an object as if it was part of the object's   */
/*            behaviour.                                                    */
/****************************************************************************/
{
    RexxArray  *arglist = OREF_NULL;     /* forwarded option string           */
    RexxObject **argumentPtr = NULL;     /* default to no arguments passed along */
    size_t argcount = 0;

    /* get the method object             */
    RexxMethod *methobj = (RexxMethod *)arguments[0];
    requiredArgument(methobj, ARG_ONE);          /* make sure we have a method        */
    if (!isOfClass(Method, methobj))         /* this a method object?             */
    {
        /* create a method object            */
        methobj = RexxMethod::newMethodObject(OREF_RUN, (RexxObject *)methobj, IntegerOne, OREF_NULL);
        /* set the correct scope             */
        methobj->setScope((RexxClass *)TheNilObject);
    }
    else
    {
        /* ensure correct scope on method    */
        methobj = methobj->newScope((RexxClass *)TheNilObject);
    }
    // we need to save this, since we might be working off of a newly created
    // one or a copy
    ProtectedObject p(methobj);

    if (argCount > 1)                    /* if any arguments passed           */
    {
        /* get the 1st one, its the option   */
        RexxString *option = (RexxString *)arguments[1];
        /* this is now required              */
        option = stringArgument(option, ARG_TWO);
        /* process the different options     */
        switch (toupper(option->getChar(0)))
        {
            case 'A':                        /* args are an array                 */
                {
                    /* so they say, make sure we have an */
                    /* array and we were only passed 3   */
                    /*args                               */
                    if (argCount < 3)              /* not enough arguments?             */
                    {
                        missingArgument(ARG_THREE); /* this is an error                  */
                    }
                    if (argCount > 3)              /* too many arguments?               */
                    {
                        reportException(Error_Incorrect_method_maxarg, IntegerThree);
                    }
                    /* now get the array                 */
                    arglist = (RexxArray *)arguments[2];
                    /* force to array form               */
                    arglist = REQUEST_ARRAY(arglist);
                    /* not an array?                     */
                    if (arglist == TheNilObject || arglist->getDimension() != 1)
                    {
                        /* raise an error                    */
                        reportException(Error_Incorrect_method_noarray, arguments[2]);
                    }
                    // request array may create a new one...keep it safe
                    ProtectedObject p1(arglist);
                    /* grab the argument information */
                    argumentPtr = arglist->data();
                    argcount = arglist->size();
                    break;
                }

            case 'I':                        /* args are "strung out"             */
                /* point to the array data for the second value */
                argumentPtr = arguments + 2;
                argcount = argCount - 2;
                break;

            default:
                /* invalid option                    */
                reportException(Error_Incorrect_method_option, "AI", option);
                break;
        }
    }
    ProtectedObject result;
    /* now just run the method....       */
    methobj->run(ActivityManager::currentActivity, this, OREF_NONE, argumentPtr, argcount, result);
    return (RexxObject *)result;
}

RexxObject  *RexxObject::defMethods(
    RexxDirectory *methods)            /* new table of methods              */
/****************************************************************************/
/* Function:  Add a table of methods to an object's behaviour               */
/****************************************************************************/
{
    /* make a copy of the behaviour      */
    OrefSet(this, this->behaviour, (RexxBehaviour *)this->behaviour->copy());
    /* loop through the list of methods  */
    for (HashLink i = methods->first(); methods->available(i); i = methods->next(i))
    {
        /* Get the methjod Object            */
        RexxMethod *method = (RexxMethod *)methods->value(i);
        if (method != TheNilObject)        /* not a removal?                    */
        {
            /* set a new scope on this           */
            method = method->newScope((RexxClass *)this);
        }
        else
        {
            method = OREF_NULL;       // this is a method removal
        }
        /* Get the name for this method      */
        RexxString *name = (RexxString *)methods->index(i);
        name = name->upper();              /* make sure the name is upperCase.  */
                                           /* add this method to the object's   */
                                           /* behaviour                         */
        this->behaviour->define(name, method);
    }
    return OREF_NULL;
}

RexxObject  *RexxObject::defMethod(
    RexxString *msgname,               /* new method name                   */
    RexxMethod *methobj,               /* associated method object          */
    RexxString *option)
/****************************************************************************/
/* Function:  Add a method to an object's behaviour                         */
/****************************************************************************/
{
    RexxMethod *methcopy;                /* copy of the original method       */
                                         /* default scope "FLOAT"             */
    RexxClass  *targetClass = (RexxClass*) TheNilObject;

    msgname = msgname->upper();          /* add this as an uppercase name     */
    if (methobj != TheNilObject)         /* not a removal?                    */
    {
        /* got an option? */
        if (option)
        {
            if (!Utilities::strCaselessCompare("OBJECT",option->getStringData()))
            {
                targetClass = this->behaviour->getOwningClass();
            }
            else
            {
                reportException(Error_Incorrect_call_list, CHAR_SETMETHOD, IntegerThree, "\"FLOAT\", \"OBJECT\"", option);
            }
        }
        /* set a new scope on this           */
        methcopy = methobj->newScope(targetClass);
    }
    else
    {
        /* no real method added              */
        methcopy = (RexxMethod *)TheNilObject;
    }
    /* is this the first added method?   */
    if (this->behaviour->getInstanceMethodDictionary() == OREF_NULL)
    {

/* copy primitive behaviour object and define the method, a copy is made to */
/* ensure that we don't update the behaviour of any other object, since they*/
/* may have been sharing the mvd.                                           */
        OrefSet(this, this->behaviour, (RexxBehaviour *)this->behaviour->copy());
    }
    /* now add this to the behaviour     */
    this->behaviour->addMethod(msgname, methcopy);
    /* adding an UNINIT method to obj?   */
    if (methobj != TheNilObject && msgname->strCompare(CHAR_UNINIT))
    {
        this->hasUninit();                 /* yes, mark it as such              */
    }
    return OREF_NULL;
}

size_t RexxInternalObject::getObjectTypeNumber()
/******************************************************************************/
/* Function:  Return the object's primitive type number                       */
/******************************************************************************/
{
  return this->behaviour->getClassType();
}

void RexxInternalObject::removedUninit()
/******************************************************************************/
/* Function:  Remove an UNINIT method from an object                          */
/******************************************************************************/
{
    memoryObject.removeUninitObject((RexxObject *)this);
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
    RexxVariableDictionary *dictionary = objectVariables;
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


RexxObject * RexxObject::getObjectVariable(
  RexxString * name,                   /* variable name (name object)       */
  RexxObject * scope)                  /* target variable scope             */
/******************************************************************************/
/* Function:   retrieve the value of an object variable.  This name           */
/*             must be a name object, and only simple variables are supported.*/
/*             If the variable has not been assigned a value, then OREF_NULL  */
/*             is returned.                                                   */
/******************************************************************************/
{
    if (OREF_NULL == scope)              /* were we passed a scope for lookup */
    {
        scope = this;                      /* no, we use our own.               */
    }
                                           /* get the ovd for our scope level   */
    RexxVariableDictionary *ovd = this->getObjectVariables(scope);
    return ovd->realValue(name);         /* get the real variable value       */
}

void RexxObject::setObjectVariable(
  RexxString *name,                    /* variable name (name object)     */
  RexxObject *value,                   /* new variable value              */
  RexxObject *scope)                   /* target variable scope           */
/******************************************************************************/
/* Function:   assign a new value to a object variable.  This name            */
/*             must be a name object, and only simple variables are supported.*/
/******************************************************************************/
{
    if (OREF_NULL == scope)              /* were we passed a scope for lookup */
    {
        scope = this;                      /* no, we use our own.               */
    }
    /* get the ovd for our scope level   */
    RexxVariableDictionary *ovd = this->getObjectVariables(scope);
    ovd->set(name, value);               /* do the variable assignment      */
}

void RexxObject::addObjectVariables(
    RexxVariableDictionary *dictionary)/* new variable set                  */
/******************************************************************************/
/* Function:  Add a new variable dictionary to an object with a given scope   */
/******************************************************************************/
{
    /* chain any existing dictionaries off of the new one */
    dictionary->setNextDictionary(objectVariables);
    /* make this the new head of the chain */
    OrefSet(this, objectVariables, dictionary);
}

RexxObject * RexxObject::superScope(
  RexxObject *startScope)              /* target scope                    */
/******************************************************************************/
/* Function:  Find the scope of a method's super class                        */
/******************************************************************************/
{
  return this->behaviour->superScope(startScope);
}

RexxMethod * RexxObject::superMethod(
  RexxString *msgName,                 /* target message name             */
  RexxObject *startScope)              /* starting lookup scope           */
/******************************************************************************/
/* Function:   Find a method using the given starting scope information       */
/******************************************************************************/
{
  return this->behaviour->superMethod(msgName, startScope);
}

RexxVariableDictionary * RexxObject::getObjectVariables(
    RexxObject *scope)                 /* required dictionary scope         */
/******************************************************************************/
/* Function:   Retrieve an object dictionary for a given scope                */
/******************************************************************************/
{
    RexxVariableDictionary *dictionary = objectVariables;        /* get the head of the chain         */
    while (dictionary != OREF_NULL)
    {    /* search for a scope match          */
        /* if we've found a match, return it */
        if (dictionary->isScope(scope))
        {
            return dictionary;
        }
        dictionary = dictionary->getNextDictionary();
    }

    /* just create a new vdict           */
    dictionary = new_objectVariableDictionary(scope);
    /* chain any existing dictionaries off of the new one */
    dictionary->setNextDictionary(objectVariables);
    /* make this the new head of the chain */
    OrefSet(this, objectVariables, dictionary);
    this->setHasReferences();            /* we now have references            */
    return dictionary;                   /* return the correct ovd            */
}

const char *RexxObject::idString(void)
/******************************************************************************/
/* Function:  Return a pointer to the object's string value                   */
/******************************************************************************/
{
    RexxString *classId = this->id();                /* get the id string                 */
    if (classId == OREF_NULL)            /* internal class?                   */
    {
        return "unknown Class";            /* return an unknown identifier      */
    }
    else
    {
        return classId->getStringData();        /* return the actual class ID        */
    }
}

RexxString *RexxObject::id(void)
/******************************************************************************/
/* Function:  Get the class string name                                       */
/******************************************************************************/
{
    /* get the class                     */
    RexxClass *createClass = this->behaviourObject()->getOwningClass();
    if (createClass == OREF_NULL)        /* no class object?                  */
    {
        return OREF_NULL;                  /* return nothing                    */
    }
    else
    {
        return createClass->getId();       /* return the class id string        */
    }
}

RexxObject *RexxObject::init(void)
/******************************************************************************/
/* Function:  Exported Object INIT method                                     */
/******************************************************************************/
{
  return OREF_NULL;                    /* this is basically a no-op         */
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
    return new_integer(this->identityHash());
}


void RexxObject::uninit(void)
/******************************************************************************/
/* Function:  Exported Object INIT method                                     */
/******************************************************************************/
{
  if (TheTrueObject == this->hasMethod(OREF_UNINIT))
  {
      this->sendMessage(OREF_UNINIT);
  }

}

bool RexxObject::hasUninitMethod()
/******************************************************************************/
/* Function:  Check to see if an object has an UNINIT method.                 */
/******************************************************************************/
{
  return TheTrueObject == this->hasMethod(OREF_UNINIT);
}

RexxObject *RexxObject::newRexx(RexxObject **arguments, size_t argCount)
/******************************************************************************/
/* Function:  Exposed REXX NEW method                                         */
/******************************************************************************/
{
  return new ((RexxClass *)this, arguments, argCount) RexxObject;
}


RexxObject *RexxInternalObject::clone()
/******************************************************************************/
/* Arguments:  Clone an object, and set up its header.  This method should    */
/*             be called by other _copy methods instead of using new_object   */
/*             and memcpy, so that memory can properly initialize the new     */
/*             object's header to avoid early gc.                             */
/*                                                                            */
/*  Returned:  A new object copied from objr, but set to be live to avoid     */
/*             being garbage collected on a pending sweep.                    */
/******************************************************************************/
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
    return cloneObj;
}

#undef operatorMethod
#define operatorMethod(name, message) RexxObject * RexxObject::name(RexxObject *operand) \
{\
    ProtectedObject result;              /* returned result                   */\
                                         /* do a real message send            */\
    this->messageSend(OREF_##message, &operand, 1, result);                      \
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
    this->messageSend(OREF_##message, &operand, operand == OREF_NULL ? 0 : 1, result); \
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

void *RexxObject::operator new(size_t size, RexxClass *classObject)
/******************************************************************************/
/* Function:  Create a new translator object                                  */
/******************************************************************************/
{
    /* get storage for new object        */
    RexxObject *newObject = (RexxObject *)new_object(size);
    // the virtual function table is still object, but the behaviour is whatever
    // the class object defines.
    newObject->setBehaviour(classObject->getInstanceBehaviour());
    // the hash value and nulled object table was handled by new_object();

    if (classObject->hasUninitDefined() || classObject->parentHasUninitDefined())
    {  /* or parent has one */
        newObject->hasUninit();
    }

    return(void *)newObject;            /* Initialize the new object         */
}


void *RexxObject::operator new(size_t size, RexxClass *classObject, RexxObject **args, size_t argCount)
/******************************************************************************/
/* Function:  Create a new instance of object                                  */
/******************************************************************************/
{
    /* create a new object               */
    ProtectedObject newObject = new (classObject) RexxObject;
    /* now drive the user INIT           */
    ((RexxObject *)newObject)->sendMessage(OREF_INIT, args, argCount);
    return(RexxObject *)newObject;      /* and returnthe new object          */
}


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
    RexxString *alias = (RexxString *)REQUEST_STRING(this);
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
    RexxString *alias = (RexxString *)REQUEST_STRING(this);
    return alias->concatBlank(otherObj);
}


RexxString *RexxObject::stringRexx()
/******************************************************************************/
/* Function:  Exported access to an object virtual function                   */
/******************************************************************************/
{
  return this->stringValue();          /* forward to the virtual function   */
}

RexxObject *RexxObject::makeStringRexx()
/******************************************************************************/
/* Function:  Exported access to an object virtual function                   */
/******************************************************************************/
{
  return this->makeString();           /* forward to the virtual function   */
}

RexxObject *RexxObject::makeArrayRexx()
/******************************************************************************/
/* Function:  Exported access to an object virtual function                   */
/******************************************************************************/
{
  return this->makeArray();            /* forward to the virtual function   */
}

RexxString *RexxObject::defaultNameRexx()
/******************************************************************************/
/* Function:  Exported access to an object virtual function                   */
/******************************************************************************/
{
  return this->defaultName();          /* forward to the virtual function   */
}

RexxObject *RexxObject::copyRexx()
/******************************************************************************/
/* Function:  Exported access to an object virtual function                   */
/******************************************************************************/
{
  return this->copy();                 /* forward to the virtual function   */
}

RexxObject *RexxObject::unknownRexx(
    RexxString *message,               /* unknown message                   */
    RexxArray  *arguments )            /* message arguments                 */
/******************************************************************************/
/* Function:  Exported access to an object virtual function                   */
/******************************************************************************/
{
                                       /* forward to the virtual function   */
  return this->unknown(message, arguments);
}

RexxObject *RexxObject::hasMethodRexx(
    RexxString *message )              /* method name                       */
/******************************************************************************/
/* Function:  Exported access to an object virtual function                   */
/******************************************************************************/
{
  message = stringArgument(message, ARG_ONE)->upper();
  return this->hasMethod(message);     /* forward to the virtual function   */
}

void RexxInternalObject::printObject()
/******************************************************************************/
/* Function:  give a formatted print of object information.                   */
/******************************************************************************/
{
    printf("Object at %p, of type %d\n", this, this->getObjectTypeNumber());
}

/**
 * Create the NIL object instance.
 */
RexxNilObject::RexxNilObject()
{
    // use the initial identify hash and save this.
    hashValue = identityHash();
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
            return ((RexxPointer *)C_self)->pointer();
        }
        // this could be a containing buffer instance as well
        else if (C_self->isInstanceOf(TheBufferClass))
        {
            // return a pointer to the buffer beginning
            return(void *)((RexxBuffer *)C_self)->getData();
        }
    }
    return NULL;                     /* no object available               */
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
    newObj->setVirtualFunctions(RexxMemory::virtualFunctionTable[T_NilObject]);
    return newObj;
}


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

