/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2019 Rexx Language Association. All rights reserved.    */
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
/* REXX API support                                                           */
/*                                                                            */
/* Stub functions for all APIs accessed via the NativeMethodContext           */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "ContextApi.hpp"
#include "NativeActivation.hpp"
#include "ProtectedObject.hpp"
#include "MethodClass.hpp"
#include "ActivityManager.hpp"

BEGIN_EXTERN_C()

RexxArrayObject RexxEntry GetMethodArguments(RexxMethodContext *c)
{
    ApiContext context(c);
    try
    {
        return (RexxArrayObject)context.context->getArguments();
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

RexxObjectPtr RexxEntry GetMethodArgument(RexxMethodContext *c, stringsize_t i)
{
    ApiContext context(c);
    try
    {
        return (RexxObjectPtr)context.context->getArgument(i);
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

RexxMethodObject RexxEntry GetCurrentMethod(RexxMethodContext *c)
{
    ApiContext context(c);
    try
    {
        return (RexxMethodObject)context.context->getExecutable();
    }
    catch (NativeActivation *)
    {
    }
    return NULL;
}

CSTRING RexxEntry GetMessageName(RexxMethodContext *c)
{
    ApiContext context(c);
    try
    {
        return (CSTRING)context.context->getMessageName()->getStringData();
    }
    catch (NativeActivation *)
    {
    }
    return NULL;
}

RexxObjectPtr RexxEntry GetSelf(RexxMethodContext *c)
{
    ApiContext context(c);
    try
    {
        return (RexxObjectPtr)context.context->getSelf();
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

POINTER RexxEntry GetCSelf(RexxMethodContext *c)
{
    ApiContext context(c);
    try
    {
        return context.context->cself();
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

RexxClassObject RexxEntry GetSuper(RexxMethodContext *c)
{
    ApiContext context(c);
    try
    {
        return (RexxClassObject)context.context->getSuper();
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

RexxObjectPtr RexxEntry GetScope(RexxMethodContext *c)
{
    ApiContext context(c);
    try
    {
        return (RexxObjectPtr)context.context->getScope();
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

void RexxEntry SetObjectVariable(RexxMethodContext *c, CSTRING n, RexxObjectPtr v)
{
    ApiContext context(c);
    try
    {
        context.context->setObjectVariable(n, (RexxObject *)v);
    }
    catch (NativeActivation *)
    {
    }
}

RexxObjectPtr RexxEntry GetObjectVariable(RexxMethodContext *c, CSTRING n)
{
    ApiContext context(c);
    try
    {
        return (RexxObjectPtr)context.context->getObjectVariable(n);
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

RexxVariableReferenceObject RexxEntry GetObjectVariableReference(RexxMethodContext *c, CSTRING n)
{
    ApiContext context(c);
    try
    {
        return (RexxVariableReferenceObject)context.context->getObjectVariableReference(n);
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

void RexxEntry DropObjectVariable(RexxMethodContext *c, CSTRING n)
{
    ApiContext context(c);
    try
    {
        context.context->dropObjectVariable(n);
    }
    catch (NativeActivation *)
    {
    }
}

RexxObjectPtr RexxEntry ForwardMessage(RexxMethodContext *c, RexxObjectPtr o, CSTRING n, RexxClassObject clazz, RexxArrayObject a)
{
    ApiContext context(c);
    try
    {
        RexxString *message = n == NULL ? OREF_NULL : new_upper_string(n);
        ProtectedObject result(context.activity);
        context.context->forwardMessage((RexxObject *)o, message, (RexxClass *)clazz, (ArrayClass *)a, result);
        return context.ret((RexxObject *)result);
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

void RexxEntry SetGuardOn(RexxMethodContext *c)
{
    ApiContext context(c);
    try
    {
        context.context->guardOn();
    }
    catch (NativeActivation *)
    {
    }
}

void RexxEntry SetGuardOff(RexxMethodContext *c)
{
    ApiContext context(c);
    try
    {
        context.context->guardOff();
    }
    catch (NativeActivation *)
    {
    }
}

RexxClassObject RexxEntry FindContextClass(RexxMethodContext *c, CSTRING n)
{
    ApiContext context(c);
    try
    {
        // convert the name to a string instance, and check the environments.
        RexxString *name = new_upper_string(n);
        return (RexxClassObject)context.ret(context.context->findClass(name));
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}


POINTER RexxEntry AllocateObjectMemory(RexxMethodContext *c, size_t l)
{
    ApiContext context(c);
    try
    {
        return context.context->allocateObjectMemory(l);
    }
    catch (NativeActivation *)
    {
        return NULL;
    }
}


POINTER RexxEntry ReallocateObjectMemory(RexxMethodContext *c, POINTER p, size_t l)
{
    ApiContext context(c);
    try
    {
        return context.context->reallocateObjectMemory(p, l);
    }
    catch (NativeActivation *)
    {
        return NULL;
    }
}


void RexxEntry FreeObjectMemory(RexxMethodContext *c, POINTER p)
{
    ApiContext context(c);
    try
    {
        context.context->freeObjectMemory(p);
    }
    catch (NativeActivation *)
    {
    }
}


RexxObjectPtr RexxEntry SetGuardOnWhenUpdated(RexxMethodContext *c, CSTRING n)
{
    ApiContext context(c);
    try
    {
        return context.ret(context.context->guardOnWhenUpdated((const char *)n));
    }
    catch (NativeActivation *)
    {
        return OREF_NULL;
    }
}


RexxObjectPtr RexxEntry SetGuardOffWhenUpdated(RexxMethodContext *c, CSTRING n)
{
    ApiContext context(c);
    try
    {
        return context.ret(context.context->guardOffWhenUpdated((const char *)n));
    }
    catch (NativeActivation *)
    {
        return OREF_NULL;
    }
}


// the following stubs are like the Raise versions, but don't use
// try/catch to allow a return to the caller. This will unwinded back
// to the invoking NativeActivation.
void RexxEntry ThrowException0(RexxMethodContext *c, size_t n)
{
    ApiContext context(c);
    reportException((RexxErrorCodes)n);
}

void RexxEntry ThrowException1(RexxMethodContext *c, size_t n, RexxObjectPtr o1)
{
    ApiContext context(c);
    reportException((RexxErrorCodes)n, (RexxObject *)o1);
}

void RexxEntry ThrowException2(RexxMethodContext *c, size_t n, RexxObjectPtr o1, RexxObjectPtr o2)
{
    ApiContext context(c);
    reportException((RexxErrorCodes)n, (RexxObject *)o1, (RexxObject *)o2);
}

void RexxEntry ThrowException(RexxMethodContext *c, size_t n, RexxArrayObject a)
{
    ApiContext context(c);
    reportException((RexxErrorCodes)n, (ArrayClass *)a);
}

void RexxEntry ThrowCondition(RexxMethodContext *c, CSTRING n, RexxStringObject desc, RexxObjectPtr add, RexxObjectPtr result)
{
    ApiContext context(c);
    Protected<RexxString> name = new_upper_string(n);
    context.context->enableConditionTrap();
    context.activity->raiseCondition(name, OREF_NULL, (RexxString *)desc, (RexxObject *)add, (RexxObject *)result);
}


END_EXTERN_C()


MethodContextInterface Activity::methodContextFunctions =
{
    METHOD_INTERFACE_VERSION,
    GetMethodArguments,
    GetMethodArgument,
    GetMessageName,
    GetCurrentMethod,
    GetSelf,
    GetSuper,
    GetScope,
    SetObjectVariable,
    GetObjectVariable,
    DropObjectVariable,
    ForwardMessage,
    SetGuardOn,
    SetGuardOff,
    FindContextClass,
    GetCSelf,
    AllocateObjectMemory,
    FreeObjectMemory,
    ReallocateObjectMemory,
    GetObjectVariableReference,
    SetGuardOnWhenUpdated,
    SetGuardOffWhenUpdated,
    ThrowException0,
    ThrowException1,
    ThrowException2,
    ThrowException,
    ThrowCondition,
};


