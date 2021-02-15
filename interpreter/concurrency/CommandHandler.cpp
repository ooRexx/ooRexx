/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2018 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                                                */
/*                                                                            */
/* Handlers for subcom callbacks                                              */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "CommandHandler.hpp"
#include "NativeActivation.hpp"
#include "RexxInternalApis.h"
#include "ProtectedObject.hpp"
#include "SystemInterpreter.hpp"
#include "DirectoryClass.hpp"
#include "ActivityManager.hpp"
#include "CommandIOContext.hpp"


/**
 * Create a new command handler instance
 *
 * @param size   The size of the handler
 *
 * @return The allocated object.
 */
void *CommandHandler::operator new(size_t size)
{
    return new_object(size, T_CommandHandler);
}


/**
 * Resolve a classic-style exit handler to the actual target
 * entry point address and invocation style.
 *
 * @param name   The registered exit name.
 */
void CommandHandler::resolve(const char *handlerName)
{
    {
        UnsafeBlock releaser;
        RexxResolveSubcom(handlerName, &entryPoint);
    }
    // only resolved if we got something back
    if (entryPoint != NULL)
    {
        type = HandlerType::REGISTERED_NAME;
    }
}


/**
 * Call a command handler
 *
 * @param activity   The current activity.
 * @param activation The top-most activation.
 * @param address    The address name.
 * @param command    The command string.
 * @param result     The returned RC value.
 * @param condition  A potential returned condition object.
 */
void CommandHandler::call(Activity *activity, RexxActivation *activation, RexxString *address, RexxString *command, ProtectedObject &result, ProtectedObject &condition, CommandIOContext *ioContext)
{
    if (type == HandlerType::REGISTERED_NAME)
    {
        // not all handlers support I/O redirection. Give an error
        // if an attempt is made here
        if (ioContext != OREF_NULL)
        {
            reportException(Error_Execution_address_redirection_not_supported, address);
        }

        CommandHandlerDispatcher dispatcher(activity, entryPoint, command);

        // run this and give back the return code
        activity->run(dispatcher);
        dispatcher.complete(command, result, condition);
    }
    // new style command handler
    else if (type == HandlerType::DIRECT)
    {
        // not all handlers support I/O redirection. Give an error
        // if an attempt is made here
        if (ioContext != OREF_NULL)
        {
            reportException(Error_Execution_address_redirection_not_supported, address);
        }

        ContextCommandHandlerDispatcher dispatcher(entryPoint, address, command, result, condition);

        // run this and give back the return code
        activity->run(dispatcher);
    }
    // a command handler that supports I/O redirection. This is
    // the only one that uses the I/O context.
    else if (type == HandlerType::REDIRECTING)
    {
        RedirectingCommandHandlerDispatcher dispatcher(entryPoint, address, command, result, condition, ioContext);

        // if we got an io context back, it is time to initialize everthing. This is the very
        // last point before we exit the interpreter.
        if (ioContext != OREF_NULL)
        {
            ioContext->init();
        }
        // run this and give back the return code
        activity->run(dispatcher);

        // and also perform clean up as soon as we get control back
        if (ioContext != OREF_NULL)
        {
            ioContext->cleanup();
        }
    }
}


CommandHandlerDispatcher::CommandHandlerDispatcher(Activity *a, REXXPFN e, RexxString *command)
{
    activity = a;               // needed for raising conditions
    entryPoint = e;             // the call point
    // clear the state flags
    flags = 0;

    // set up a return code buffer
    MAKERXSTRING(retstr, default_return_buffer, DEFRXSTRING);
    // set up the command RXSTRING
    MAKERXSTRING(rxstrcmd, command->getStringData(), command->getLength());
}


/**
 * Process a callout to a system exit function.
 */
void CommandHandlerDispatcher::run()
{
    RexxSubcomHandler *subcom_addr = (RexxSubcomHandler *)entryPoint;
    sbrc = (*subcom_addr )(&rxstrcmd, &flags, &retstr);
}


/**
 * Do post-callout processing of a command dispatch.  This
 * code runs after re-entering the interpreter, so all
 * interpreter facilities are available.
 *
 * @param result    The return RC result.
 * @param condition A potential condition return.
 */
void CommandHandlerDispatcher::complete(RexxString *command, ProtectedObject &result, ProtectedObject &condition)
{
    // did we get a numeric return code?  Turn into an Integer object.
    if (sbrc != 0)
    {
        result = new_integer(sbrc);
    }
    // maybe we got a string value back?
    else if (!RXNULLSTRING(retstr))
    {
        // make into a string value and try to convert to an integer (not an error
        // if it doesn't convert)
        result = new_string(retstr.strptr, retstr.strlength);
        // try to get this as a numeric value
        result->numberValue(sbrc);
        // handle any buffer reallocation
        if (retstr.strptr != default_return_buffer)
        {
            SystemInterpreter::releaseResultMemory(retstr.strptr);
        }
    }
    // default return code is zero
    else
    {
        result = IntegerZero;
    }

    // Check error flags from subcom handler and if needed, stick condition
    // into result array.
    if (flags & (unsigned short)RXSUBCOM_FAILURE)
    {
        // raise the condition when things are done
        condition = activity->createConditionObject(GlobalNames::FAILURE, result, command, OREF_NULL, OREF_NULL);
    }
    else if (flags & (unsigned short)RXSUBCOM_ERROR)
    {
        // raise the condition when things are done
        condition = activity->createConditionObject(GlobalNames::ERRORNAME, result, command, OREF_NULL, OREF_NULL);
    }
}


/**
 * Process a callout to a system exit function.
 */
void ContextCommandHandlerDispatcher::run()
{
    RexxContextCommandHandler *handler_address = (RexxContextCommandHandler *)entryPoint;

    ExitContext context;

    // build a context pointer to pass out
    activity->createExitContext(context, activation);

    result = (RexxObject *)(*handler_address)(&context.threadContext, (RexxStringObject)address, (RexxStringObject)command);
}


/**
 * Default handler for any error conditions.  This just sets the
 * condition information in the dispatch unit.
 *
 * @param c      The condition information for the error.
 */
void ContextCommandHandlerDispatcher::handleError(DirectoryClass *c)
{
    // this only gets added if there is a condition
    // NB:  This is called by the native activation after re-entering the
    // kernel code, so this has full access to kernel calls.
    if (c != OREF_NULL)
    {
        // check to see if this is an error or failure situation
        RexxString *conditionName = (RexxString *)c->get(GlobalNames::CONDITION);
        // if this is not a syntax error, this is likely an error
        // or failure condition, and the return value is taken from
        // the condition object.
        if (!conditionName->strCompare(GlobalNames::SYNTAX))
        {
            // just set the condition now...additional processing will be
            // done on return to the command issuer
            condition = c;
        }
        else
        {
            // raise this as a normal error by default
            activation->checkConditions();
        }
    }
}


/**
 * Process a callout to a system exit function.
 */
void RedirectingCommandHandlerDispatcher::run()
{
    RexxRedirectingCommandHandler *handler_address = (RexxRedirectingCommandHandler *)entryPoint;

    // we create two different contexts for this call. Each manages its own locks on
    // call backs
    ExitContext context;
    RedirectorContext redirectorContext;

    // build a context pointer to pass out
    activity->createExitContext(context, activation);
    activity->createRedirectorContext(redirectorContext, activation);
    redirectorContext.ioContext = ioContext;

    result = (RexxObject *)(*handler_address)(&context.threadContext, (RexxStringObject)address, (RexxStringObject)command, &redirectorContext.redirectorContext);
}
