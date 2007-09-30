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
/* Object REXX Kernel                                           ObjectClass.c    */
/*                                                                            */
/* The main REXX object definitions                                           */
/*                                                                            */
/******************************************************************************/
#include <ctype.h>
#include <string.h>
#include "RexxCore.h"
#include "ObjectClass.hpp"
#include "StringClass.hpp"
#include "RexxBuffer.hpp"
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

static RexxString *msgname_save;       /* last issued message               */
static RexxMethod *method_save;        /* last issued method object         */

RexxString *last_msgname (void)
/******************************************************************************/
/* Function:  Return name of last message sent via messageSend()              */
/******************************************************************************/
{
  return msgname_save;
}

RexxMethod *last_method  (void)
/******************************************************************************/
/* Function:  Return last invoked method object (for use by kernel methods    */
/*            only)                                                           */
/******************************************************************************/
{
  return method_save;
}

void RexxObject::live()
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  setUpMemoryMark
  memory_mark(this->objectVariables);
  cleanUpMemoryMark
}

void RexxObject::liveGeneral()
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  setUpMemoryMarkGeneral
  memory_mark_general(this->objectVariables);
  cleanUpMemoryMarkGeneral
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
      return (RexxObject *)new_proxy("NIL");
    else
      return (RexxObject *)this;
}

BOOL RexxInternalObject::isEqual(
    RexxObject *other )                /* other object for comparison       */
/******************************************************************************/
/* Function:  primitive level equality method used by the hash collection     */
/*            classes for determining equality.                               */
/******************************************************************************/
{
  return ((RexxObject *)this) == other;/* simple identity equality          */
}

BOOL RexxObject::isEqual(
    RexxObject *other )                /* other object for comparison       */
/******************************************************************************/
/* Function:  primitive level equality method used by the hash collection     */
/*            classes for determining equality.                               */
/******************************************************************************/
{
  if (isPrimitive(this))               /* primitive object?                 */
                                       /* simple identity equality          */
    return ((RexxObject *)this) == other;
  else                                 /* return truth value of a compare   */
    return this->sendMessage(OREF_STRICT_EQUAL, other)->truthValue(Error_Logical_value_method);
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
    RexxObject *result = sendMessage(OREF_COMPARETO, other);
    if (result == OREF_NULL)
    {
        reportException(Error_No_result_object_message, OREF_COMPARETO);
    }
    wholenumber_t comparison = result->longValue(DEFAULT_DIGITS);
    if (comparison == NO_LONG)
    {
        reportException(Error_Invalid_whole_number_compareto, result);
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
    required_arg(other, ONE);
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
    method_name = REQUIRED_STRING(method_name, ARG_ONE)->upper();
    // retrieve the method from the dictionary
    RexxMethod *method_object = (RexxMethod *)this->behaviour->getMethodDictionary()->stringGet(method_name);
    // this is an error if it doesn't exist
    if (method_object == OREF_NULL)
    {
        report_exception2(Error_No_method_name, this, method_name);
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
    unsigned long hash = HASHVALUE(this);
                                         /* create a string value             */
    return (RexxObject *)new_string((char *)&hash, sizeof(long));
}


/**
 * Hash an exported object.  Of we're a non-primitive one, this
 * will require us to send the HASHCODE message to request a
 * hash value.
 *
 * @return A "hashed hash" that can be used by the map collections.
 */
ULONG RexxObject::hash()
{
    // if this is a primitive object, we can just return the stored hash code.
    if (isPrimitive(this))
    {
        return HASHVALUE(this);
    }
    else
    {
        ULONG hash;

        // we have some other type of object, so we need to request a hash code
        // by sending the HASHCODE() message.
        RexxString *hashString = this->sendMessage(OREF_HASHCODE)->stringValue();

        // ok, we need to pick this string apart and turn this into a numeric code
        // a null string is simple.
        if (hashString->length == 0)
        {
            hash = 1;
        }

        // if we have at least 4 characters, use them as binary, since that's
        // what is normally returned here.
        else if (hashString->length >= sizeof(LONG))
        {
            hash = *((PULONG)hashString->stringData);
        }

        else
        {
            // either 1 or 2 characters.  Just pick up a short value, which will
            // also pick up terminating null if only a single character
            hash = *((PSHORT)hashString->stringData);
        }
        return hash;
  }
}


RexxObject * RexxObject::strictEqual(
    RexxObject * other)                /* other object for comparison       */
/******************************************************************************/
/* Function:  Process the default "==" strict comparison operator             */
/******************************************************************************/
{
                                       /* this is direct object equality    */
    return (RexxObject *)((this == other)? TheTrueObject: TheFalseObject);
}

RexxObject * RexxObject::equal(RexxObject * other)
/******************************************************************************/
/* Function:  Normal "=" type comparison.  This only returns true if the      */
/*            two objects are the same object                                 */
/******************************************************************************/
{
  required_arg(other, ONE);            /* must have the other argument      */
                                       /* this is direct object equality    */
  return (RexxObject *)((this == other)? TheTrueObject: TheFalseObject);
}

RexxInteger *RexxObject::strictNotEqual(RexxObject *other)
/******************************************************************************/
/* Function:  Return the strict inequality of two objects                     */
/******************************************************************************/
{
   required_arg(other, ONE);           /* first argument is required        */
   return this != other ? TheTrueObject : TheFalseObject;
}

RexxInteger *RexxObject::notEqual(RexxObject *other)
/******************************************************************************/
/* Function:  Return the inequality of two objects                            */
/******************************************************************************/
{
   required_arg(other, ONE);           /* first argument is required        */
   return this != other ? TheTrueObject : TheFalseObject;
}

BOOL RexxInternalObject::truthValue(
    LONG   errorCode )                 /* error to issue for bad conversion */
/******************************************************************************/
/* Function:  test the truth value of a primitive object                      */
/******************************************************************************/
{
                                       /* report the error                  */
  report_exception1(errorCode, OREF_NULLSTRING);
  return FALSE;                        /* need a return value               */
}

BOOL RexxObject::truthValue(
    LONG   errorCode )                 /* error to issue for bad conversion */
/******************************************************************************/
/* Function:  test the truth value of a primitive object                      */
/******************************************************************************/
{
                                       /* just return string truth value    */
   return REQUEST_STRING(this)->truthValue(errorCode);
}

// this talks about "duplicate logic" but it seems to never be called!
// besides, if they really wanted this, why recalculate the hash, it's
// in the string already:
//   ...
//   else {
//     hashString = this->sendMessage(OREF_STRICT_EQUAL)->stringValue();
//     hash = hashString->hashvalue;
//   }
//   return hash;
//
// would have done the same! voila, just one place for hash calculation?!!
#if 0
ULONG RexxObject::hash()
/******************************************************************************/
/* Function:  retrieve the hash value of a primitive object                   */
/******************************************************************************/
{
  RexxString *hashString;              /* string value of the hash          */
  ULONG       hash;                    /* returned hash value               */

  if (isPrimitive(this))               /* is this a primitve class?         */
    return HASHVALUE(this);            /* use the macro to return the hash  */
  else {
                                       /* get a hash value (as a string)    */
    hashString = this->sendMessage(OREF_STRICT_EQUAL)->stringValue();
                                       /* the following logic is duplicated */
                                       /* in string_class_new function.  Any*/
                                       /* changes to the hashing must be    */
                                       /* reflected in both locations       */
    if (hashString->length == 0)       /* nullstring?                       */
      hash = 1;                        /* use 1 for the hash value          */
                                       /* got a long string?                */
    else if (hashString->length >= sizeof(LONG))
                                       /* just pick up the first 4 bytes    */
      hash = *((PULONG)hashString->stringData);
    else
                                       /* just pick up the first 2 bytes    */
                                       /* (this may pick up the trailing    */
                                       /* NULL if only one character long   */
      hash = *((PSHORT)hashString->stringData);
    return hash;                       /* return the hash value             */
  }
}
#endif

RexxObject * RexxInternalObject::copy()
/******************************************************************************/
/* Function:  First level primitive copy of an object.  This just copies      */
/*            the object storage, and nothing else.                           */
/******************************************************************************/
{
  /* Instead of calling new_object and memcpy, ask the memory object to make  */
  /* a copy of ourself.  This way, any header information can be correctly    */
  /* initialized by memory.                                                   */
  return (RexxObject *)memoryObject.clone((RexxObject *)this);
}

void *RexxInternalObject::operator new(size_t size,
    RexxClass *classObject)            /* class of the object               */
/******************************************************************************/
/* Function:  Create a new primitive object                                   */
/******************************************************************************/
{
  RexxObject *newObject;               /* new object                        */

                                       /* get storage for a new object      */
  newObject = (RexxObject *)new_object(size);
                                       /* use the class instance behaviour  */
  BehaviourSet(newObject, classObject->instanceBehaviour);
                                       /* use the default hash value        */
  newObject->hashvalue = HASHOREF(newObject);
  return (void *)newObject;            /* and return the new object         */
}

void *RexxInternalObject::operator new(size_t size,
    RexxClass * classObject,           /* class of the object               */
    RexxObject **arguments,            /* arguments to the new method       */
    size_t       argCount)             /* the count of arguments            */
/******************************************************************************/
/* Function:  Create a new instance of object (with arguments)                */
/******************************************************************************/
{
  RexxObject *newObject;               /* newly create object               */

                                       /* Get storage for a new object      */
  newObject = (RexxObject *)new_object(size);
                                       /* use the classes instance behaviour*/
  BehaviourSet(newObject, classObject->instanceBehaviour);
                                       /* use the default hash value        */
  newObject->hashvalue = HASHOREF(newObject);
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
  RexxObject       *newObject;         /* copied object                     */

                                       /* first copy the object             */
  newObject = (RexxObject *)memoryObject.clone(this);
                                       /* have object variables?            */
  if (this->objectVariables != OREF_NULL) {
    save(newObject);                   /* protect the copy through this process */
    copyObjectVariables(newObject);    /* copy the object variables into the new object */
    discard_hold(newObject);           /* release lock on the copy          */
  }
                                       /* have instance methods?            */
  if (this->behaviour->instanceMethodDictionary != OREF_NULL)
                                       /* need to copy the behaviour        */
    BehaviourSet(newObject, newObject->behaviour->copy());
  return newObject;                    /* return the copied version         */
}

void RexxObject::copyObjectVariables(RexxObject *newObject)
/******************************************************************************/
/* Function:  Copy an object's object variable dictionaries into another obj. */
/******************************************************************************/
{
    RexxVariableDictionary *dictionary = objectVariables;
    /* clear out the existing object variable pointer */
    newObject->objectVariables = OREF_NULL;

    while (dictionary != OREF_NULL) {
        /* copy the dictionary */
        RexxVariableDictionary *newDictionary = (RexxVariableDictionary *)dictionary->copy();
        /* add this to the variable set */
        newObject->addObjectVariables(newDictionary);
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
  RexxActivationBase *activation;      /* current activation                */
  RexxObject         *sender;          /* sending activation                */

                                       /* get the top activation            */
  activation = CurrentActivity->current();
                                       /* have an activation?               */
  if (activation != (RexxActivationBase *)TheNilObject) {
    sender = activation->getReceiver();/* get the receiving object          */
    if (sender == (RexxObject *)this)  /* the same receiver?                */
      return method;                   /* just return the same method       */
  }
  return (RexxMethod *)TheNilObject;   /* return a failure indicator        */
}

RexxObject * RexxObject::sendMessage(
    RexxString      *message,          /* name of the message to process    */
    RexxArray  *arguments )            /* array of arguments                */
/******************************************************************************/
/* Function:  Issue a using a set of arguments already in an array item       */
/******************************************************************************/
{
  return this->messageSend(message, arguments->size(), arguments->data());
}

RexxObject * RexxObject::sendMessage(
    RexxString *message,               /* name of the message to process    */
    RexxObject *argument1,             /* first argument                    */
    RexxObject *argument2 )            /* second argument                   */
/******************************************************************************/
/* Function:  Send a message with two arguments                               */
/******************************************************************************/
{
  RexxObject *arguments[2];            /* argument array                    */

  arguments[0] = argument1;            /* set each argument                 */
  arguments[1] = argument2;
                                       /* just pass on to message send      */
  return this->messageSend(message, 2, arguments);
}

RexxObject * RexxObject::sendMessage(
    RexxString *message,               /* name of the message to process    */
    RexxObject *argument1,             /* first argument                    */
    RexxObject *argument2,             /* second argument                   */
    RexxObject *argument3 )            /* third argument                    */
/******************************************************************************/
/* Function:  Send a message with three arguments                             */
/******************************************************************************/
{
  RexxObject *arguments[3];            /* argument array                    */

  arguments[0] = argument1;            /* set each argument                 */
  arguments[1] = argument2;
  arguments[2] = argument3;
                                       /* just pass on to message send      */
  return this->messageSend(message, 3, arguments);
}

RexxObject * RexxObject::sendMessage(
    RexxString *message,               /* name of the message to process    */
    RexxObject *argument1,             /* first argument                    */
    RexxObject *argument2,             /* second argument                   */
    RexxObject *argument3,             /* third argument                    */
    RexxObject *argument4 )            /* fourth argument                   */
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
  return this->messageSend(message, 4, arguments);
}

RexxObject * RexxObject::sendMessage(
    RexxString *message,               /* name of the message to process    */
    RexxObject *argument1,             /* first argument                    */
    RexxObject *argument2,             /* second argument                   */
    RexxObject *argument3,             /* third argument                    */
    RexxObject *argument4,             /* fourth argument                   */
    RexxObject *argument5 )            /* fifth argument                    */
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
  return this->messageSend(message, 5, arguments);
}

RexxObject * RexxObject::messageSend(
    RexxString      *msgname,          /* name of the message to process    */
    LONG             count,            /* count of arguments                */
    RexxObject     **arguments )       /* array of arguments                */
/******************************************************************************/
/* Function:    send a message (with message lookup) to an object.            */
/*              All types of methods are handled and dispatched               */
/******************************************************************************/
{
  RexxObject     *result;              /* returned result                   */

  msgname_save = msgname;              /* save the message name             */
  CurrentActivity->stackSpace();       /* have enough stack space?          */
                                       /* grab the method from this level   */
  method_save = this->behaviour->methodLookup(msgname);
                                       /* method exists...special processing*/
  if (method_save != (RexxMethod *)TheNilObject && method_save->isSpecial()) {
    if (method_save->isPrivate())      /* actually private method?          */
                                       /* go validate a private method      */
      method_save = this->checkPrivate(method_save);
                                       /* now process protected methods     */
    if (method_save != (RexxMethod *)TheNilObject && method_save->isProtected())
                                       /* really a protected method         */
      return this->processProtectedMethod(msgname, count, arguments);
  }
                                       /* have a method                     */
  if (method_save != (RexxMethod *)TheNilObject) {
                                       /* run the method                    */
    result = method_save->run(CurrentActivity, this, msgname, count, arguments);
    return result;                     /* return the result                 */
  }
                                       /* go process an unknown method      */
  return this->processUnknown(msgname, count, arguments);
}

RexxObject * RexxObject::messageSend(
    RexxString      *msgname,          /* name of the message to process    */
    LONG             count,            /* count of arguments                */
    RexxObject     **arguments,        /* array of arguments                */
    RexxObject      *startscope)       /* starting superclass scope         */
/******************************************************************************/
/* Function:    send a message (with message lookup) to an object.            */
/*              All types of methods are handled and dispatched               */
/******************************************************************************/
{
  msgname_save = msgname;              /* save the message name             */
  CurrentActivity->stackSpace();       /* have enough stack space?          */
                                       /* go to the higher level            */
  method_save = this->superMethod(msgname, startscope);
  if (method_save != (RexxMethod *)TheNilObject && method_save->isProtected()) {
    if (method_save->isPrivate())      /* actually private method?          */
                                       /* go validate a private method      */
      method_save = this->checkPrivate(method_save);
    else                               /* really a protected method         */
      return this->processProtectedMethod(msgname, count, arguments);
  }
                                       /* have a method                     */
  if (method_save != (RexxMethod *)TheNilObject)
                                       /* run the method                    */
    return method_save->run(CurrentActivity, this, msgname, count, arguments);
                                       /* go process an unknown method      */
  return this->processUnknown(msgname, count, arguments);
}

RexxObject * RexxObject::processProtectedMethod(
    RexxString   * messageName,        /* message to issue                  */
    LONG           count,              /* count of arguments                */
    RexxObject  ** arguments)          /* actual message arguments          */
/******************************************************************************/
/* Function:  Process an unknown message, uncluding looking for an UNKNOWN    */
/*            method and raising a NOMETHOD condition                         */
/******************************************************************************/
{
  RexxArray          *argumentArray;   /* unknown method argument array     */
  RexxDirectory      *securityArgs;    /* security arguments                */
  RexxMethod         *thisMethod;      /* saved method                      */
  RexxActivationBase *activation;      /* current activation                */

                                       /* get the top activation            */
  activation = CurrentActivity->current();
                                       /* have an activation?               */
  if (activation != (RexxActivationBase *)TheNilObject) {
                                       /* have a security manager?          */
    if (activation->hasSecurityManager()) {
      thisMethod = method_save;        /* save the original method          */
      securityArgs = new_directory();  /* get the security args             */
                                       /* stuff in the name                 */
      securityArgs->put(messageName, OREF_NAME);
                                       /* add in the actual object          */
      securityArgs->put(this, OREF_OBJECTSYM);
      /* get an array for the arguments */
      argumentArray = new (count, arguments) RexxArray;
                                       /* add in the arguments              */
      securityArgs->put(argumentArray, OREF_ARGUMENTS);
                                       /* now go ask permission             */
      if (((RexxActivation *)activation)->callSecurityManager(OREF_METHODNAME, securityArgs))
                                       /* handled, just return the result   */
        return securityArgs->fastAt(OREF_RESULT);
      method_save = thisMethod;        /* restore the saved method          */
      msgname_save = messageName;      /* restore the message name          */
    }
  }
                                       /* run the method                    */
  return method_save->run(CurrentActivity, this, messageName, count, arguments);
}

RexxObject * RexxObject::processUnknown(
    RexxString   * messageName,        /* message to issue                  */
    LONG           count,              /* count of arguments                */
    RexxObject  ** arguments )         /* actual message arguments          */
/******************************************************************************/
/* Function:  Process an unknown message, uncluding looking for an UNKNOWN    */
/*            method and raising a NOMETHOD condition                         */
/******************************************************************************/
{
  RexxObject     *unknown_arguments[2];/* arguments to the unknown method   */
  INT             i;                   /* loop counter                      */
  RexxArray      *argumentArray;       /* unknown method argument array     */

                                       /* no method for this msgname        */
                                       /* find the unknown method           */
  method_save = this->behaviour->methodLookup(OREF_UNKNOWN);
  if (method_save == TheNilObject)     /* "unknown" method exists?          */
                                       /* no unknown method - try to raise  */
                                       /* a NOMETHOD condition, and if that */
    report_nomethod(messageName, this);/* fails, it is an error message     */
  argumentArray = new_array(count);    /* get an array for the arguments    */
  for (i = 1; i <= count; i++)         /* copy the arguments into an array  */
    argumentArray->put(arguments[i - 1], i);
  unknown_arguments[0] = messageName;  /* method name is first argument     */
                                       /* second argument is array of       */
  unknown_arguments[1] = argumentArray;/* arguments for the original call   */
                                       /* run the unknown method            */
  return method_save->run(CurrentActivity, this, OREF_UNKNOWN, 2, unknown_arguments);
}

RexxMethod * RexxObject::methodLookup(
    RexxString *msgname)               /* name of the target message        */
/******************************************************************************/
/* Function:  Return the method object associated with a message name         */
/******************************************************************************/
{
  return this->behaviour->methodLookup(msgname);
}

RexxMethod * RexxObject::methodObject(
    RexxString *msgname)               /* name of the target message        */
/******************************************************************************/
/* Function:  Return the method object associated with a message name         */
/******************************************************************************/
{
  RexxObject *startScope;              /* message target scope              */
  RexxString *message;                 /* string message name               */
  RexxArray  *arrayMessage;            /* message as an array version       */

  required_arg(msgname, ONE);          /* make sure we have the requireds   */

                                       /* if start scope passed, do a       */
                                       /* superclass search                 */
  arrayMessage = (RexxArray *)REQUEST_ARRAY(msgname);
                                       /* convert ok?                       */
  if (arrayMessage != TheNilObject && arrayMessage->getDimension() == 1) {
    startScope = arrayMessage->get(2L);/* get the scope                     */
                                       /* and the message name              */
    message = (RexxString *)arrayMessage->get(1L);
                                       /* and do the lookup                 */
    return this->superMethod(message, startScope);
  }
  else {
                                       /* just do the simple method lookup  */
                                       /* behaviour methodObject does all   */
                                       /* needed error checks               */
    return this->behaviour->methodLookup(msgname);
  }
}

long RexxInternalObject::longValue(
    size_t precision)                  /* precision to use                  */
/******************************************************************************/
/* Function:  Convert a primitive internal object to a long value             */
/******************************************************************************/
{
  return NO_LONG;                      /* give a "safe" default here        */
}

RexxInteger * RexxInternalObject::integerValue(
    size_t precision)                  /* precision to use                  */
/******************************************************************************/
/* Function:  Convert a primitive internal object to an integer value         */
/******************************************************************************/
{
  return (RexxInteger *)TheNilObject;  /* give a "safe" default here        */
}

double RexxInternalObject::doubleValue()
/******************************************************************************/
/* Function:  Convert a primitive internal object to a double value           */
/******************************************************************************/
{
  return NO_DOUBLE;                    /* give a "safe" default here        */
}

RexxNumberString * RexxInternalObject::numberString()
/******************************************************************************/
/* Function:  convert an internal object to a numberstring representation     */
/******************************************************************************/
{
  return OREF_NULL;                    /* this never converts               */
}

long RexxObject::longValue(
    size_t precision)                  /* precision to use                  */
/******************************************************************************/
/* Function:  Convert a REXX object to a long value                           */
/******************************************************************************/
{
                                       /* get a string and convert          */
  return REQUEST_STRING(this)->longValue(precision);
}

long RexxObject::longValueNoNOSTRING(
    size_t precision)                  /* precision to use                  */
/******************************************************************************/
/* Function:  Convert a REXX object to a long value                           */
/******************************************************************************/
{
                                       /* get a string and convert          */
  return this->requestStringNoNOSTRING()->longValue(precision);
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

double RexxObject::doubleValue()
/******************************************************************************/
/* Function:  Convert a primitive internal object to a double value           */
/******************************************************************************/
{
                                       /* get a string and convert          */
  return this->requestString()->doubleValue();
}

double RexxObject::doubleValueNoNOSTRING()
/******************************************************************************/
/* Function:  Convert a primitive internal object to a double value           */
/******************************************************************************/
{
                                       /* get a string and convert          */
  return this->requestStringNoNOSTRING()->doubleValue();
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
  if (isPrimitive(this))               /* primitive object?                 */
    return (RexxString *)TheNilObject; /* this never converts               */
  else                                 /* process as a string request       */
    return (RexxString *)this->sendMessage(OREF_REQUEST, OREF_STRINGSYM);
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
  if (isPrimitive(this))               /* primitive object?                 */
    return (RexxArray *)TheNilObject;  /* this never converts               */
  else                                 /* process as a string request       */
    return (RexxArray *)this->sendMessage(OREF_REQUEST, OREF_ARRAYSYM);
}

RexxString *RexxObject::requestString()
/******************************************************************************/
/* Function:  Handle a string request for a REXX object.  This will go        */
/*            through the whole search order to do the conversion.            */
/******************************************************************************/
{
  RexxString *string_value;            /* converted object                  */

                                       /* primitive object?                 */
  if (isPrimitive((RexxObject *)this)) {
                                       /* get the string representation     */
    string_value = this->primitiveMakeString();
    if (string_value == TheNilObject) {/* didn't convert?                   */
                                       /* get the final string value        */
      string_value = this->stringValue();
                                       /* raise a NOSTRING condition        */
      CurrentActivity->raiseCondition(OREF_NOSTRING, OREF_NULL, string_value, (RexxObject *)this, OREF_NULL, OREF_NULL);
    }
  }
  else {                               /* do a real request for this        */
    string_value = (RexxString *)this->sendMessage(OREF_REQUEST, OREF_STRINGSYM);
    if (string_value == TheNilObject) {/* didn't convert?                   */
                                       /* get the final string value        */
      string_value = (RexxString *)this->sendMessage(OREF_STRINGSYM);
                                       /* raise a NOSTRING condition        */
      CurrentActivity->raiseCondition(OREF_NOSTRING, OREF_NULL, string_value, this, OREF_NULL, OREF_NULL);
    }
  }
  return string_value;                 /* return the converted form         */
}

RexxString *RexxObject::requestStringNoNOSTRING()
/******************************************************************************/
/* Function:  Handle a string request for a REXX object.  This will go        */
/*            through the whole search order to do the conversion.            */
/******************************************************************************/
{
  RexxString *string_value;            /* converted object                  */

                                       /* primitive object?                 */
  if (isPrimitive((RexxObject *)this)) {
                                       /* get the string representation     */
    string_value = this->primitiveMakeString();
    if (string_value == TheNilObject) {/* didn't convert?                   */
                                       /* get the final string value        */
      string_value = this->stringValue();
    }
  }
  else {                               /* do a real request for this        */
    string_value = (RexxString *)this->sendMessage(OREF_REQUEST, OREF_STRINGSYM);
    if (string_value == TheNilObject) {/* didn't convert?                   */
                                       /* get the final string value        */
      string_value = (RexxString *)this->sendMessage(OREF_STRINGSYM);
    }
  }
  return string_value;                 /* return the converted form         */
}

RexxString *RexxObject::requiredString(
    LONG  position )                   /* required argument position        */
/******************************************************************************/
/* Function:  Handle a string request for a REXX object in a context where    */
/*            the object MUST have a string value.                            */
/******************************************************************************/
{
  RexxString *string_value;            /* converted object                  */

  if (isPrimitive(this))               /* primitive object?                 */
    string_value = this->makeString(); /* get the string representation     */
  else                                 /* do a full request for this        */
    string_value = (RexxString *)this->sendMessage(OREF_REQUEST, OREF_STRINGSYM);
                                       /* didn't convert?                   */
  if (string_value == (RexxString *)TheNilObject)
                                       /* this is an error                  */
    report_exception1(Error_Incorrect_method_nostring, new_integer(position));
  return string_value;                 /* return the converted form         */
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
    if (isPrimitive(this))
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
  if (isPrimitive(this))               /* primitive object?                 */
                                       /* return the integer value          */
    return this->integerValue(precision);
  else                                 /* return integer value of string    */
    return this->requestString()->integerValue(precision);
}

RexxInteger *RexxObject::requiredInteger(
    LONG   position,                   /* precision to use                  */
    size_t precision)                  /* argument position for errors      */
/******************************************************************************/
/* Function:  Request an integer value from an object.  If this is not a      */
/*            primitive object, a REQUEST('STRING') will be performeding,     */
/*            and then the string integer value will be returned.             */
/******************************************************************************/
{
  RexxInteger *result;                 /* returned result                   */

  if (isPrimitive(this))               /* primitive object?                 */
                                       /* return the integer value          */
    result = this->integerValue(precision);
  else                                 /* return integer value of string    */
    result = this->requiredString(position)->integerValue(precision);
                                       /* didn't convert well?              */
  if (result == (RexxInteger *)TheNilObject)
                                       /* raise an error                    */
    report_exception2(Error_Incorrect_method_whole, new_integer(position), this);
  return result;                       /* return the new integer            */
}

LONG  RexxObject::requestLong(
    size_t precision )                 /* precision to use                  */
/******************************************************************************/
/* Function:  Request a long value from an object.  If this is not a          */
/*            primitive object, the object will be converted to a string,     */
/*            and then the string long value will be returned.                */
/******************************************************************************/
{
  if (isPrimitive(this))               /* primitive object?                 */
    return this->longValue(precision); /* return the long value             */
  else                                 /* return integer value of string    */
    return this->requestString()->longValue(precision);
}

LONG  RexxObject::requiredLong(
    LONG   position ,                  /* precision to use                  */
    size_t precision)                  /* argument position for errors      */
/******************************************************************************/
/* Function:  Request a long value from an object.  If this is not a          */
/*            primitive object, a REQUEST('STRING') will be performed         */
/*            and then the string long value will be returned.                */
/******************************************************************************/
{
  LONG         result;                 /* returned result                   */

                                       /* primitive object?                 */
  if (isPrimitive(this) && !OTYPE(Object, this))
                                       /* return the integer value          */
    result = this->longValue(precision);
  else                                 /* return integer value of string    */
    result = this->requiredString(position)->longValue(precision);
  if (result == NO_LONG)               /* didn't convert well?              */
                                       /* raise an error                    */
    report_exception2(Error_Incorrect_method_whole, new_integer(position), this);
  return result;                       /* return the result                 */
}

LONG  RexxObject::requiredPositive(
    LONG   position,                   /* precision to use                  */
    size_t precision)                  /* argument position for errors      */
/******************************************************************************/
/* Function:  Request a a positive long value from an object.  If this is not */
/*            positive, it will raise an error.                               */
/******************************************************************************/
{
  LONG         result;                 /* returned result                   */

                                       /* get the value first               */
  result = this->requiredLong(position, precision);
  if (result <= 0)                     /* non-positive number?              */
                                       /* raise the error                   */
    report_exception2(Error_Incorrect_method_positive, new_integer(position), this);
  return result;                       /* return the result                 */
}

LONG  RexxObject::requiredNonNegative(
    LONG   position ,                  /* precision to use                  */
    size_t precision)                  /* argument position for errors      */
/******************************************************************************/
/* Function:  Request a non-negative long value from an object.  If this is   */
/*            less than zero, it will raise an error                          */
/******************************************************************************/
{
  LONG         result;                 /* returned result                   */

                                       /* get the value first               */
  result = this->requiredLong(position, precision);
  if (result < 0)                      /* negative number?                  */
                                       /* raise the error                   */
    report_exception2(Error_Incorrect_method_nonnegative, new_integer(position), this);
  return result;                       /* return the result                 */
}

RexxArray *RexxObject::requestArray()
/******************************************************************************/
/* Function:  Request an array value from an object.                          */
/******************************************************************************/
{
  if (isPrimitive(this)) {             /* primitive object?                 */
    if (OTYPE(Array, this))            /* already an array?                 */
      return (RexxArray *)this;        /* return directly, don't makearray  */
    else
      return this->makeArray();        /* return the array value            */
  }
  else                                 /* return integer value of string    */
    return (RexxArray *)this->sendMessage(OREF_REQUEST, OREF_ARRAYSYM);
}

RexxString *RexxObject::objectName()
/******************************************************************************/
/* Function:  Retrieve the object name for a primitive object                 */
/******************************************************************************/
{
  RexxObject *scope;                   /* method's variable scope           */
  RexxString *string_value;            /* returned string value             */

  scope = last_method()->scope;        /* get the method's scope            */
                                       /* get the object name variable      */
  string_value = (RexxString *)this->getObjectVariable(OREF_NAME, scope);
  if (string_value == OREF_NULL) {     /* no name?                          */
    if (isPrimitive(this))             /* primitive object?                 */
                                       /* use fast path to default name     */
      string_value = this->defaultName();
    else                               /* go through the full search        */
      string_value = (RexxString *)this->sendMessage(OREF_DEFAULTNAME);
  }
  return string_value;                 /* return the string value           */
}

RexxObject  *RexxObject::objectNameEquals(RexxObject *name)
/******************************************************************************/
/* Function:  retrieve an objects default name value                          */
/******************************************************************************/
{
  RexxObject *scope;                   /* scope of the object               */

  required_arg(name, ONE);             /* must have a name                  */
  scope = last_method()->scope;        /* get the method's scope            */
                                       /* get this as a string              */
  name = (RexxObject *)REQUIRED_STRING(name, ARG_ONE);
                                       /* set the name                      */
  this->setObjectVariable(OREF_NAME, name, scope);
  return OREF_NULL;                    /* no return value                   */
}

RexxObject  *RexxObject::setAttribute(RexxObject *value)
/******************************************************************************/
/* Function:  set an object's named attribute to a value                  */
/******************************************************************************/
{
  required_arg(value, ONE);            /* must have a value                 */
                                       /* just directly set the value       */
  method_save->getAttribute()->set(this->getObjectVariables(method_save->scope), value);
  return OREF_NULL;                    /* no return value                   */
}

RexxObject  *RexxObject::getAttribute()
/******************************************************************************/
/* Function:  retrieve an object's named attribute                            */
/******************************************************************************/
{
                                       /* just directly retrieve the value  */
  return method_save->getAttribute()->getValue(this->getObjectVariables(method_save->scope));
}

RexxObject  *RexxObject::abstractMethod(RexxObject **args, size_t count)
{
    report_exception1(Error_Incorrect_method_abstract, last_msgname());
    return OREF_NULL;
}

RexxString  *RexxObject::defaultName()
/******************************************************************************/
/* Function:  Handle "final" string coercion level                            */
/******************************************************************************/
{
  RexxString* defaultname;             /* returned default name             */

                                       /* use the class id as the default   */
                                       /* name                              */
  defaultname = this->behaviour->getCreateClass()->id;
                                       /* check if it is from an enhanced   */
  if (this->behaviour->isEnhanced()) { /* class                             */
                                       /* return the 'enhanced' id          */
   return defaultname->concatToCstring("enhanced ");
  }
  switch (defaultname->getChar(0)) {   /* process based on first character*/
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
  return (this->behaviour->methodObject(msgname) != TheNilObject) ? TheTrueObject : TheFalseObject;
}

RexxClass   *RexxObject::classObject()
/******************************************************************************/
/* Function:  Return the class object associated with an object               */
/******************************************************************************/
{
                                       /* just return class from behaviour  */
  return this->behaviour->getCreateClass();
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
  msgname = REQUIRED_STRING(msgname, ARG_ONE)->upper();
  if (option) {
    option = REQUIRED_STRING(option, ARG_THREE);
    if (!stricmp("OBJECT",option->stringData)) {
      // do nothing if OBJECT
    } else if (!stricmp("FLOAT",option->stringData))
      // "FLOAT" makes option a NULL pointer, causing the old default behaviour on setMethod...
      option = OREF_NULL;
    else
      report_exception4(Error_Incorrect_call_list, new_cstring(CHAR_SETMETHOD), IntegerThree, new_cstring("\"FLOAT\", \"OBJECT\""), option);
  }

  if (methobj == OREF_NULL)            /* we weren't passed a method,       */
                                       /* add a dummy method                */
    methobj = (RexxMethod *)TheNilObject;
  else if (!OTYPE(Method, methobj))    /* not a method type already?        */
                                       /* make one from a string or array   */
    methobj = TheMethodClass->newRexxCode(msgname, (RexxObject *)methobj, IntegerTwo, OREF_NULL);
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
  msgname = REQUIRED_STRING(msgname, ARG_ONE)->upper();
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
  RexxString *  make_method;           /* needed make method                */
  RexxMethod *  method;                /* MAKExxxxx method pointer          */
  RexxString *  class_id;              /* class of this object              */

                                       /* Verify we have a string parm      */
  className = REQUIRED_STRING(className, ARG_ONE)->upper();
  class_id = this->id()->upper();      /* get the class name in uppercase   */
                                       /* of the same class?                */
  if (className->strictEqual(class_id) == TheTrueObject) {
    return this;                       /* already converted                 */
  }
                                       /* Get "MAKE"||class methodname      */
  make_method = className->concatToCstring(CHAR_MAKE);
                                       /* find the MAKExxxx method          */
  method = this->behaviour->methodLookup(make_method);
                                       /* have this method?                 */
  if (method != (RexxMethod *)TheNilObject)
                                       /* Return its results                */
    return (RexxObject *)this->sendMessage(make_method);
  else                                 /* No makeclass method               */
    return TheNilObject;               /* Let user handle it                */
}

RexxMessage *RexxObject::start(
    RexxObject **arguments,            /* message arguments                 */
    size_t       argCount)             /* the number of arguments           */
/******************************************************************************/
/* Function:  Spin a message off on a seperate activity                       */
/******************************************************************************/
{
  RexxMessage *newMessage;             /* new message object                */
  RexxObject  *message;                /* message to be sent to receiver.   */
                                       /* message to be sent to receiver.   */
  RexxArray   *messageArray = (RexxArray*) TheNilObject;
  RexxActivationBase *activation;      /* current activation                */
  RexxObject  *sender;                 /* sending object                    */
  RexxString  *newMsgName;             /* msgname to be sent                */

#ifdef NOTHREADSUPPORT
   report_exception1(Error_Execution_no_concurrency,
                  new_cstring("Concurrency not supported"));
#else

  if (argCount < 1 )                   /* no arguments?                     */
    missing_argument(ARG_ONE);         /* Yes, this is an error.            */
                                       /* Get the message name.             */
  message = arguments[0];              /* get the message .                 */
                                       /* Did we receive a message name     */
  if (message == OREF_NULL)
    missing_argument(ARG_ONE);         /* Yes, this is an error.            */

                                       /* if 1st arg is a string, we can do */
                                       /* this quickly                      */
  if (!OTYPE(String, message)) {
                                       /* is this an array?                 */
    if (OTYPE(Array, message)) {
      messageArray = (RexxArray*) message;
    } else {
      RexxClass *theClass = message->classObject();
      RexxArray *classes = theClass->classSuperClasses;
      size_t i = classes->numItems();
      for (; i != 0; i--) {
        if (classes->get(i) == TheStringClass)
          break;
      }
      if (i == 0) {                    /* not subclassed from string?       */
                                       /* see if this is an array item      */
        messageArray = REQUEST_ARRAY(message);
      }
    }
  }
  if (messageArray != TheNilObject) {  /* is message specified as an array? */
                                       /* didn't get two arguments?         */
    if (messageArray->getDimension() != 1 || messageArray->size() != 2)
                                       /* raise an error                    */
      report_exception(Error_Incorrect_method_message);
                                       /* get the message as a string       */
    newMsgName = REQUIRED_STRING(messageArray->get(1), ARG_ONE);
                                       /* Was starting scope omitted ?      */
    if (OREF_NULL == messageArray->get(2))
                                       /* Yes, this is an error, report it. */
      report_exception1(Error_Incorrect_method_noarg, IntegerTwo);
                                       /* get the top activation            */
    activation = CurrentActivity->current();
                                       /* have an activation?               */
    if (activation != (RexxActivation *)TheNilObject) {
                                       /* get the receiving object          */
      sender = (RexxObject *)activation->u_receiver;
      if (sender != this)              /* not the same receiver?            */
                                       /* this is an error                  */
         report_exception(Error_Execution_super);
    }
    else
                                       /* this is an error                  */
      report_exception(Error_Execution_super);
  }
  else {                               /* not an array as message.          */
                                       /* force to a string value           */
    message = REQUIRED_STRING(message, ARG_ONE);
  }
                                       /* Create the new message object.    */
  newMessage = (RexxMessage *)save(new_message(this, message, new (argCount - 1, arguments + 1) RexxArray));
  newMessage->start(OREF_NULL);        /* Tell the message object to start  */
  discard_hold(newMessage);            /* make sure message object stays    */
#endif                                 // end of NOTHREADSUPPORT
  return newMessage;                   /* return the new message object     */
}

RexxEnvelope*RexxObject::startAt(RexxObject **args, size_t argCount)
/****************************************************************************/
/* Function:  Flatten and run a method "elsewhere"                          */
/****************************************************************************/
{
  RexxEnvelope *envelope;
  RexxSmartBuffer *envelopeBuffer;
  RexxBuffer      *buffer;
  PVOID            data;
  RexxString *location;
  RexxString *message;
  RexxArray  *arguments;
  long bufferLength;

  FILE *dumpfile;

                                       /* Get new envelope object           */
  envelope  = (RexxEnvelope *)save(new_envelope());

                                       /* now break apart args, no error    */
                                       /* checking now, will come later     */
                                       /* Where are we going?               */
  location  = (RexxString *)args[0];
                                       /* what message are we sending?      */
  message   = (RexxString *)args[1];
                                       /* all remaining args are arguments  */
                                       /*  to the message, so get rest into */
                                       /*  and array.                       */
  arguments = new (argCount - 2, args + 2) RexxArray;

                                       /* now pack up the envelope for send */
                                       /* to remote system, the receiver is */
                                       /*  ourself                          */
  envelope->pack(location, this, message, arguments);

  /* ******************************************************************************** */
  /* ***   Temporary - Just dump object to a file for now.                        *** */
  /* ******************************************************************************** */
                                       /* get the smartbuffer               */
  envelopeBuffer = envelope->getBuffer();
  buffer = envelopeBuffer->getBuffer();/* get the buffer                    */
  data = (PVOID)buffer->data;          /* get the address                   */
  dumpfile = fopen("flatten.obj","wb");/* open for binary write             */
  bufferLength = (long)envelopeBuffer->current;
                                            /* Write in te size of buffer           */
  fwrite((PVOID)&bufferLength, 1, sizeof(bufferLength), dumpfile);
                                            /* NOW WRITE IN ENTIRE BUFFER           */
  fwrite((PVOID)data, 1, bufferLength, dumpfile);
  fclose(dumpfile);
  return envelope;                         /* Temp for now ...                     */
}

RexxString  *RexxObject::oref()
/****************************************************************************/
/* Function:  Return the objects address as a HEX string (debugging only)   */
/****************************************************************************/
{
  char buffer[9];                      /* buffered address                  */

  sprintf(buffer, "%08lX", this);      /* format this                       */
                                       /* and return a string               */
  return (RexxString *)new_string(buffer,8);
}

RexxObject  *RexxInternalObject::hasUninit()
/****************************************************************************/
/* Function:  Tag an object as having an UNINIT method                      */
/****************************************************************************/
{
                                       /* tell the activity about this      */
   CurrentActivity->addUninitObject((RexxObject *)this);
   return OREF_NULL;
}

RexxObject  *RexxObject::shriekRun(
    RexxMethod * method,               /* method to invoke                  */
    RexxString * calltype,             /* type of invocation                */
    RexxString * environment,          /* initial address                   */
    RexxObject **arguments,            /* array of arguments                */
    size_t       argCount)             /* the number of arguments           */
/****************************************************************************/
/* Function:  Run a method as a program                                     */
/****************************************************************************/
{
  RexxObject * result;
                                       /* ensure correct scope              */
  method = method->newScope((RexxClass *)this);
  /* go run the method                 */
  result = method->call(CurrentActivity, this, OREF_NONE, arguments, argCount, calltype, environment, PROGRAMCALL);
  if ((result != OREF_NULL) && method->isRexxMethod()) discard(result);
  return result;
}

RexxObject  *RexxObject::run(
    RexxObject **arguments,            /* method arguments                  */
    size_t       argCount)             /* the number of arguments           */
/****************************************************************************/
/* Function:  Run a method on an object as if it was part of the object's   */
/*            behaviour.                                                    */
/****************************************************************************/
{
  RexxMethod *methobj;                 /* the object method                 */
  RexxString *option;                  /* run option string                 */
  RexxArray  *arglist = OREF_NULL;     /* forwarded option string           */
  RexxObject * result;
  RexxObject **argumentPtr = NULL;     /* default to no arguments passed along */
  size_t argcount = 0;

                                       /* get the method object             */
  methobj = (RexxMethod *)arguments[0];
  required_arg(methobj, ONE);          /* make sure we have a method        */
  if (!OTYPE(Method, methobj)) {       /* this a method object?             */
                                       /* create a method object            */
    methobj = TheMethodClass->newRexxCode(OREF_RUN, (RexxObject *)methobj, IntegerOne, OREF_NULL);
                                       /* set the correct scope             */
    methobj->setScope((RexxClass *)TheNilObject);
  }
  else
                                       /* ensure correct scope on method    */
    methobj = methobj->newScope((RexxClass *)TheNilObject);
  // we need to save this, since we might be working off of a newly created
  // one or a copy
  save(methobj);

  if (argCount > 1L) {                 /* if any arguments passed           */
                                       /* get the 1st one, its the option   */
    option = (RexxString *)arguments[1];
                                       /* this is now required              */
    option = REQUIRED_STRING(option, ARG_TWO);
                                       /* process the different options     */
    switch (toupper(option->getChar(0))) {
      case 'A':                        /* args are an array                 */
                                       /* so they say, make sure we have an */
                                       /* array and we were only passed 3   */
                                       /*args                               */
        if (argCount < 3)              /* not enough arguments?             */
          missing_argument(ARG_THREE); /* this is an error                  */
        if (argCount > 3)              /* too many arguments?               */
         report_exception1(Error_Incorrect_method_maxarg, IntegerThree);
                                       /* now get the array                 */
        arglist = (RexxArray *)arguments[2];
                                       /* force to array form               */
        arglist = REQUEST_ARRAY(arglist);
                                       /* not an array?                     */
        if (arglist == TheNilObject || arglist->getDimension() != 1)
                                       /* raise an error                    */
          report_exception1(Error_Incorrect_method_noarray, arguments[2]);
        // request array may create a new one...keep it safe
        save(arglist);
        /* grab the argument information */
        argumentPtr = arglist->data();
        argcount = arglist->size();
        break;

      case 'I':                        /* args are "strung out"             */
        /* point to the array data for the second value */
        argumentPtr = arguments + 2;
        argcount = argCount - 2;
        break;

      default:
                                       /* invalid option                    */
        report_exception2(Error_Incorrect_method_option, new_cstring("AI"), option);
        break;
    }
  }
                                       /* now just run the method....       */
  result = methobj->call(CurrentActivity, this, OREF_NONE, argumentPtr, argcount, OREF_METHODNAME, OREF_NULL, METHODCALL);

  discard(methobj);
  // and if we have a saved argument, release it also
  if (arglist != OREF_NULL)
  {
      discard(arglist);
  }
  //if ((result != OREF_NULL) && u_method->isRexxMethod()) discard(result);
  return result;
}

RexxObject  *RexxObject::defMethods(
    RexxDirectory *methods)            /* new table of methods              */
/****************************************************************************/
/* Function:  Add a table of methods to an object's behaviour               */
/****************************************************************************/
{
  LONG i;                              /* loop counter                      */
  RexxMethod *method;
  RexxString *name;

                                       /* make a copy of the behaviour      */
  OrefSet(this, this->behaviour, (RexxBehaviour *)this->behaviour->copy());
                                       /* loop through the list of methods  */
  for (i = methods->first(); methods->available(i); i = methods->next(i)) {
                                       /* Get the methjod Object            */
    method = (RexxMethod *)methods->value(i);
    if (method != TheNilObject)        /* not a removal?                    */
                                       /* set a new scope on this           */
      method = method->newScope((RexxClass *)this);
    else
                                       /* no real method added              */
      method = (RexxMethod *)TheNilObject;
                                       /* Get the name for this method      */
    name = (RexxString *)methods->index(i);
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
  if (methobj != TheNilObject) {       /* not a removal?                    */
                                       /* got an option? */
    if (option) {
      if (!stricmp("OBJECT",option->stringData))
        targetClass = this->behaviour->getCreateClass();
      else
        report_exception4(Error_Incorrect_call_list, new_cstring(CHAR_SETMETHOD), IntegerThree, new_cstring("\"FLOAT\", \"OBJECT\""), option);
    }
                                       /* set a new scope on this           */
    methcopy = methobj->newScope(targetClass);
  }
  else
                                       /* no real method added              */
    methcopy = (RexxMethod *)TheNilObject;
                                       /* is this the first added method?   */
  if (this->behaviour->instanceMethodDictionary == OREF_NULL) {

/* copy primitive behaviour object and define the method, a copy is made to */
/* ensure that we don't update the behaviour of any other object, since they*/
/* may have been sharing the mvd.                                           */
    OrefSet(this, this->behaviour, (RexxBehaviour *)this->behaviour->copy());
  }
                                       /* now add this to the behaviour     */
  this->behaviour->addMethod(msgname, methcopy);
                                       /* adding an UNINIT method to obj?   */
  if (methobj != TheNilObject && msgname->strCompare(CHAR_UNINIT)) {
    this->hasUninit();                 /* yes, mark it as such              */
  }
  return OREF_NULL;
}

short RexxObject::ptype()
/******************************************************************************/
/* Function:  Return the object's primitive type number                       */
/******************************************************************************/
{
  return this->behaviour->typenum();
}

void RexxInternalObject::removedUninit()
/******************************************************************************/
/* Function:  Remove an UNINIT method from an object                          */
/******************************************************************************/
{
  CurrentActivity->removeUninitObject((RexxObject *)this);
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
  RexxVariableDictionary *ovd;         /* ovd for this object at base scope */

  if (OREF_NULL == scope)              /* were we passed a scope for lookup */
    scope = this;                      /* no, we use our own.               */
                                       /* get the ovd for our scope level   */
  ovd = this->getObjectVariables(scope);
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
  RexxVariableDictionary *ovd;         /* target OVD for this scope         */

  if (OREF_NULL == scope)              /* were we passed a scope for lookup */
    scope = this;                      /* no, we use our own.               */
                                       /* get the ovd for our scope level   */
  ovd = this->getObjectVariables(scope);
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
  RexxVariableDictionary *dictionary;  /* retrieved dictionary              */

  dictionary = objectVariables;        /* get the head of the chain         */
  while (dictionary != OREF_NULL) {    /* search for a scope match          */
      /* if we've found a match, return it */
      if (dictionary->isScope(scope)) {
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
  SetObjectHasReferences(this);        /* we now have references            */
  return dictionary;                   /* return the correct ovd            */
}

char *RexxObject::idString(void)
/******************************************************************************/
/* Function:  Return a pointer to the object's string value                   */
/******************************************************************************/
{
  RexxString *classId;                 /* return class id string            */

  classId = this->id();                /* get the id string                 */
  if (classId == OREF_NULL)            /* internal class?                   */
    return "unknown Class";            /* return an unknown identifier      */
  else
    return classId->stringData;        /* return the actual class ID        */
}

RexxString *RexxObject::id(void)
/******************************************************************************/
/* Function:  Get the class string name                                       */
/******************************************************************************/
{
  RexxClass *createClass;              /* class associated with the object  */

                                       /* get the class                     */
  createClass = this->behaviourObject()->getCreateClass();
  if (createClass == OREF_NULL)        /* no class object?                  */
    return OREF_NULL;                  /* return nothing                    */
  else
    return createClass->id;            /* return the class id string        */
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


void        RexxObject::uninit(void)
/******************************************************************************/
/* Function:  Exported Object INIT method                                     */
/******************************************************************************/
{
  if (TheTrueObject == this->hasMethod(OREF_UNINIT))
    this->sendMessage(OREF_UNINIT);

}

BOOL RexxObject::hasUninitMethod()
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

#undef operatorMethod
#define operatorMethod(name, message) RexxObject * RexxObject::name(RexxObject *operand) \
{\
  RexxObject *result;                  /* returned result                   */\
                                       /* do a real message send            */\
  result = (RexxObject *)this->messageSend(OREF_##message, 1, &operand); \
  if (result == OREF_NULL)             /* in an expression and need a result*/ \
                                       /* need to raise an exception        */ \
    report_exception1(Error_No_result_object_message, OREF_##message); \
  return result;                       /* return the final result           */ \
}\


#undef prefixOperatorMethod
#define prefixOperatorMethod(name, message) RexxObject * RexxObject::name(RexxObject *operand) \
{\
  RexxObject *result;                  /* returned result                   */\
                                       /* do a real message send            */\
  result = (RexxObject *)this->messageSend(OREF_##message, operand == OREF_NULL ? 0 : 1, &operand); \
  if (result == OREF_NULL)             /* in an expression and need a result*/ \
                                       /* need to raise an exception        */ \
    report_exception1(Error_No_result_object_message, OREF_##message); \
  return result;                       /* return the final result           */ \
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
  RexxObject *newObject;               /* created new object                */

                                       /* get storage for new object        */
  newObject = (RexxObject *)new_object(size);
                                       /* use the class instance behaviour  */
  BehaviourSet(newObject, classObject->instanceBehaviour);
                                       /* use the default hash value        */
  newObject->hashvalue = HASHOREF(newObject);
                                       /* clear the object variable oref    */
  OrefSet(newObject, newObject->objectVariables, OREF_NULL);


  if (classObject->uninitDefined() || classObject->parentUninitDefined()) {  /* or parent has one */
     newObject->hasUninit();
   }

  return (void *)newObject;            /* Initialize the new object         */
}

void *RexxObject::operator new(size_t size, RexxClass *classObject, RexxObject **args, size_t argCount)
/******************************************************************************/
/* Function:  Create a new instance of object                                  */
/******************************************************************************/
{
  RexxObject *newObject;               /* created new object                */

                                       /* create a new object               */
  newObject = new (classObject) RexxObject;
  save(newObject);                     /* protect from GC                   */
                                       /* now drive the user INIT           */
  newObject->sendMessage(OREF_INIT, args, argCount);
  discard_hold(newObject);
  return newObject;                    /* and returnthe new object          */
}

void RexxObject::operator delete(void *arg)
/******************************************************************************/
/* Function:  Create a new translator object                                  */
/******************************************************************************/
{
  ;                                    // no explicit delete
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

RexxObject *RexxObject::freeSOMObjRexx()
/******************************************************************************/
/* Function:  Exported access to an object virtual function                   */
/******************************************************************************/
{
  return this->freeSOMObj();           /* forward to the virtual function   */
}

RexxObject *RexxObject::SOMObjRexx()
/******************************************************************************/
/* Function:  Exported access to an object virtual function                   */
/******************************************************************************/
{
  return this->SOMObj();               /* forward to the virtual function   */
}

RexxObject *RexxObject::serverRexx()
/******************************************************************************/
/* Function:  Exported access to an object virtual function                   */
/******************************************************************************/
{
  return this->server();               /* forward to the virtual function   */
}

RexxObject *RexxObject::initProxyRexx(
    RexxInteger *proxy)                /* the proxy object                  */
/******************************************************************************/
/* Function:  Exported access to an object virtual function                   */
/******************************************************************************/
{
  return this->initProxy(proxy);       /* forward to the virtual function   */
}

RexxObject *RexxObject::hasMethodRexx(
    RexxString *message )              /* method name                       */
/******************************************************************************/
/* Function:  Exported access to an object virtual function                   */
/******************************************************************************/
{
  return this->hasMethod(message);     /* forward to the virtual function   */
}

void RexxInternalObject::printObject()
/******************************************************************************/
/* Function:  give a formatted print of object information.                   */
/******************************************************************************/
{
    printf("Object at %p, of type %d\n",
        this, ObjectTypeNumber(this));
}

#include "RexxNativeAPI.h"

native0 (REXXOBJECT, OBJECT_NEW)

/******************************************************************************/
/* Function:  External interface to the nativeact object method               */
/******************************************************************************/
{
  native_entry;                        /* synchronize access              */
                                       /* just forward and return         */
  return_oref(new ((RexxClass *)self) RexxObject);
}
