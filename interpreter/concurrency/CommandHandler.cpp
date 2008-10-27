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
/* REXX Kernel                                                                */
/*                                                                            */
/* Handlers for subcom callbacks                                              */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "RexxCore.h"
#include "CommandHandler.hpp"
#include "RexxNativeActivation.hpp"
#include "RexxInternalApis.h"
#include "ProtectedObject.hpp"
#include "SystemInterpreter.hpp"
#include "DirectoryClass.hpp"


/**
 * Create a new command handler instance
 *
 * @param size   The size of the handler
 *
 * @return The allocated object.
 */
void *CommandHandler::operator new(size_t      size)
{
    return new_object(size, T_CommandHandler);  // Get new object
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
        type = REGISTERED_NAME;
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
void CommandHandler::call(RexxActivity *activity, RexxActivation *activation, RexxString *address, RexxString *command, ProtectedObject &result, ProtectedObject &condition)
{
    if (type == REGISTERED_NAME)
    {
        CommandHandlerDispatcher dispatcher(entryPoint, command);

        // run this and give back the return code
        activity->run(dispatcher);
        dispatcher.complete(command, result, condition);
    }
    else
    {
        ContextCommandHandlerDispatcher dispatcher(entryPoint, address, command, result, condition);

        // run this and give back the return code
        activity->run(dispatcher);
    }
}

CommandHandlerDispatcher::CommandHandlerDispatcher(REXXPFN e, RexxString *command)
{
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
    if (sbrc != 0)                     /* have a numeric return code?         */
    {
        result = new_integer(sbrc);      /* just use the binary return code     */
    }
    else if (!RXNULLSTRING(retstr))  /* we have something in retstr?        */
    {
        /* make into a return string           */
        result = new_string(retstr.strptr, retstr.strlength);
        // try to get this as a numeric value
        ((RexxObject *)result)->numberValue(sbrc);
        /* user give us a new buffer?          */
        if (retstr.strptr != default_return_buffer)
        {
            /* free it                             */
            SystemInterpreter::releaseResultMemory(retstr.strptr);
        }
    }
    else
    {
        result = IntegerZero;            /* got a zero return code              */
    }

    /****************************************************************************/
    /* Check error flags from subcom handler and if needed, stick condition     */
    /* into result array.                                                       */
    /****************************************************************************/

    if (flags & (unsigned short)RXSUBCOM_FAILURE)/* If failure flag set               */
    {
        /*   send failure condition back     */
        // raise the condition when things are done
        condition = (RexxObject *)RexxActivity::createConditionObject(OREF_FAILURENAME, (RexxObject *)result, command, OREF_NULL, OREF_NULL);
    }
    /* If error flag set                 */
    else if (flags & (unsigned short)RXSUBCOM_ERROR)
    {
        // raise the condition when things are done
        condition = (RexxObject *)RexxActivity::createConditionObject(OREF_ERRORNAME, (RexxObject *)result, command, OREF_NULL, OREF_NULL);
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
void ContextCommandHandlerDispatcher::handleError(RexxDirectory *c)
{
    // this only gets added if there is a condition
    // NB:  This is called by the native activation after re-entering the
    // kernel code, so this has full access to kernel calls.
    if (c != OREF_NULL)
    {
        // check to see if this is an error or failure situation
        RexxString *conditionName = (RexxString *)c->at(OREF_CONDITION);
        // if this is not a syntax error, this is likely an error
        // or failure condition, and the return value is taken from
        // the condition object.
        if (!conditionName->strCompare(CHAR_SYNTAX))
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
