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
#include "SupplierClass.hpp"
#include "Interpreter.hpp"
#include "MethodClass.hpp"
#include "PackageClass.hpp"
#include "DirectoryClass.hpp"

BEGIN_EXTERN_C()

RexxArrayObject RexxEntry GetCallArguments(RexxCallContext *c)
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

RexxObjectPtr RexxEntry GetCallArgument(RexxCallContext *c, stringsize_t i)
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

CSTRING RexxEntry GetRoutineName(RexxCallContext *c)
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

RexxRoutineObject RexxEntry GetCurrentRoutine(RexxCallContext *c)
{
    ApiContext context(c);
    try
    {
        return (RexxRoutineObject)context.context->getExecutable();
    }
    catch (RexxNativeActivation *)
    {
    }
    return NULL;
}

void RexxEntry SetContextVariable(RexxCallContext *c, CSTRING n, RexxObjectPtr v)
{
    ApiContext context(c);
    try
    {
        context.context->setContextVariable((const char *)n, (RexxObject *)v);
    }
    catch (RexxNativeActivation *)
    {
    }
}

RexxObjectPtr RexxEntry GetContextVariable(RexxCallContext *c, CSTRING n)
{
    ApiContext context(c);
    try
    {
        return (RexxObjectPtr)context.context->getContextVariable((const char *)n);
    }
    catch (RexxNativeActivation *)
    {
    }
    return NULLOBJECT;
}

void RexxEntry DropContextVariable(RexxCallContext *c, CSTRING n)
{
    ApiContext context(c);
    try
    {
        context.context->dropContextVariable((const char *)n);
    }
    catch (RexxNativeActivation *)
    {
    }
}

RexxDirectoryObject RexxEntry GetAllContextVariables(RexxCallContext *c)
{
    ApiContext context(c);
    try
    {
        return (RexxDirectoryObject)context.ret(context.context->getAllContextVariables());
    }
    catch (RexxNativeActivation *)
    {
    }
    return NULLOBJECT;
}

RexxStemObject RexxEntry ResolveStemVariable(RexxCallContext *c, RexxObjectPtr s)
{
    ApiContext context(c);
    try
    {
        return (RexxStemObject)context.context->resolveStemVariable((RexxObject *)s);
    }
    catch (RexxNativeActivation *)
    {
        // this may throw an exception, so clear it out.  The null return is the
        // failure indication.
        context.context->clearException();
    }
    return NULLOBJECT;
}

void RexxEntry InvalidRoutine(RexxCallContext *c)
{
    ApiContext context(c);
    try
    {
        // raise an exception which will be reraised when the caller returns.
        reportException(Error_Incorrect_call_external, context.context->getMessageName());
    }
    catch (RexxNativeActivation *)
    {
    }
}

void RexxEntry SetExitContextVariable(RexxExitContext *c, CSTRING n, RexxObjectPtr v)
{
    ApiContext context(c);
    try
    {
        context.context->setContextVariable((const char *)n, (RexxObject *)v);
    }
    catch (RexxNativeActivation *)
    {
    }
}

RexxObjectPtr RexxEntry GetExitContextVariable(RexxExitContext *c, CSTRING n)
{
    ApiContext context(c);
    try
    {
        return (RexxObjectPtr)context.context->getContextVariable((const char *)n);
    }
    catch (RexxNativeActivation *)
    {
    }
    return NULLOBJECT;
}

void RexxEntry DropExitContextVariable(RexxExitContext *c, CSTRING n)
{
    ApiContext context(c);
    try
    {
        context.context->dropContextVariable((const char *)n);
    }
    catch (RexxNativeActivation *)
    {
    }
}

RexxDirectoryObject RexxEntry GetAllExitContextVariables(RexxExitContext *c)
{
    ApiContext context(c);
    try
    {
        return (RexxDirectoryObject)context.ret(context.context->getAllContextVariables());
    }
    catch (RexxNativeActivation *)
    {
    }
    return NULLOBJECT;
}


RexxObjectPtr RexxEntry GetExitCallerContext(RexxExitContext *c)
{
    ApiContext context(c);
    try
    {
        return (RexxObjectPtr)context.ret(context.context->getRexxContextObject());
    }
    catch (RexxNativeActivation *)
    {
    }
    return NULLOBJECT;
}

stringsize_t RexxEntry GetContextDigits(RexxCallContext *c)
{
    ApiContext context(c);
    try
    {
        return context.context->digits();
    }
    catch (RexxNativeActivation *)
    {
    }
    return 0;
}

stringsize_t RexxEntry GetContextFuzz(RexxCallContext *c)
{
    ApiContext context(c);
    try
    {
        return context.context->fuzz();
    }
    catch (RexxNativeActivation *)
    {
    }
    return 0;

}
logical_t RexxEntry GetContextForm(RexxCallContext *c)
{
    ApiContext context(c);
    try
    {
        return context.context->form() ? true : false;
    }
    catch (RexxNativeActivation *)
    {
    }
    return false;
}

RexxObjectPtr RexxEntry GetCallerContext(RexxCallContext *c)
{
    ApiContext context(c);
    try
    {
        return (RexxObjectPtr)context.ret(context.context->getRexxContextObject());
    }
    catch (RexxNativeActivation *)
    {
    }
    return NULLOBJECT;
}


RexxClassObject RexxEntry FindCallContextClass(RexxCallContext *c, CSTRING n)
{
    ApiContext context(c);
    try
    {
        // convert the name to a string instance, and check the environments.
        RexxString *name = new_upper_string(n);
        return (RexxClassObject)context.ret(context.context->findCallerClass(name));
    }
    catch (RexxNativeActivation *)
    {
    }
    return NULLOBJECT;
}


END_EXTERN_C()

CallContextInterface RexxActivity::callContextFunctions =
{
    CALL_INTERFACE_VERSION,
    GetCallArguments,
    GetCallArgument,
    GetRoutineName,
    GetCurrentRoutine,
    SetContextVariable,
    GetContextVariable,
    DropContextVariable,
    GetAllContextVariables,
    ResolveStemVariable,
    InvalidRoutine,
    GetContextDigits,
    GetContextFuzz,
    GetContextForm,
    GetCallerContext,
    FindCallContextClass
};

ExitContextInterface RexxActivity::exitContextFunctions =
{
    EXIT_INTERFACE_VERSION,
    SetExitContextVariable,
    GetExitContextVariable,
    DropExitContextVariable,
    GetAllExitContextVariables,
    GetExitCallerContext,
};

