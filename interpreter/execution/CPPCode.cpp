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

#include "RexxCore.h"
#include "CPPCode.hpp"
#include "ActivityManager.hpp"
#include "ProtectedObject.hpp"
#include "Interpreter.hpp"
#include "RexxVariableDictionary.hpp"
#include "ActivationFrame.hpp"


/**
 * Allocate a new CPP code object.
 *
 * @param size   the allocation size.
 *
 * @return A pointer to the newly allocated object.
 */
void *CPPCode::operator new(size_t size)
{
    // just allocate ane return
    return new_object(size, T_CPPCode);
}


/**
 * Constructor for a CPPCode object.
 *
 * @param index    The index of the method used to restore unflattened internal method.
 * @param entry    The entry point address.
 * @param argcount The number of arguments this method expects.
 */
CPPCode::CPPCode(size_t index, PCPPM entry, size_t argcount)
{
    methodIndex = (uint16_t)index;
    cppEntry = entry;
    argumentCount = (uint16_t)argcount;
}


void CPPCode::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
    if (reason == RESTORINGIMAGE)        /* restoring the image?              */
    {
        this->cppEntry = exportedMethods[this->methodIndex];
    }
}


RexxObject *CPPCode::unflatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  unflatten an object                                             */
/******************************************************************************/
{
    this->cppEntry = exportedMethods[this->methodIndex];
    return (RexxObject *)this;
}


/**
 * Run (or call) a CPPMethod.
 *
 * @param activity The activity we're running under.
 * @param method   The method to run.
 * @param receiver The receiver object.
 * @param messageName
 *                 The name used to invoke the message.
 * @param argPtr   The actual arguments.
 * @param count    The argument count.
 * @param result   The returned result.
 */
void CPPCode::run(RexxActivity *activity, RexxMethod *method, RexxObject *receiver, RexxString *messageName,
    RexxObject **argPtr, size_t count, ProtectedObject &result)
{
    InternalActivationFrame frame(activity, messageName, method, argPtr, count);
    PCPPM methodEntry = this->cppEntry;  /* get the entry point               */
                                       /* expecting an array?               */
                                       /* expecting a pointer/count pair?   */
    if (this->argumentCount == A_COUNT)
    {
                                       /* we can pass this right on         */
      result = (receiver->*((PCPPMC1)methodEntry))(argPtr, count);
    }
    else
    {
        if (count > argumentCount)
        {
            reportException(Error_Incorrect_method_maxarg, this->argumentCount);
        }
        // This is the temporary list of arguments
        RexxObject * argument_list[7];
        if (count < argumentCount)              /* need to pad these out?            */
        {
            // null out the argument list
            memset(argument_list, 0, sizeof(argument_list));
            // and copy over the provided argument pointers
            memcpy(argument_list, argPtr, count * sizeof(RexxObject *));
            // now switch the argument pointer to the temporary
            argPtr = &argument_list[0];
        }
        // now we make the actual call
        switch (argumentCount)
        {
          case 0:                        /* zero                              */
            result = (receiver->*((PCPPM0)methodEntry))();
            break;

          case 1:
            result = (receiver->*((PCPPM1)methodEntry))(argPtr[0]);
            break;

          case 2:
            result = (receiver->*((PCPPM2)methodEntry))(argPtr[0], argPtr[1]);
            break;

          case 3:
            result = (receiver->*((PCPPM3)methodEntry))(argPtr[0], argPtr[1], argPtr[2]);
            break;

          case 4:
            result = (receiver->*((PCPPM4)methodEntry))(argPtr[0], argPtr[1], argPtr[2], argPtr[3]);
            break;

          case 5:
            result = (receiver->*((PCPPM5)methodEntry))(argPtr[0], argPtr[1], argPtr[2],
                argPtr[3], argPtr[4]);
            break;

          case 6:
            result = (receiver->*((PCPPM6)methodEntry))(argPtr[0], argPtr[1], argPtr[2],
                argPtr[3], argPtr[4], argPtr[5]);
            break;

          case 7:
            result = (receiver->*((PCPPM7)methodEntry))(argPtr[0], argPtr[1], argPtr[2],
                argPtr[3], argPtr[4], argPtr[5], argPtr[6]);
            break;
        }
    }
}


/**
 * Allocate a new attribute getter code object.
 *
 * @param size   the allocation size.
 *
 * @return A pointer to the newly allocated object.
 */
void *AttributeGetterCode::operator new(size_t size)
{
    // just allocate ane return
    return new_object(size, T_AttributeGetterCode);
}

void AttributeGetterCode::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  memory_mark(this->attribute);
}

void AttributeGetterCode::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  memory_mark_general(this->attribute);
}

void AttributeGetterCode::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(AttributeGetterCode)

  flatten_reference(newThis->attribute, envelope);

  cleanUpFlatten
}


/**
 * Execute an attribute-get operation.
 *
 * @param activity The current activity.
 * @param method   The method we're invoking.
 * @param receiver The receiver object.
 * @param messageName
 *                 The name of the message used to invoke the method.
 * @param argPtr   The pointer to the arguments.
 * @param count    The argument count.
 * @param result   The returned result.
 */
void AttributeGetterCode::run(RexxActivity *activity, RexxMethod *method, RexxObject *receiver, RexxString *messageName,
    RexxObject **argPtr, size_t count, ProtectedObject &result)
{
    // validate the number of arguments
    if (count > 0)
    {
        reportException(Error_Incorrect_method_maxarg, (wholenumber_t)0);
    }
    // this is simplier if the method is not guarded
    if (!method->isGuarded())
    {
        result = attribute->getValue(receiver->getObjectVariables(method->getScope()));
    }
    else {
        // get the variable pool and get the guard lock
        RexxVariableDictionary *objectVariables = receiver->getObjectVariables(method->getScope());
        objectVariables->reserve(activity);
        result = attribute->getValue(objectVariables);
        // and ensure we release this afterwards
        objectVariables->release(activity);
    }
}


/**
 * Allocate a new attribute setter code object.
 *
 * @param size   the allocation size.
 *
 * @return A pointer to the newly allocated object.
 */
void *AttributeSetterCode::operator new(size_t size)
{
    // just allocate ane return
    return new_object(size, T_AttributeSetterCode);
}


/**
 * Execute an attribute-set operation.
 *
 * @param activity The current activity.
 * @param method   The method we're invoking.
 * @param receiver The receiver object.
 * @param messageName
 *                 The name of the message used to invoke the method.
 * @param argPtr   The pointer to the arguments.
 * @param count    The argument count.
 * @param result   The returned result.
 */
void AttributeSetterCode::run(RexxActivity *activity, RexxMethod *method, RexxObject *receiver, RexxString *messageName,
    RexxObject **argPtr, size_t count, ProtectedObject &result)
{
    // validate the number of arguments
    if (count > 1)
    {
        reportException(Error_Incorrect_method_maxarg, 1);
    }

    if (count == 0 || *argPtr == OREF_NULL)
    {
        missingArgument(1);
    }
    // this is simplier if the method is not guarded
    if (!method->isGuarded())
    {
        // go set the attribue
        attribute->set(receiver->getObjectVariables(method->getScope()), argPtr[0]);
    }
    else {
        // get the variable pool and get the guard lock
        RexxVariableDictionary *objectVariables = receiver->getObjectVariables(method->getScope());
        objectVariables->reserve(activity);
        // go set the attribue
        attribute->set(objectVariables, argPtr[0]);
        // and ensure we release this afterwards
        objectVariables->release(activity);
    }
}


/**
 * Allocate a new constant getter code object.
 *
 * @param size   the allocation size.
 *
 * @return A pointer to the newly allocated object.
 */
void *ConstantGetterCode::operator new(size_t size)
{
    // just allocate ane return
    return new_object(size, T_AttributeGetterCode);
}


void ConstantGetterCode::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  memory_mark(this->constantValue);
}

void ConstantGetterCode::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  memory_mark_general(this->constantValue);
}

void ConstantGetterCode::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(ConstantGetterCode)

  flatten_reference(newThis->constantValue, envelope);

  cleanUpFlatten
}


/**
 * Execute a constant get operation
 *
 * @param activity The current activity.
 * @param method   The method we're invoking.
 * @param receiver The receiver object.
 * @param messageName
 *                 The name of the message used to invoke the method.
 * @param argPtr   The pointer to the arguments.
 * @param count    The argument count.
 * @param result   The returned result.
 */
void ConstantGetterCode::run(RexxActivity *activity, RexxMethod *method, RexxObject *receiver, RexxString *messageName,
    RexxObject **argPtr, size_t count, ProtectedObject &result)
{
    // validate the number of arguments
    if (count > 0)
    {
        reportException(Error_Incorrect_method_maxarg, (wholenumber_t)0);
    }
    result = constantValue;
}


/**
 * Allocate a new abstract code object.
 *
 * @param size   the allocation size.
 *
 * @return A pointer to the newly allocated object.
 */
void *AbstractCode::operator new(size_t size)
{
    // just allocate ane return
    return new_object(size, T_AbstractCode);
}


/**
 * Execute a constant get operation
 *
 * @param activity The current activity.
 * @param method   The method we're invoking.
 * @param receiver The receiver object.
 * @param messageName
 *                 The name of the message used to invoke the method.
 * @param argPtr   The pointer to the arguments.
 * @param count    The argument count.
 * @param result   The returned result.
 */
void AbstractCode::run(RexxActivity *activity, RexxMethod *method, RexxObject *receiver, RexxString *messageName,
    RexxObject **argPtr, size_t count, ProtectedObject &result)
{
    reportException(Error_Incorrect_method_abstract, messageName);
}


#include "RexxCore.h"
#include "TableClass.hpp"
#include "RexxMemory.hpp"
#include "RexxBehaviour.hpp"
#include "ClassClass.hpp"
#include "NumberStringClass.hpp"
#include "IntegerClass.hpp"
#include "StringClass.hpp"
#include "MutableBufferClass.hpp"
#include "ArrayClass.hpp"
#include "DirectoryClass.hpp"
#include "RelationClass.hpp"
#include "ListClass.hpp"
#include "QueueClass.hpp"
#include "SupplierClass.hpp"
#include "MethodClass.hpp"
#include "RoutineClass.hpp"
#include "PackageClass.hpp"
#include "RexxEnvelope.hpp"
#include "MessageClass.hpp"
#include "StemClass.hpp"
#include "RexxMisc.hpp"
#include "RexxNativeCode.hpp"
#include "RexxActivity.hpp"
#include "ActivityManager.hpp"
#include "RexxNativeActivation.hpp"
#include "RexxVariableDictionary.hpp"
#include "ExpressionVariable.hpp"
#include "RexxLocalVariables.hpp"
#include "ProtectedObject.hpp"
#include "PointerClass.hpp"
#include "BufferClass.hpp"
#include "WeakReferenceClass.hpp"
#include "ContextClass.hpp"
#include "StackFrameClass.hpp"

PCPPM CPPCode::exportedMethods[] =     /* start of exported methods table   */
{
CPPM(RexxObject::objectName),
CPPM(RexxObject::objectNameEquals),
CPPM(RexxObject::run),
CPPM(RexxObject::stringRexx),
CPPM(RexxObject::notEqual),
CPPM(RexxObject::setMethod),
CPPM(RexxObject::hasMethodRexx),
CPPM(RexxObject::start),
CPPM(RexxObject::startWith),
CPPM(RexxObject::send),
CPPM(RexxObject::sendWith),
CPPM(RexxObject::unsetMethod),
CPPM(RexxObject::requestRexx),
CPPM(RexxObject::makeStringRexx),
CPPM(RexxObject::makeArrayRexx),
CPPM(RexxInternalObject::hasUninit),
CPPM(RexxObject::classObject),
CPPM(RexxObject::equal),
CPPM(RexxObject::strictEqual),
CPPM(RexxObject::hashCode),
CPPM(RexxObject::init),
CPPM(RexxObject::strictNotEqual),
CPPM(RexxObject::copyRexx),
CPPM(RexxObject::defaultNameRexx),
CPPM(RexxObject::unknownRexx),
CPPM(RexxObject::isInstanceOfRexx),
CPPM(RexxObject::instanceMethodRexx),
CPPM(RexxObject::instanceMethodsRexx),
CPPM(RexxObject::identityHashRexx),
CPPM(RexxObject::concatRexx),
CPPM(RexxObject::concatBlank),

CPPM(RexxObject::newRexx),

CPPM(RexxClass::setRexxDefined),       /* Class methods                     */
CPPM(RexxClass::defaultNameRexx),
CPPM(RexxClass::queryMixinClass),
CPPM(RexxClass::getId),
CPPM(RexxClass::getBaseClass),
CPPM(RexxClass::getMetaClass),
CPPM(RexxClass::getSuperClasses),
CPPM(RexxClass::getSuperClass),
CPPM(RexxClass::getSubClasses),
CPPM(RexxClass::defmeths),
CPPM(RexxClass::defineMethod),
CPPM(RexxClass::defineMethods),
CPPM(RexxClass::deleteMethod),
CPPM(RexxClass::method),
CPPM(RexxClass::methods),
CPPM(RexxClass::inherit),
CPPM(RexxClass::uninherit),
CPPM(RexxClass::enhanced),
CPPM(RexxClass::mixinclass),
CPPM(RexxClass::subclass),
CPPM(RexxClass::equal),
CPPM(RexxClass::strictEqual),
CPPM(RexxClass::notEqual),
CPPM(RexxClass::isSubclassOf),

CPPM(RexxClass::newRexx),

CPPM(RexxArray::sizeRexx),             /* Array methods                     */
CPPM(RexxArray::itemsRexx),
CPPM(RexxArray::dimension),
CPPM(RexxArray::supplier),
CPPM(RexxArray::getRexx),
CPPM(RexxArray::putRexx),
CPPM(RexxArray::hasIndexRexx),
CPPM(RexxArray::sectionRexx),
CPPM(RexxArray::removeRexx),
CPPM(RexxArray::firstRexx),
CPPM(RexxArray::lastRexx),
CPPM(RexxArray::nextRexx),
CPPM(RexxArray::previousRexx),
CPPM(RexxArray::appendRexx),
CPPM(RexxArray::allIndexes),
CPPM(RexxArray::allItems),
CPPM(RexxArray::empty),
CPPM(RexxArray::isEmpty),
CPPM(RexxArray::index),
CPPM(RexxArray::hasItem),
CPPM(RexxArray::removeItem),
CPPM(RexxArray::toString),
CPPM(RexxArray::sortRexx),
CPPM(RexxArray::stableSortRexx),
CPPM(RexxArray::sortWithRexx),
CPPM(RexxArray::stableSortWithRexx),

CPPM(RexxArray::newRexx),
CPPM(RexxArray::makeString),
CPPM(RexxArray::of),

CPPM(RexxDirectory::atRexx),           /* Directory methods                 */
CPPM(RexxDirectory::put),
CPPM(RexxDirectory::entryRexx),
CPPM(RexxDirectory::hasEntry),
CPPM(RexxDirectory::hasIndex),
CPPM(RexxDirectory::itemsRexx),
CPPM(RexxHashTableCollection::merge),
CPPM(RexxDirectory::removeRexx),
CPPM(RexxDirectory::setEntry),
CPPM(RexxDirectory::setMethod),
CPPM(RexxDirectory::supplier),
CPPM(RexxDirectory::allIndexes),
CPPM(RexxDirectory::allItems),
CPPM(RexxDirectory::empty),
CPPM(RexxDirectory::isEmpty),
CPPM(RexxDirectory::indexRexx),
CPPM(RexxDirectory::hasItem),
CPPM(RexxDirectory::removeItem),

CPPM(RexxDirectory::newRexx),

CPPM(RexxInteger::plus),               /* Integer methods                   */
CPPM(RexxInteger::minus),
CPPM(RexxInteger::multiply),
CPPM(RexxInteger::divide),
CPPM(RexxInteger::integerDivide),
CPPM(RexxInteger::remainder),
CPPM(RexxInteger::power),
CPPM(RexxInteger::notOp),
CPPM(RexxInteger::andOp),
CPPM(RexxInteger::orOp),
CPPM(RexxInteger::xorOp),
CPPM(RexxInteger::equal),
CPPM(RexxInteger::notEqual),
CPPM(RexxInteger::strictEqual),
CPPM(RexxInteger::hashCode),
CPPM(RexxInteger::strictNotEqual),
CPPM(RexxInteger::isGreaterThan),
CPPM(RexxInteger::isLessThan),
CPPM(RexxInteger::isGreaterOrEqual),
CPPM(RexxInteger::isLessOrEqual),
CPPM(RexxInteger::strictGreaterThan),
CPPM(RexxInteger::strictLessThan),
CPPM(RexxInteger::strictGreaterOrEqual),
CPPM(RexxInteger::strictLessOrEqual),
CPPM(RexxInteger::abs),
CPPM(RexxInteger::sign),
CPPM(RexxInteger::Max),
CPPM(RexxInteger::Min),
CPPM(RexxInteger::d2x),
CPPM(RexxInteger::d2c),
CPPM(RexxInteger::format),
CPPM(RexxInteger::trunc),

CPPM(RexxList::value),                 /* list methods                      */
CPPM(RexxList::remove),
CPPM(RexxList::firstRexx),
CPPM(RexxList::lastRexx),
CPPM(RexxList::next),
CPPM(RexxList::previous),
CPPM(RexxList::hasIndex),
CPPM(RexxList::supplier),
CPPM(RexxList::itemsRexx),
CPPM(RexxList::put),
CPPM(RexxList::section),
CPPM(RexxList::firstItem),
CPPM(RexxList::lastItem),
CPPM(RexxList::insertRexx),
CPPM(RexxList::append),
CPPM(RexxList::allIndexes),
CPPM(RexxList::allItems),
CPPM(RexxList::empty),
CPPM(RexxList::isEmpty),
CPPM(RexxList::index),
CPPM(RexxList::hasItem),
CPPM(RexxList::removeItem),

CPPM(RexxList::newRexx),
CPPM(RexxList::classOf),

CPPM(RexxMessage::notify),             /* Message methods                   */
CPPM(RexxMessage::result),
CPPM(RexxMessage::send),
CPPM(RexxMessage::start),
CPPM(RexxMessage::completed),
CPPM(RexxMessage::hasError),
CPPM(RexxMessage::errorCondition),
CPPM(RexxMessage::messageTarget),
CPPM(RexxMessage::messageName),
CPPM(RexxMessage::arguments),

CPPM(RexxMessage::newRexx),

CPPM(RexxMethod::setUnguardedRexx),    /* Method methods                    */
CPPM(RexxMethod::setGuardedRexx),
CPPM(BaseExecutable::source),
CPPM(BaseExecutable::getPackage),
CPPM(RexxMethod::setPrivateRexx),
CPPM(RexxMethod::setProtectedRexx),
CPPM(RexxMethod::setSecurityManager),
CPPM(RexxMethod::isGuardedRexx),
CPPM(RexxMethod::isPrivateRexx),
CPPM(RexxMethod::isProtectedRexx),

CPPM(RexxMethod::newFileRexx),
CPPM(RexxMethod::newRexx),
CPPM(RexxMethod::loadExternalMethod),

CPPM(RoutineClass::setSecurityManager),
CPPM(RoutineClass::callRexx),
CPPM(RoutineClass::callWithRexx),

CPPM(RoutineClass::newFileRexx),
CPPM(RoutineClass::newRexx),
CPPM(RoutineClass::loadExternalRoutine),

CPPM(PackageClass::setSecurityManager),
CPPM(PackageClass::getSource),
CPPM(PackageClass::getSourceLineRexx),
CPPM(PackageClass::getSourceSize),
CPPM(PackageClass::getClasses),
CPPM(PackageClass::getPublicClasses),
CPPM(PackageClass::getImportedClasses),
CPPM(PackageClass::getMethods),
CPPM(PackageClass::getRoutines),
CPPM(PackageClass::getPublicRoutines),
CPPM(PackageClass::getImportedRoutines),
CPPM(PackageClass::getImportedPackages),
CPPM(PackageClass::loadPackage),
CPPM(PackageClass::addPackage),
CPPM(PackageClass::findClassRexx),
CPPM(PackageClass::findRoutineRexx),
CPPM(PackageClass::addRoutine),
CPPM(PackageClass::addPublicRoutine),
CPPM(PackageClass::addClass),
CPPM(PackageClass::addPublicClass),
CPPM(PackageClass::getName),
CPPM(PackageClass::loadLibrary),
CPPM(PackageClass::digits),
CPPM(PackageClass::form),
CPPM(PackageClass::fuzz),
CPPM(PackageClass::trace),

CPPM(PackageClass::newRexx),

CPPM(RexxNumberString::formatRexx),    /* NumberString methods              */
CPPM(RexxNumberString::trunc),
CPPM(RexxNumberString::equal),
CPPM(RexxNumberString::notEqual),
CPPM(RexxNumberString::isLessThan),
CPPM(RexxNumberString::isGreaterThan),
CPPM(RexxNumberString::isGreaterOrEqual),
CPPM(RexxNumberString::isLessOrEqual),
CPPM(RexxNumberString::strictNotEqual),
CPPM(RexxNumberString::strictLessThan),
CPPM(RexxNumberString::strictGreaterThan),
CPPM(RexxNumberString::strictGreaterOrEqual),
CPPM(RexxNumberString::strictLessOrEqual),
CPPM(RexxNumberString::plus),
CPPM(RexxNumberString::minus),
CPPM(RexxNumberString::multiply),
CPPM(RexxNumberString::divide),
CPPM(RexxNumberString::integerDivide),
CPPM(RexxNumberString::remainder),
CPPM(RexxNumberString::power),
CPPM(RexxNumberString::abs),
CPPM(RexxNumberString::Sign),
CPPM(RexxNumberString::notOp),
CPPM(RexxNumberString::andOp),
CPPM(RexxNumberString::orOp),
CPPM(RexxNumberString::xorOp),
CPPM(RexxNumberString::Max),
CPPM(RexxNumberString::Min),
CPPM(RexxNumberString::isInteger),
CPPM(RexxNumberString::d2c),
CPPM(RexxNumberString::d2x),
CPPM(RexxNumberString::d2xD2c),
CPPM(RexxNumberString::strictEqual),
CPPM(RexxNumberString::hashCode),

CPPM(RexxQueue::supplier),             /* Queue methods                     */
CPPM(RexxQueue::pushRexx),
CPPM(RexxQueue::queueRexx),
CPPM(RexxQueue::pullRexx),
CPPM(RexxQueue::peek),
CPPM(RexxQueue::put),
CPPM(RexxQueue::at),
CPPM(RexxQueue::hasindex),
CPPM(RexxQueue::remove),
CPPM(RexxQueue::append),
CPPM(RexxQueue::allIndexes),
CPPM(RexxQueue::index),
CPPM(RexxQueue::firstRexx),
CPPM(RexxQueue::lastRexx),
CPPM(RexxQueue::next),
CPPM(RexxQueue::previous),
CPPM(RexxQueue::insert),

CPPM(RexxQueue::newRexx),
CPPM(RexxQueue::ofRexx),

CPPM(RexxStem::bracket),               /* Stem methods                      */
CPPM(RexxStem::bracketEqual),
CPPM(RexxStem::request),
CPPM(RexxStem::supplier),
CPPM(RexxStem::allIndexes),
CPPM(RexxStem::allItems),
CPPM(RexxStem::empty),
CPPM(RexxStem::isEmpty),
CPPM(RexxStem::itemsRexx),
CPPM(RexxStem::hasIndex),
CPPM(RexxStem::remove),
CPPM(RexxStem::index),
CPPM(RexxStem::hasItem),
CPPM(RexxStem::removeItem),
CPPM(RexxStem::toDirectory),

CPPM(RexxStem::newRexx),

CPPM(RexxString::lengthRexx),          /* String methods                    */
CPPM(RexxString::concatRexx),
CPPM(RexxString::concatBlank),
CPPM(RexxString::concatWith),
CPPM(RexxString::equal),
CPPM(RexxString::notEqual),
CPPM(RexxString::isLessThan),
CPPM(RexxString::isGreaterThan),
CPPM(RexxString::isGreaterOrEqual),
CPPM(RexxString::isLessOrEqual),
CPPM(RexxString::strictEqual),
CPPM(RexxString::strictNotEqual),
CPPM(RexxString::strictLessThan),
CPPM(RexxString::strictGreaterThan),
CPPM(RexxString::strictGreaterOrEqual),
CPPM(RexxString::strictLessOrEqual),
CPPM(RexxString::plus),
CPPM(RexxString::minus),
CPPM(RexxString::multiply),
CPPM(RexxString::divide),
CPPM(RexxString::integerDivide),
CPPM(RexxString::remainder),
CPPM(RexxString::power),
CPPM(RexxString::abs),
CPPM(RexxString::sign),
CPPM(RexxString::notOp),
CPPM(RexxString::andOp),
CPPM(RexxString::orOp),
CPPM(RexxString::xorOp),
CPPM(RexxString::Max),
CPPM(RexxString::Min),
CPPM(RexxString::isInteger),
CPPM(RexxString::upperRexx),
CPPM(RexxString::lowerRexx),

                                          /* All BIF methods start here.  They */
                                          /*  will be arranged according to the*/
                                          /*  they are defined in.             */

                                          /* following methods are in OKBSUBS  */
CPPM(RexxString::center),
CPPM(RexxString::delstr),
CPPM(RexxString::insert),
CPPM(RexxString::left),
CPPM(RexxString::overlay),
CPPM(RexxString::reverse),
CPPM(RexxString::right),
CPPM(RexxString::strip),
CPPM(RexxString::substr),
CPPM(RexxString::subchar),
CPPM(RexxString::replaceAt),

                                          /* following methods are in OKBWORD  */
CPPM(RexxString::delWord),
CPPM(RexxString::space),
CPPM(RexxString::subWord),
CPPM(RexxString::word),
CPPM(RexxString::wordIndex),
CPPM(RexxString::wordLength),
CPPM(RexxString::wordPos),
CPPM(RexxString::caselessWordPos),
CPPM(RexxString::words),

                                          /* following methods are in OKBMISC  */

CPPM(RexxString::changeStr),
CPPM(RexxString::caselessChangeStr),
CPPM(RexxString::countStrRexx),
CPPM(RexxString::caselessCountStrRexx),
CPPM(RexxString::abbrev),
CPPM(RexxString::caselessAbbrev),
CPPM(RexxString::compare),
CPPM(RexxString::caselessCompare),
CPPM(RexxString::copies),
CPPM(RexxString::dataType),
CPPM(RexxString::lastPosRexx),
CPPM(RexxString::posRexx),
CPPM(RexxString::caselessLastPosRexx),
CPPM(RexxString::caselessPosRexx),
CPPM(RexxString::translate),
CPPM(RexxString::verify),

                                          /* following methods are in OKBBITS  */
CPPM(RexxString::bitAnd),
CPPM(RexxString::bitOr),
CPPM(RexxString::bitXor),

                                          /* following methods are in OKBCONV  */

CPPM(RexxString::b2x),
CPPM(RexxString::c2d),
CPPM(RexxString::c2x),
CPPM(RexxString::d2c),
CPPM(RexxString::d2x),
CPPM(RexxString::x2b),
CPPM(RexxString::x2c),
CPPM(RexxString::x2d),
CPPM(RexxString::format),
CPPM(RexxString::trunc),
CPPM(RexxString::x2dC2d),
CPPM(RexxString::encodeBase64),
CPPM(RexxString::decodeBase64),

CPPM(RexxString::match),
CPPM(RexxString::caselessMatch),
CPPM(RexxString::matchChar),
CPPM(RexxString::caselessMatchChar),
CPPM(RexxString::equals),
CPPM(RexxString::caselessEquals),
CPPM(RexxString::compareToRexx),
CPPM(RexxString::caselessCompareToRexx),
                                          /* End of BIF methods                */
CPPM(RexxString::makeArray),

CPPM(RexxString::newRexx),
CPPM(RexxMutableBufferClass::newRexx),
CPPM(RexxMutableBuffer::lengthRexx),
CPPM(RexxMutableBuffer::makearray),
CPPM(RexxMutableBuffer::append),
CPPM(RexxMutableBuffer::insert),
CPPM(RexxMutableBuffer::overlay),
CPPM(RexxMutableBuffer::mydelete),
CPPM(RexxMutableBuffer::substr),
CPPM(RexxMutableBuffer::subchar),
CPPM(RexxMutableBuffer::posRexx),
CPPM(RexxMutableBuffer::lastPos),
CPPM(RexxMutableBuffer::caselessPos),
CPPM(RexxMutableBuffer::caselessLastPos),
CPPM(RexxMutableBuffer::getBufferSize),
CPPM(RexxMutableBuffer::setBufferSize),
CPPM(RexxMutableBuffer::replaceAt),
CPPM(RexxMutableBuffer::countStrRexx),
CPPM(RexxMutableBuffer::caselessCountStrRexx),
CPPM(RexxMutableBuffer::changeStr),
CPPM(RexxMutableBuffer::caselessChangeStr),
CPPM(RexxMutableBuffer::upper),
CPPM(RexxMutableBuffer::lower),
CPPM(RexxMutableBuffer::translate),
CPPM(RexxMutableBuffer::match),
CPPM(RexxMutableBuffer::caselessMatch),
CPPM(RexxMutableBuffer::matchChar),
CPPM(RexxMutableBuffer::caselessMatchChar),
CPPM(RexxMutableBuffer::verify),
CPPM(RexxMutableBuffer::subWord),
CPPM(RexxMutableBuffer::word),
CPPM(RexxMutableBuffer::wordIndex),
CPPM(RexxMutableBuffer::wordLength),
CPPM(RexxMutableBuffer::words),
CPPM(RexxMutableBuffer::wordPos),
CPPM(RexxMutableBuffer::caselessWordPos),
CPPM(RexxMutableBuffer::delWord),

CPPM(RexxSupplier::available),         /* Supplier methods                  */
CPPM(RexxSupplier::next),
CPPM(RexxSupplier::value),
CPPM(RexxSupplier::index),
CPPM(RexxSupplier::initRexx),

CPPM(RexxSupplierClass::newRexx),

                                       /* Table methods                     */
CPPM(RexxHashTableCollection::removeRexx),
CPPM(RexxHashTableCollection::getRexx),
CPPM(RexxHashTableCollection::putRexx),
CPPM(RexxHashTableCollection::addRexx),
CPPM(RexxHashTableCollection::allAt),
CPPM(RexxHashTableCollection::hasIndexRexx),
CPPM(RexxHashTableCollection::merge),
CPPM(RexxHashTableCollection::supplier),
CPPM(RexxHashTableCollection::allItems),
CPPM(RexxHashTableCollection::allIndexes),
CPPM(RexxHashTableCollection::empty),
CPPM(RexxHashTableCollection::isEmpty),
CPPM(RexxHashTableCollection::indexRexx),
CPPM(RexxHashTableCollection::hasItemRexx),
CPPM(RexxHashTableCollection::removeItemRexx),

CPPM(RexxTable::itemsRexx),
CPPM(RexxTable::newRexx),

CPPM(RexxIdentityTable::newRexx),

CPPM(RexxRelation::put),               /* Relation methods                  */
CPPM(RexxRelation::removeItemRexx),
CPPM(RexxRelation::allIndex),
CPPM(RexxRelation::itemsRexx),
CPPM(RexxRelation::supplier),
CPPM(RexxRelation::hasItem),

CPPM(RexxRelation::newRexx),

CPPM(RexxLocal::local),                /* the .local environment methods    */

CPPM(RexxPointer::equal),
CPPM(RexxPointer::notEqual),
CPPM(RexxPointer::newRexx),
CPPM(RexxPointer::isNull),

CPPM(RexxBuffer::newRexx),

CPPM(WeakReference::newRexx),
CPPM(WeakReference::value),

CPPM(RexxContext::newRexx),
CPPM(RexxContext::copyRexx),
CPPM(RexxContext::getPackage),
CPPM(RexxContext::getDigits),
CPPM(RexxContext::getFuzz),
CPPM(RexxContext::getForm),
CPPM(RexxContext::getVariables),
CPPM(RexxContext::getExecutable),
CPPM(RexxContext::getArgs),
CPPM(RexxContext::getCondition),
CPPM(RexxContext::getLine),
CPPM(RexxContext::getRS),

CPPM(StackFrameClass::getName),
CPPM(StackFrameClass::getExecutable),
CPPM(StackFrameClass::getLine),
CPPM(StackFrameClass::getTraceLine),
CPPM(StackFrameClass::getType),
CPPM(StackFrameClass::newRexx),
NULL                                   /* final terminating method          */
};


/**
 * Resolve the entry point of a CPP method into a CPPCode wrapper
 * for that method.
 *
 * @param name   The name of the method (used for error reporting)
 * @param targetMethod
 *               The method to wrapper
 * @param argumentCount
 *               The argument descriptor.
 *
 * @return A CPPCode object for the wrappered method.
 */
CPPCode *CPPCode::resolveExportedMethod(const char *name, PCPPM targetMethod, size_t argumentCount)
{
    for (size_t i = 0; exportedMethods[i] != NULL; i++)
    {
        // found the one we need?  Wrap a CPPCode object around it
        if (exportedMethods[i] == targetMethod)
        {
            return new CPPCode(i, targetMethod, argumentCount);
        }
    }

    char buffer[256];
    sprintf(buffer,"Unresolved exported method:  %s", name);
    /* this is a bad error               */
    Interpreter::logicError(buffer);
    return NULL;                         /* needs a return value              */
}
