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
#include "SupplierClass.hpp"
#include "Interpreter.hpp"
#include "MethodClass.hpp"
#include "PackageClass.hpp"
#include "DirectoryClass.hpp"
#include "ActivityManager.hpp"
#include "CommandIOContext.hpp"

BEGIN_EXTERN_C()

RexxArrayObject RexxEntry GetCallArguments(RexxCallContext *c)
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

RexxObjectPtr RexxEntry GetCallArgument(RexxCallContext *c, stringsize_t i)
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

CSTRING RexxEntry GetRoutineName(RexxCallContext *c)
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

RexxRoutineObject RexxEntry GetCurrentRoutine(RexxCallContext *c)
{
    ApiContext context(c);
    try
    {
        return (RexxRoutineObject)context.context->getExecutable();
    }
    catch (NativeActivation *)
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
    catch (NativeActivation *)
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
    catch (NativeActivation *)
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
    catch (NativeActivation *)
    {
    }
}


RexxVariableReferenceObject RexxEntry GetContextVariableReference(RexxCallContext * c, CSTRING n)
{
    ApiContext context(c);
    try
    {
        return (RexxVariableReferenceObject)context.context->getContextVariableReference((const char *)n);
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}


RexxDirectoryObject RexxEntry GetAllContextVariables(RexxCallContext * c)
{
    ApiContext context(c);
    try
    {
        return (RexxDirectoryObject)context.ret(context.context->getAllContextVariables());
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}

RexxStemObject RexxEntry ResolveStemVariable(RexxCallContext * c, RexxObjectPtr s)
{
    ApiContext context(c);
    try
    {
        return (RexxStemObject)context.context->resolveStemVariable((RexxObject *)s);
    }
    catch (NativeActivation *)
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
    catch (NativeActivation *)
    {
    }
}


// the following stubs are like the Raise versions, but don't use
// try/catch to allow a return to the caller. This will unwinded back
// to the invoking NativeActivation.
void RexxEntry CallThrowException0(RexxCallContext *c, size_t n)
{
    ApiContext context(c);
    reportException((RexxErrorCodes)n);
}

void RexxEntry CallThrowException1(RexxCallContext *c, size_t n, RexxObjectPtr o1)
{
    ApiContext context(c);
    reportException((RexxErrorCodes)n, (RexxObject *)o1);
}

void RexxEntry CallThrowException2(RexxCallContext *c, size_t n, RexxObjectPtr o1, RexxObjectPtr o2)
{
    ApiContext context(c);
    reportException((RexxErrorCodes)n, (RexxObject *)o1, (RexxObject *)o2);
}

void RexxEntry CallThrowException(RexxCallContext *c, size_t n, RexxArrayObject a)
{
    ApiContext context(c);
    reportException((RexxErrorCodes)n, (ArrayClass *)a);
}

void RexxEntry CallThrowCondition(RexxCallContext *c, CSTRING n, RexxStringObject desc, RexxObjectPtr add, RexxObjectPtr result)
{
    ApiContext context(c);
    Protected<RexxString> name = new_upper_string(n);
    context.context->enableConditionTrap();
    context.activity->raiseCondition(name, OREF_NULL, (RexxString *)desc, (RexxObject *)add, (RexxObject *)result);
}

void RexxEntry SetExitContextVariable(RexxExitContext *c, CSTRING n, RexxObjectPtr v)
{
    ApiContext context(c);
    try
    {
        context.context->setContextVariable((const char *)n, (RexxObject *)v);
    }
    catch (NativeActivation *)
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
    catch (NativeActivation *)
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
    catch (NativeActivation *)
    {
    }
}

RexxVariableReferenceObject RexxEntry GetExitContextVariableReference(RexxExitContext *c, CSTRING n)
{
    ApiContext context(c);
    try
    {
        return (RexxVariableReferenceObject)context.context->getContextVariableReference((const char *)n);
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}



RexxDirectoryObject RexxEntry GetAllExitContextVariables(RexxExitContext *c)
{
    ApiContext context(c);
    try
    {
        return (RexxDirectoryObject)context.ret(context.context->getAllContextVariables());
    }
    catch (NativeActivation *)
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
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}


// the following stubs are like the Raise versions, but don't use
// try/catch to allow a return to the caller. This will unwinded back
// to the invoking NativeActivation.
void RexxEntry ExitThrowException0(RexxExitContext *c, size_t n)
{
    ApiContext context(c);
    reportException((RexxErrorCodes)n);
}

void RexxEntry ExitThrowException1(RexxExitContext *c, size_t n, RexxObjectPtr o1)
{
    ApiContext context(c);
    reportException((RexxErrorCodes)n, (RexxObject *)o1);
}

void RexxEntry ExitThrowException2(RexxExitContext *c, size_t n, RexxObjectPtr o1, RexxObjectPtr o2)
{
    ApiContext context(c);
    reportException((RexxErrorCodes)n, (RexxObject *)o1, (RexxObject *)o2);
}

void RexxEntry ExitThrowException(RexxExitContext *c, size_t n, RexxArrayObject a)
{
    ApiContext context(c);
    reportException((RexxErrorCodes)n, (ArrayClass *)a);
}

void RexxEntry ExitThrowCondition(RexxExitContext *c, CSTRING n, RexxStringObject desc, RexxObjectPtr add, RexxObjectPtr result)
{
    ApiContext context(c);
    Protected<RexxString> name = new_upper_string(n);
    context.context->enableConditionTrap();
    context.activity->raiseCondition(name, OREF_NULL, (RexxString *)desc, (RexxObject *)add, (RexxObject *)result);
}

stringsize_t RexxEntry GetContextDigits(RexxCallContext *c)
{
    ApiContext context(c);
    try
    {
        return context.context->digits();
    }
    catch (NativeActivation *)
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
    catch (NativeActivation *)
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
    catch (NativeActivation *)
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
    catch (NativeActivation *)
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
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}


void RexxEntry ReadInput(RexxIORedirectorContext *c, CSTRING *data, size_t *length)
{
    ApiContext context(c);

    // set the default
    *data = NULL;
    *length = 0;
    try
    {
        CommandIOContext *ioContext = ((RedirectorContext *)c)->ioContext;

        // The command handler does not get passed an operable context if
        // this is not an ADDRESS WITH variant. If that's the case, we return
        // nothing if this is called.
        if (ioContext == OREF_NULL)
        {
            return;
        }
        // request the next input line. This will be NULL if we've reached the end.
        // Note that the string object is anchored by the ioContext, so
        // we don't need to add this to the context local reference table
        RexxString *nextLine = ioContext->readInput(context.context);
        if (nextLine != OREF_NULL)
        {
            *data = (CSTRING)nextLine->getStringData();
            *length = nextLine->getLength();
        }
    }
    catch (NativeActivation *)
    {
    }
    return;
}


void RexxEntry ReadInputBuffer(RexxIORedirectorContext *c, CSTRING *data, size_t *length)
{
    ApiContext context(c);

    // set the default
    *data = NULL;
    *length = 0;
    try
    {
        CommandIOContext *ioContext = ((RedirectorContext *)c)->ioContext;

        // The command handler does not get passed an operable context if
        // this is not an ADDRESS WITH variant. If that's the case, we return
        // nothing if this is called.
        if (ioContext == OREF_NULL)
        {
            return;
        }
        // go read everything into a buffer and return
        ioContext->readInputBuffered(context.context, *data, *length);
    }
    catch (NativeActivation *)
    {
    }
    return;
}


void RexxEntry WriteOutput(RexxIORedirectorContext *c, const char *data, size_t length)
{
    ApiContext context(c);
    try
    {
        CommandIOContext *ioContext = ((RedirectorContext *)c)->ioContext;

        // The command handler does not get passed an operable context if
        // this is not an ADDRESS WITH variant. If that's the case, we return
        // nothing if this is called.
        if (ioContext != OREF_NULL)
        {
            ioContext->writeOutput(context.context, data, length);
        }
    }
    catch (NativeActivation *)
    {
    }
}


void RexxEntry WriteError(RexxIORedirectorContext *c, const char *data, size_t length)
{
    ApiContext context(c);
    try
    {
        CommandIOContext *ioContext = ((RedirectorContext *)c)->ioContext;

        // The command handler does not get passed an operable context if
        // this is not an ADDRESS WITH variant. If that's the case, we return
        // nothing if this is called.
        if (ioContext != OREF_NULL)
        {
            Protected<RexxString> value = new_string(data, length);
            ioContext->writeError(context.context, data, length);
        }
    }
    catch (NativeActivation *)
    {
    }
}


void RexxEntry WriteOutputBuffer(RexxIORedirectorContext *c, const char *data, size_t length)
{
    ApiContext context(c);
    try
    {
        CommandIOContext *ioContext = ((RedirectorContext *)c)->ioContext;

        // The command handler does not get passed an operable context if
        // this is not an ADDRESS WITH variant. If that's the case, we return
        // nothing if this is called.
        if (ioContext != OREF_NULL)
        {
            ioContext->writeOutputBuffer(context.context, data, length);
        }
    }
    catch (NativeActivation *)
    {
    }
}


void RexxEntry WriteErrorBuffer(RexxIORedirectorContext *c, const char *data, size_t length)
{
    ApiContext context(c);
    try
    {
        CommandIOContext *ioContext = ((RedirectorContext *)c)->ioContext;

        // The command handler does not get passed an operable context if
        // this is not an ADDRESS WITH variant. If that's the case, we return
        // nothing if this is called.
        if (ioContext != OREF_NULL)
        {
            Protected<RexxString> value = new_string(data, length);
            ioContext->writeErrorBuffer(context.context, data, length);
        }
    }
    catch (NativeActivation *)
    {
    }
}


logical_t RexxEntry IsInputRedirected(RexxIORedirectorContext *c)
{
    ApiContext context(c);
    try
    {
        CommandIOContext *ioContext = ((RedirectorContext *)c)->ioContext;

        // The command handler does not get passed an operable context if
        // this is not an ADDRESS WITH variant. If that's the case, we return
        // nothing if this is called.
        if (ioContext != OREF_NULL)
        {
            return ioContext->isInputRedirected();
        }
    }
    catch (NativeActivation *)
    {
    }
    return false;
}


logical_t RexxEntry IsOutputRedirected(RexxIORedirectorContext *c)
{
    ApiContext context(c);
    try
    {
        CommandIOContext *ioContext = ((RedirectorContext *)c)->ioContext;

        // The command handler does not get passed an operable context if
        // this is not an ADDRESS WITH variant. If that's the case, we return
        // nothing if this is called.
        if (ioContext != OREF_NULL)
        {
            return ioContext->isOutputRedirected();
        }
    }
    catch (NativeActivation *)
    {
    }
    return false;
}


logical_t RexxEntry IsErrorRedirected(RexxIORedirectorContext *c)
{
    ApiContext context(c);
    try
    {
        CommandIOContext *ioContext = ((RedirectorContext *)c)->ioContext;

        // The command handler does not get passed an operable context if
        // this is not an ADDRESS WITH variant. If that's the case, we return
        // nothing if this is called.
        if (ioContext != OREF_NULL)
        {
            return ioContext->isErrorRedirected();
        }
    }
    catch (NativeActivation *)
    {
    }
    return false;
}


logical_t RexxEntry AreOutputAndErrorSameTarget(RexxIORedirectorContext *c)
{
    ApiContext context(c);
    try
    {
        CommandIOContext *ioContext = ((RedirectorContext *)c)->ioContext;

        // The command handler does not get passed an operable context if
        // this is not an ADDRESS WITH variant. If that's the case, we return
        // nothing if this is called.
        if (ioContext != OREF_NULL)
        {
            return ioContext->areOutputAndErrorSameTarget();
        }
    }
    catch (NativeActivation *)
    {
    }
    return false;
}


logical_t RexxEntry IsRedirectionRequested(RexxIORedirectorContext *c)
{
    // we can perform this operaiton without getting a lock
    CommandIOContext *ioContext = ((RedirectorContext *)c)->ioContext;
    return ioContext != OREF_NULL;
}


END_EXTERN_C()

/**
 * The interface vector for call context callbacks.
 */
CallContextInterface Activity::callContextFunctions =
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
    FindCallContextClass,
    GetContextVariableReference,
    CallThrowException0,
    CallThrowException1,
    CallThrowException2,
    CallThrowException,
    CallThrowCondition,
};


/**
 * The interface vector for exit handler callbacks.
 */
ExitContextInterface Activity::exitContextFunctions =
{
    EXIT_INTERFACE_VERSION,
    SetExitContextVariable,
    GetExitContextVariable,
    DropExitContextVariable,
    GetAllExitContextVariables,
    GetExitCallerContext,
    GetExitContextVariableReference,
    ExitThrowException0,
    ExitThrowException1,
    ExitThrowException2,
    ExitThrowException,
    ExitThrowCondition,
};


/**
 * The interface vector for command handler I/O redirection
 * callbacks.
 */
IORedirectorInterface Activity::ioRedirectorContextFunctions =
{
    REDIRECT_INTERFACE_VERSION,
    ReadInput,
    ReadInputBuffer,
    WriteOutput,
    WriteError,
    WriteOutputBuffer,
    WriteErrorBuffer,
    IsInputRedirected,
    IsOutputRedirected,
    IsErrorRedirected,
    AreOutputAndErrorSameTarget,
    IsRedirectionRequested,
};

