/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.ibm.com/developerworks/oss/CPLv1.0.htm                          */
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
#include "RexxNativeActivation.hpp"
#include "ProtectedObject.hpp"
#include "MethodClass.hpp"

BEGIN_EXTERN_C()

RexxArrayObject RexxEntry GetMethodArguments(RexxMethodContext *c)
{
    ApiContext context(c);
    try
    {
        return (RexxArrayObject)context.context->getArguments();
    }
    catch (RexxNativeActivation *)
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
    catch (RexxNativeActivation *)
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
    catch (RexxNativeActivation *)
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
    catch (RexxNativeActivation *)
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
    catch (RexxNativeActivation *)
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
    catch (RexxNativeActivation *)
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
    catch (RexxNativeActivation *)
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
    catch (RexxNativeActivation *)
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
    catch (RexxNativeActivation *)
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
    catch (RexxNativeActivation *)
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
    catch (RexxNativeActivation *)
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
        context.context->forwardMessage((RexxObject *)o, message, (RexxClass *)clazz, (RexxArray *)a, result);
        return context.ret((RexxObject *)result);
    }
    catch (RexxNativeActivation *)
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
    catch (RexxNativeActivation *)
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
    catch (RexxNativeActivation *)
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
    catch (RexxNativeActivation *)
    {
    }
    return NULLOBJECT;
}

END_EXTERN_C()


MethodContextInterface RexxActivity::methodContextFunctions =
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
};


