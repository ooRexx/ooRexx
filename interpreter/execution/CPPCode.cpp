/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2025 Rexx Language Association. All rights reserved.    */
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

#include "RexxCore.h"
#include "CPPCode.hpp"
#include "ActivityManager.hpp"
#include "ProtectedObject.hpp"
#include "Interpreter.hpp"
#include "VariableDictionary.hpp"
#include "ActivationFrame.hpp"
#include "ExpressionBaseVariable.hpp"
#include "MethodArguments.hpp"
#include "PackageClass.hpp"
#include "VariableReference.hpp"
#include "EventSemaphore.hpp"
#include "MutexSemaphore.hpp"

#include <stdio.h>


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


/**
 * Normal garbage collection live marking
 *
 * @param liveMark The current live mark.
 */
void CPPCode::live(size_t liveMark)
{
    memory_mark(package);
}


/**
 * Generalized object marking.  If restoring or unflattening,
 * make sure we restore the method pointer.
 *
 * @param reason The reason for the call.
 */
void CPPCode::liveGeneral(MarkReason reason)
{
    // if we're getting ready to save the image, replace the source
    // package with the global REXX package
    if (reason == PREPARINGIMAGE)
    {
        package = TheRexxPackage;
    }
    else if (reason == SAVINGIMAGE)
    {
        // if we're saving the image, zero the pointer value out so that this creates
        // consistently repeatable build.
        cppEntry = NULL;
    }
    else if (reason == RESTORINGIMAGE || reason == UNFLATTENINGOBJECT)
    {
        cppEntry = exportedMethods[methodIndex];
    }

    memory_mark_general(package);
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
void CPPCode::run(Activity *activity, MethodClass *method, RexxObject *receiver, RexxString *messageName,
                  RexxObject **argPtr, size_t count, ProtectedObject &result)
{
    InternalActivationFrame frame(activity, messageName, receiver, method, argPtr, count);
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
        RexxObject *argument_list[7];
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
void* AttributeGetterCode::operator new(size_t size)
{
    return new_object(size, T_AttributeGetterCode);
}


/**
 * Normal garbage collection live marking
 *
 * @param liveMark The current live mark.
 */
void AttributeGetterCode::live(size_t liveMark)
{
    memory_mark(package);
    memory_mark(attribute);
}


/**
 * Generalized object marking.
 *
 * @param reason The reason for this live marking operation.
 */
void AttributeGetterCode::liveGeneral(MarkReason reason)
{
    // if we're getting ready to save the image, replace the source
    // package with the global REXX package
    if (reason == PREPARINGIMAGE)
    {
        package = TheRexxPackage;
    }

    memory_mark_general(package);
    memory_mark_general(attribute);
}


/**
 * Flatten the table contents as part of a saved program.
 *
 * @param envelope The envelope we're flattening into.
 */
void AttributeGetterCode::flatten(Envelope *envelope)
{
    setUpFlatten(AttributeGetterCode)

    flattenRef(package);
    flattenRef(attribute);

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
void AttributeGetterCode::run(Activity *activity, MethodClass *method, RexxObject *receiver, RexxString *messageName,
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
    else
    {
        // get the variable pool and get the guard lock
        VariableDictionary *objectVariables = receiver->getObjectVariables(method->getScope());
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
void* AttributeSetterCode::operator new(size_t size)
{
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
void AttributeSetterCode::run(Activity *activity, MethodClass *method, RexxObject *receiver, RexxString *messageName,
                              RexxObject **argPtr, size_t count, ProtectedObject &result)
{
    // validate the number of arguments
    if (count > 1)
    {
        reportException(Error_Incorrect_method_maxarg, 1);
    }

    if (count == 0 || *argPtr == OREF_NULL)
    {
        missingArgument(ARG_ONE);
    }

    // this is simplier if the method is not guarded
    if (!method->isGuarded())
    {
        // go set the attribue
        attribute->set(receiver->getObjectVariables(method->getScope()), argPtr[0]);
    }
    else
    {
        // get the variable pool and get the guard lock
        VariableDictionary *objectVariables = receiver->getObjectVariables(method->getScope());
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
void* ConstantGetterCode::operator new(size_t size)
{
    return new_object(size, T_ConstantGetterCode);
}


/**
 * Normal garbage collection live marking
 *
 * @param liveMark The current live mark.
 */
void ConstantGetterCode::live(size_t liveMark)
{
    memory_mark(package);
    memory_mark(constantValue);
    memory_mark(constantName);
}


/**
 * Generalized object marking.
 *
 * @param reason The reason for this live marking operation.
 */
void ConstantGetterCode::liveGeneral(MarkReason reason)
{
    // if we're getting ready to save the image, replace the source
    // package with the global REXX package
    if (reason == PREPARINGIMAGE)
    {
        package = TheRexxPackage;
    }

    memory_mark_general(package);
    memory_mark_general(constantValue);
    memory_mark_general(constantName);
}


/**
 * Flatten the table contents as part of a saved program.
 *
 * @param envelope The envelope we're flattening into.
 */
void ConstantGetterCode::flatten(Envelope *envelope)
{
    setUpFlatten(ConstantGetterCode)

    flattenRef(package);
    flattenRef(constantValue);
    flattenRef(constantName);

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
void ConstantGetterCode::run(Activity *activity, MethodClass *method, RexxObject *receiver, RexxString *messageName,
                             RexxObject **argPtr, size_t count, ProtectedObject &result)
{
    // validate the number of arguments
    if (count > 0)
    {
        reportException(Error_Incorrect_method_maxarg, (wholenumber_t)0);
    }
    // if this constant is initialized via a dynamic expression, it is possible that the
    // initialization never completed. Treat this as a NO METHOD situation, but with a special error code.
    if (constantValue == OREF_NULL)
    {
        reportNomethod(Error_No_method_constant, constantName, receiver);
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
void* AbstractCode::operator new(size_t size)
{
    return new_object(size, T_AbstractCode);
}


/**
 * Normal garbage collection live marking
 *
 * @param liveMark The current live mark.
 */
void AbstractCode::live(size_t liveMark)
{
    memory_mark(package);
}


/**
 * Generalized object marking.  If restoring or unflattening,
 * make sure we restore the method pointer.
 *
 * @param reason The reason for the call.
 */
void AbstractCode::liveGeneral(MarkReason reason)
{
    // if we're getting ready to save the image, replace the source
    // package with the global REXX package
    if (reason == PREPARINGIMAGE)
    {
        package = TheRexxPackage;
    }
    memory_mark_general(package);
}


/**
 * Flatten an AbstractCode object.
 *
 * @param envelope the target envelope we're flatting in.
 */
void AbstractCode::flatten(Envelope *envelope)
{
    setUpFlatten(AbstractCode)

    flattenRef(package);

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
void AbstractCode::run(Activity *activity, MethodClass *method, RexxObject *receiver, RexxString *messageName,
                       RexxObject **argPtr, size_t count, ProtectedObject &result)
{
    reportException(Error_Incorrect_method_abstract, messageName);
}


/**
 * Allocate a new attribute getter code object.
 *
 * @param size   the allocation size.
 *
 * @return A pointer to the newly allocated object.
 */
void* DelegateCode::operator new(size_t size)
{
    return new_object(size, T_DelegateCode);
}


/**
 * Normal garbage collection live marking
 *
 * @param liveMark The current live mark.
 */
void DelegateCode::live(size_t liveMark)
{
    memory_mark(package);
    memory_mark(attribute);
}


/**
 * Generalized object marking.
 *
 * @param reason The reason for this live marking operation.
 */
void DelegateCode::liveGeneral(MarkReason reason)
{
    // if we're getting ready to save the image, replace the source
    // package with the global REXX package
    if (reason == PREPARINGIMAGE)
    {
        package = TheRexxPackage;
    }

    memory_mark_general(package);
    memory_mark_general(attribute);
}


/**
 * Flatten the table contents as part of a saved program.
 *
 * @param envelope The envelope we're flattening into.
 */
void DelegateCode::flatten(Envelope *envelope)
{
    setUpFlatten(DelegateCode)

    flattenRef(package);
    flattenRef(attribute);

    cleanUpFlatten
}


/**
 * Execute a Delegate method forward operation.
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
void DelegateCode::run(Activity *activity, MethodClass *method, RexxObject *receiver, RexxString *messageName,
    RexxObject **argPtr, size_t count, ProtectedObject &result)
{
    // get the variable pool and get
    VariableDictionary *objectVariables = receiver->getObjectVariables(method->getScope());
    RexxObject *target;

    // if this is a guarded method, we grab the guard only long enough to
    // get the target variable.  The message send is sent without holding the lock
    if (method->isGuarded())
    {
        objectVariables->reserve(activity);
        target = attribute->getValue(receiver->getObjectVariables(method->getScope()));
        // and ensure we release this afterwards
        objectVariables->release(activity);
    }
    else
    {
        target = attribute->getValue(receiver->getObjectVariables(method->getScope()));
    }

    // and finally, send the message to the resolved target
    target->sendMessage(messageName, argPtr, count, result);
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
#include "Envelope.hpp"
#include "MessageClass.hpp"
#include "StemClass.hpp"
#include "NativeCode.hpp"
#include "Activity.hpp"
#include "ActivityManager.hpp"
#include "NativeActivation.hpp"
#include "VariableDictionary.hpp"
#include "ExpressionVariable.hpp"
#include "RexxLocalVariables.hpp"
#include "ProtectedObject.hpp"
#include "PointerClass.hpp"
#include "BufferClass.hpp"
#include "WeakReferenceClass.hpp"
#include "ContextClass.hpp"
#include "StackFrameClass.hpp"
#include "ActivityManager.hpp"
#include "SetClass.hpp"
#include "BagClass.hpp"
#include "RexxInfoClass.hpp"

// start of the exported methods table.  Any method used in Setup.cpp must also
// be included in this table here so that methods can recover the pointer
// to the backing code after being restored.
PCPPM CPPCode::exportedMethods[] =
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
    CPPM(RexxObject::classObject),
    CPPM(RexxObject::equal),
    CPPM(RexxObject::strictEqual),
    CPPM(RexxObject::hashCode),
    CPPM(RexxObject::initRexx),
    CPPM(RexxObject::strictNotEqual),
    CPPM(RexxObject::copyRexx),
    CPPM(RexxObject::defaultNameRexx),
    CPPM(RexxObject::isInstanceOfRexx),
    CPPM(RexxObject::isNilRexx),
    CPPM(RexxObject::instanceMethodRexx),
    CPPM(RexxObject::instanceMethodsRexx),
    CPPM(RexxObject::identityHashRexx),
    CPPM(RexxObject::concatRexx),
    CPPM(RexxObject::concatBlank),

    CPPM(RexxObject::newRexx),

    CPPM(RexxClass::inheritInstanceMethodsRexx),
    CPPM(RexxClass::defaultNameRexx),
    CPPM(RexxClass::queryMixinClass),
    CPPM(RexxClass::isMetaClassRexx),
    CPPM(RexxClass::isAbstractRexx),
    CPPM(RexxClass::getId),
    CPPM(RexxClass::getBaseClass),
    CPPM(RexxClass::getMetaClass),
    CPPM(RexxClass::getSuperClasses),
    CPPM(RexxClass::getSuperClass),
    CPPM(RexxClass::getSubClasses),
    CPPM(RexxClass::defineMethodsRexx),
    CPPM(RexxClass::defineMethod),
    CPPM(RexxClass::defineMethods),
    CPPM(RexxClass::defineClassMethod),
    CPPM(RexxClass::removeClassMethod),
    CPPM(RexxClass::deleteMethod),
    CPPM(RexxClass::method),
    CPPM(RexxClass::methods),
    CPPM(RexxClass::inherit),
    CPPM(RexxClass::uninherit),
    CPPM(RexxClass::enhanced),
    CPPM(RexxClass::mixinClassRexx),
    CPPM(RexxClass::subclassRexx),
    CPPM(RexxClass::equal),
    CPPM(RexxClass::strictEqual),
    CPPM(RexxClass::notEqual),
    CPPM(RexxClass::isSubclassOf),
    CPPM(RexxClass::getPackage),
    CPPM(RexxClass::getAnnotations),
    CPPM(RexxClass::getAnnotationRexx),
    CPPM(RexxClass::copyRexx),

    CPPM(RexxClass::newRexx),

    CPPM(ArrayClass::sizeRexx),
    CPPM(ArrayClass::itemsRexx),
    CPPM(ArrayClass::dimensionRexx),
    CPPM(ArrayClass::getDimensionsRexx),
    CPPM(ArrayClass::supplier),
    CPPM(ArrayClass::getRexx),
    CPPM(ArrayClass::putRexx),
    CPPM(ArrayClass::hasIndexRexx),
    CPPM(ArrayClass::sectionRexx),
    CPPM(ArrayClass::removeRexx),
    CPPM(ArrayClass::firstRexx),
    CPPM(ArrayClass::getFirstItem),
    CPPM(ArrayClass::lastRexx),
    CPPM(ArrayClass::getLastItem),
    CPPM(ArrayClass::nextRexx),
    CPPM(ArrayClass::previousRexx),
    CPPM(ArrayClass::appendRexx),
    CPPM(ArrayClass::allIndexes),
    CPPM(ArrayClass::allItems),
    CPPM(ArrayClass::empty),
    CPPM(ArrayClass::isEmptyRexx),
    CPPM(ArrayClass::indexRexx),
    CPPM(ArrayClass::hasItemRexx),
    CPPM(ArrayClass::removeItemRexx),
    CPPM(ArrayClass::toString),
    CPPM(ArrayClass::stableSortRexx),
    CPPM(ArrayClass::stableSortWithRexx),
    CPPM(ArrayClass::insertRexx),
    CPPM(ArrayClass::deleteRexx),
    CPPM(ArrayClass::fillRexx),

    CPPM(ArrayClass::newRexx),
    CPPM(ArrayClass::ofRexx),

    CPPM(RexxInteger::plus),
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
    CPPM(RexxInteger::choiceRexx),
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
    CPPM(RexxInteger::modulo),
    CPPM(RexxInteger::floor),
    CPPM(RexxInteger::ceiling),
    CPPM(RexxInteger::round),
    CPPM(RexxInteger::classObject),

    CPPM(ListClass::initRexx),
    CPPM(ListClass::getRexx),
    CPPM(ListClass::putRexx),
    CPPM(ListClass::getRexx),
    CPPM(ListClass::firstItemRexx),
    CPPM(ListClass::hasIndexRexx),
    CPPM(ListClass::insertRexx),
    CPPM(ListClass::itemsRexx),
    CPPM(ListClass::lastItemRexx),
    CPPM(ListClass::firstRexx),
    CPPM(ListClass::lastRexx),
    CPPM(ListClass::nextRexx),
    CPPM(ListClass::previousRexx),
    CPPM(ListClass::putRexx),
    CPPM(ListClass::removeRexx),
    CPPM(ListClass::removeRexx),
    CPPM(ListClass::sectionRexx),
    CPPM(ListClass::supplier),
    CPPM(ListClass::appendRexx),
    CPPM(ListClass::allItems),
    CPPM(ListClass::allIndexes),
    CPPM(ListClass::emptyRexx),
    CPPM(ListClass::isEmptyRexx),
    CPPM(ListClass::indexRexx),
    CPPM(ListClass::hasItemRexx),
    CPPM(ListClass::removeItemRexx),

    CPPM(ListClass::newRexx),
    CPPM(ListClass::ofRexx),

    CPPM(MessageClass::notify),
    CPPM(MessageClass::result),
    CPPM(MessageClass::sendRexx),
    CPPM(MessageClass::startRexx),
    CPPM(MessageClass::replyRexx),
    CPPM(MessageClass::sendWithRexx),
    CPPM(MessageClass::startWithRexx),
    CPPM(MessageClass::replyWithRexx),
    CPPM(MessageClass::completed),
    CPPM(MessageClass::hasError),
    CPPM(MessageClass::hasResult),
    CPPM(MessageClass::errorCondition),
    CPPM(MessageClass::messageTarget),
    CPPM(MessageClass::messageName),
    CPPM(MessageClass::arguments),
    CPPM(MessageClass::messageCompleted),
    CPPM(MessageClass::wait),
    CPPM(MessageClass::halt),

    CPPM(MessageClass::newRexx),

    CPPM(BaseExecutable::source),
    CPPM(BaseExecutable::getPackage),
    CPPM(BaseExecutable::getAnnotations),
    CPPM(BaseExecutable::getAnnotationRexx),

    CPPM(MethodClass::setUnguardedRexx),
    CPPM(MethodClass::setGuardedRexx),
    CPPM(MethodClass::setPrivateRexx),
    CPPM(MethodClass::setProtectedRexx),
    CPPM(MethodClass::setSecurityManager),
    CPPM(MethodClass::isGuardedRexx),
    CPPM(MethodClass::isPackageRexx),
    CPPM(MethodClass::isPrivateRexx),
    CPPM(MethodClass::isProtectedRexx),
    CPPM(MethodClass::isAbstractRexx),
    CPPM(MethodClass::isConstantRexx),
    CPPM(MethodClass::isAttributeRexx),
    CPPM(MethodClass::getScopeRexx),

    CPPM(MethodClass::newFileRexx),
    CPPM(MethodClass::newRexx),
    CPPM(MethodClass::loadExternalMethod),

    CPPM(RoutineClass::setSecurityManager),
    CPPM(RoutineClass::callRexx),
    CPPM(RoutineClass::callWithRexx),

    CPPM(RoutineClass::newFileRexx),
    CPPM(RoutineClass::newRexx),
    CPPM(RoutineClass::loadExternalRoutine),

    CPPM(PackageClass::setSecurityManagerRexx),
    CPPM(PackageClass::getProgramName),
    CPPM(PackageClass::getSourceRexx),
    CPPM(PackageClass::getSourceLineRexx),
    CPPM(PackageClass::getSourceSizeRexx),
    CPPM(PackageClass::getClassesRexx),
    CPPM(PackageClass::getPublicClassesRexx),
    CPPM(PackageClass::getImportedClassesRexx),
    CPPM(PackageClass::getMethodsRexx),
    CPPM(PackageClass::getResourcesRexx),
    CPPM(PackageClass::getResourceRexx),
    CPPM(PackageClass::getNamespacesRexx),
    CPPM(PackageClass::getAnnotations),
    CPPM(PackageClass::getAnnotationRexx),
    CPPM(PackageClass::getRoutinesRexx),
    CPPM(PackageClass::getPublicRoutinesRexx),
    CPPM(PackageClass::getImportedRoutinesRexx),
    CPPM(PackageClass::getImportedPackagesRexx),
    CPPM(PackageClass::loadPackageRexx),
    CPPM(PackageClass::addPackageRexx),
    CPPM(PackageClass::findClassRexx),
    CPPM(PackageClass::findRoutineRexx),
    CPPM(PackageClass::findPublicClassRexx),
    CPPM(PackageClass::findPublicRoutineRexx),
    CPPM(PackageClass::findNamespaceRexx),
    CPPM(PackageClass::addRoutineRexx),
    CPPM(PackageClass::addPublicRoutineRexx),
    CPPM(PackageClass::addClassRexx),
    CPPM(PackageClass::addPublicClassRexx),
    CPPM(PackageClass::loadLibraryRexx),
    CPPM(PackageClass::digitsRexx),
    CPPM(PackageClass::formRexx),
    CPPM(PackageClass::fuzzRexx),
    CPPM(PackageClass::traceRexx),
    CPPM(PackageClass::getMainRexx),
    CPPM(PackageClass::findProgramRexx),
    CPPM(PackageClass::getPackageLocal),

    CPPM(PackageClass::newRexx),

    CPPM(NumberString::formatRexx),
    CPPM(NumberString::trunc),
    CPPM(NumberString::modulo),
    CPPM(NumberString::floor),
    CPPM(NumberString::ceiling),
    CPPM(NumberString::round),
    CPPM(NumberString::equal),
    CPPM(NumberString::notEqual),
    CPPM(NumberString::isLessThan),
    CPPM(NumberString::isGreaterThan),
    CPPM(NumberString::isGreaterOrEqual),
    CPPM(NumberString::isLessOrEqual),
    CPPM(NumberString::strictNotEqual),
    CPPM(NumberString::strictLessThan),
    CPPM(NumberString::strictGreaterThan),
    CPPM(NumberString::strictGreaterOrEqual),
    CPPM(NumberString::strictLessOrEqual),
    CPPM(NumberString::plus),
    CPPM(NumberString::minus),
    CPPM(NumberString::multiply),
    CPPM(NumberString::divide),
    CPPM(NumberString::integerDivide),
    CPPM(NumberString::remainder),
    CPPM(NumberString::power),
    CPPM(NumberString::abs),
    CPPM(NumberString::Sign),
    CPPM(NumberString::notOp),
    CPPM(NumberString::andOp),
    CPPM(NumberString::orOp),
    CPPM(NumberString::xorOp),
    CPPM(NumberString::Max),
    CPPM(NumberString::Min),
    CPPM(NumberString::d2c),
    CPPM(NumberString::d2x),
    CPPM(NumberString::d2xD2c),
    CPPM(NumberString::strictEqual),
    CPPM(NumberString::hashCode),
    CPPM(NumberString::classObject),

    CPPM(QueueClass::initRexx),
    CPPM(QueueClass::pushRexx),
    CPPM(QueueClass::queueRexx),
    CPPM(QueueClass::pullRexx),
    CPPM(QueueClass::peek),
    CPPM(QueueClass::putRexx),
    CPPM(QueueClass::deleteRexx),

    CPPM(QueueClass::newRexx),
    CPPM(QueueClass::ofRexx),

    CPPM(StemClass::bracket),
    CPPM(StemClass::bracketEqual),
    CPPM(StemClass::request),
    CPPM(StemClass::supplier),
    CPPM(StemClass::allIndexes),
    CPPM(StemClass::allItems),
    CPPM(StemClass::empty),
    CPPM(StemClass::isEmptyRexx),
    CPPM(StemClass::itemsRexx),
    CPPM(StemClass::hasIndex),
    CPPM(StemClass::remove),
    CPPM(StemClass::index),
    CPPM(StemClass::hasItem),
    CPPM(StemClass::removeItemRexx),
    CPPM(StemClass::toDirectory),
    CPPM(StemClass::unknownRexx),

    CPPM(StemClass::newRexx),

    CPPM(RexxString::lengthRexx),
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
    CPPM(RexxString::choiceRexx),
    CPPM(RexxString::Max),
    CPPM(RexxString::Min),
    CPPM(RexxString::upperRexx),
    CPPM(RexxString::lowerRexx),

    CPPM(RexxString::center),
    CPPM(RexxString::delstr),
    CPPM(RexxString::insert),
    CPPM(RexxString::left),
    CPPM(RexxString::overlay),
    CPPM(RexxString::reverse),
    CPPM(RexxString::right),
    CPPM(RexxString::strip),
    CPPM(RexxString::substr),
    CPPM(RexxString::brackets),
    CPPM(RexxString::subchar),
    CPPM(RexxString::replaceAt),

    CPPM(RexxString::delWord),
    CPPM(RexxString::space),
    CPPM(RexxString::subWord),
    CPPM(RexxString::subWords),
    CPPM(RexxString::word),
    CPPM(RexxString::wordIndex),
    CPPM(RexxString::wordLength),
    CPPM(RexxString::wordPos),
    CPPM(RexxString::caselessWordPos),
    CPPM(RexxString::words),

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
    CPPM(RexxString::containsRexx),
    CPPM(RexxString::caselessContains),
    CPPM(RexxString::containsWord),
    CPPM(RexxString::caselessContainsWord),

    CPPM(RexxString::bitAnd),
    CPPM(RexxString::bitOr),
    CPPM(RexxString::bitXor),

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
    CPPM(RexxString::modulo),
    CPPM(RexxString::floor),
    CPPM(RexxString::ceiling),
    CPPM(RexxString::round),
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
    CPPM(RexxString::startsWithRexx),
    CPPM(RexxString::endsWithRexx),
    CPPM(RexxString::caselessStartsWithRexx),
    CPPM(RexxString::caselessEndsWithRexx),

    CPPM(RexxString::makeArrayRexx),

    CPPM(RexxString::newRexx),

    CPPM(MutableBuffer::newRexx),
    CPPM(MutableBuffer::lengthRexx),
    CPPM(MutableBuffer::makeArrayRexx),
    CPPM(MutableBuffer::appendRexx),
    CPPM(MutableBuffer::insert),
    CPPM(MutableBuffer::overlay),
    CPPM(MutableBuffer::mydelete),
    CPPM(MutableBuffer::substr),
    CPPM(MutableBuffer::brackets),
    CPPM(MutableBuffer::subchar),
    CPPM(MutableBuffer::posRexx),
    CPPM(MutableBuffer::lastPos),
    CPPM(MutableBuffer::caselessPos),
    CPPM(MutableBuffer::caselessLastPos),
    CPPM(MutableBuffer::getBufferSize),
    CPPM(MutableBuffer::setBufferSize),
    CPPM(MutableBuffer::replaceAt),
    CPPM(MutableBuffer::bracketsEqual),
    CPPM(MutableBuffer::countStrRexx),
    CPPM(MutableBuffer::caselessCountStrRexx),
    CPPM(MutableBuffer::changeStr),
    CPPM(MutableBuffer::caselessChangeStr),
    CPPM(MutableBuffer::upper),
    CPPM(MutableBuffer::lower),
    CPPM(MutableBuffer::translate),
    CPPM(MutableBuffer::match),
    CPPM(MutableBuffer::caselessMatch),
    CPPM(MutableBuffer::matchChar),
    CPPM(MutableBuffer::caselessMatchChar),
    CPPM(MutableBuffer::verify),
    CPPM(MutableBuffer::space),
    CPPM(MutableBuffer::subWord),
    CPPM(MutableBuffer::subWords),
    CPPM(MutableBuffer::word),
    CPPM(MutableBuffer::wordIndex),
    CPPM(MutableBuffer::wordLength),
    CPPM(MutableBuffer::words),
    CPPM(MutableBuffer::wordPos),
    CPPM(MutableBuffer::caselessWordPos),
    CPPM(MutableBuffer::delWord),
    CPPM(MutableBuffer::containsRexx),
    CPPM(MutableBuffer::caselessContains),
    CPPM(MutableBuffer::containsWord),
    CPPM(MutableBuffer::caselessContainsWord),
    CPPM(MutableBuffer::setTextRexx),
    CPPM(MutableBuffer::startsWithRexx),
    CPPM(MutableBuffer::endsWithRexx),
    CPPM(MutableBuffer::caselessStartsWithRexx),
    CPPM(MutableBuffer::caselessEndsWithRexx),
    CPPM(MutableBuffer::makeString),

    CPPM(SupplierClass::available),
    CPPM(SupplierClass::next),
    CPPM(SupplierClass::item),
    CPPM(SupplierClass::index),
    CPPM(SupplierClass::initRexx),

    CPPM(SupplierClass::newRexx),

    CPPM(TableClass::itemsRexx),
    CPPM(TableClass::newRexx),

    CPPM(IdentityTable::newRexx),
    CPPM(StringTable::newRexx),

    CPPM(SetClass::newRexx),
    CPPM(SetClass::ofRexx),

    CPPM(BagClass::newRexx),
    CPPM(BagClass::ofRexx),
    CPPM(BagClass::hasItemRexx),
    CPPM(BagClass::removeItemRexx),

    CPPM(DirectoryClass::newRexx),
    CPPM(DirectoryClass::initRexx),
    CPPM(DirectoryClass::setMethodRexx),
    CPPM(DirectoryClass::unsetMethodRexx),
    CPPM(DirectoryClass::removeRexx),

    CPPM(HashCollection::getRexx),
    CPPM(HashCollection::putRexx),
    CPPM(HashCollection::getRexx),
    CPPM(HashCollection::hasIndexRexx),
    CPPM(HashCollection::itemsRexx),
    CPPM(HashCollection::putRexx),
    CPPM(HashCollection::removeRexx),
    CPPM(HashCollection::supplier),
    CPPM(HashCollection::allItems),
    CPPM(HashCollection::allIndexes),
    CPPM(HashCollection::emptyRexx),
    CPPM(HashCollection::isEmptyRexx),
    CPPM(HashCollection::indexRexx),
    CPPM(HashCollection::hasItemRexx),
    CPPM(HashCollection::removeItemRexx),

    CPPM(StringHashCollection::entryRexx),
    CPPM(StringHashCollection::hasEntryRexx),
    CPPM(StringHashCollection::setEntryRexx),
    CPPM(StringHashCollection::removeEntryRexx),
    CPPM(StringHashCollection::unknownRexx),

    CPPM(RelationClass::removeItemRexx),
    CPPM(RelationClass::supplierRexx),
    CPPM(RelationClass::itemsRexx),
    CPPM(RelationClass::hasItemRexx),
    CPPM(RelationClass::allIndexRexx),
    CPPM(RelationClass::removeAll),
    CPPM(RelationClass::uniqueIndexes),
    CPPM(RelationClass::allAt),

    CPPM(RelationClass::newRexx),

    CPPM(ActivityManager::getLocalRexx),

    CPPM(PointerClass::equal),
    CPPM(PointerClass::notEqual),
    CPPM(PointerClass::newRexx),
    CPPM(PointerClass::isNull),

    CPPM(BufferClass::newRexx),

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
    CPPM(RexxContext::getInvocation),
    CPPM(RexxContext::getThread),
    CPPM(RexxContext::getInterpreter),
    CPPM(RexxContext::getRS),
    CPPM(RexxContext::getName),
    CPPM(RexxContext::getStackFrames),

    CPPM(StackFrameClass::getName),
    CPPM(StackFrameClass::getExecutable),
    CPPM(StackFrameClass::getLine),
    CPPM(StackFrameClass::getInvocation),
    CPPM(StackFrameClass::getContext),
    CPPM(StackFrameClass::getTraceLine),
    CPPM(StackFrameClass::getType),
    CPPM(StackFrameClass::getTarget),
    CPPM(StackFrameClass::getArguments),
    CPPM(StackFrameClass::newRexx),

    CPPM(RexxInfo::newRexx),
    CPPM(RexxInfo::copyRexx),
    CPPM(RexxInfo::getPackage),
    CPPM(RexxInfo::getDigits),
    CPPM(RexxInfo::getInternalDigits),
    CPPM(RexxInfo::getForm),
    CPPM(RexxInfo::getFuzz),
    CPPM(RexxInfo::getLanguageLevel),
    CPPM(RexxInfo::getInterpreterVersion),
    CPPM(RexxInfo::getInterpreterName),
    CPPM(RexxInfo::getInterpreterDate),
    CPPM(RexxInfo::getPlatform),
    CPPM(RexxInfo::getArchitecture),
    CPPM(RexxInfo::getFileEndOfLine),
    CPPM(RexxInfo::getPathSeparator),
    CPPM(RexxInfo::getDirectorySeparator),
    CPPM(RexxInfo::getCaseSensitiveFiles),
    CPPM(RexxInfo::getMajorVersion),
    CPPM(RexxInfo::getRelease),
    CPPM(RexxInfo::getModification),
    CPPM(RexxInfo::getRevision),
    CPPM(RexxInfo::getInternalMaxNumber),
    CPPM(RexxInfo::getInternalMinNumber),
    CPPM(RexxInfo::getMaxExponent),
    CPPM(RexxInfo::getMinExponent),
    CPPM(RexxInfo::getMaxPathLength),
    CPPM(RexxInfo::getMaxArraySize),
    CPPM(RexxInfo::getRexxExecutable),
    CPPM(RexxInfo::getRexxLibrary),
    CPPM(RexxInfo::getDebug),

    CPPM(VariableReference::newRexx),
    CPPM(VariableReference::getName),
    CPPM(VariableReference::getValue),
    CPPM(VariableReference::setValueRexx),
    CPPM(VariableReference::unknownRexx),
    CPPM(VariableReference::request),

    CPPM(EventSemaphoreClass::newRexx),
    CPPM(EventSemaphoreClass::close),
    CPPM(EventSemaphoreClass::post),
    CPPM(EventSemaphoreClass::reset),
    CPPM(EventSemaphoreClass::wait),
    CPPM(EventSemaphoreClass::posted),

    CPPM(MutexSemaphoreClass::newRexx),
    CPPM(MutexSemaphoreClass::close),
    CPPM(MutexSemaphoreClass::release),
    CPPM(MutexSemaphoreClass::request),
    // This NULL terminator is important to mark the end of the table.
    NULL
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
CPPCode *CPPCode::resolveExportedMethod(const char *name, PCPPM targetMethod, size_t argumentCount, const char *entryPointName)
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
    snprintf(buffer, sizeof(buffer), "Unresolved exported method:  %s, entrypoint: %s", name, entryPointName == NULL ? "<unknown>" : entryPointName);
    // this is a terminal error...problems in initial definitions
    Interpreter::logicError(buffer);
    return NULL;
}
