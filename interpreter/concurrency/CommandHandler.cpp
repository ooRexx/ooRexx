/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2026 Rexx Language Association. All rights reserved.    */
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
#include "MethodClass.hpp"
#include "RexxActivation.hpp"
#include "Interpreter.hpp"
#include "InterpreterInstance.hpp"
#include "BufferClass.hpp"
#include "ArrayClass.hpp"
#include "StringTableClass.hpp"
#include "VariableDictionary.hpp"
#include "ExpressionBaseVariable.hpp"
#include "../api/ContextApi.hpp"


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
 * Normal garbage collection live marking
 *
 * @param liveMark The current live mark.
 */
void CommandHandler::live(size_t liveMark)
{
    memory_mark(handlerObject);
    memory_mark(wrapper);
    memory_mark(previous);
}


/**
 * Generalized object marking.
 *
 * @param reason The reason for this live marking operation.
 */
void CommandHandler::liveGeneral(MarkReason reason)
{
    memory_mark_general(handlerObject);
    memory_mark_general(wrapper);
    memory_mark_general(previous);
}


/**
 * Look up a classic subcommand handler registered (RexxRegisterSubcom...)
 * under a name.  The lookup goes to the RexxAPI with the kernel lock
 * released, so it fills a local variable: an object written to meanwhile
 * (as this used to write into a new CommandHandler that nothing anchored
 * yet) may be collected by another thread, and the write then lands in
 * whatever reuses its memory.
 *
 * @param handlerName
 *               The registered subcommand handler name.
 *
 * @return The entry point, or NULL if no handler is registered under it.
 */
REXXPFN CommandHandler::resolveRegisteredName(const char *handlerName)
{
    REXXPFN entry = NULL;
    {
        UnsafeBlock releaser;
        RexxResolveSubcom(handlerName, &entry);
    }
    return entry;
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
    // a command environment written in Rexx: the command goes straight to
    // the handler object, on this activity, with no native code in between
    else if (type == HandlerType::REXX_OBJECT)
    {
        callRexxObject(activity, activation, address, command, result, condition, ioContext);
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



/******************************************************************************/
/* Command environments written in Rexx (.CommandHandler~register)            */
/*                                                                            */
/* The handler object gets, for every command,                                */
/*                                                                            */
/*     handler~command(command, caller)                                       */
/*                                                                            */
/* where caller is a .CommandContext (CoreClasses.orx), valid while that      */
/* command runs and only on its activity.  The context's native methods       */
/* below reach the activation that issued the command (its variables) and    */
/* its ADDRESS ... WITH redirection through a CommandState, kept in a Buffer  */
/* the context holds (its CSELF).                                             */
/******************************************************************************/

namespace
{
    const uint32_t CommandStateMagic = 0x434d4448;   // "CMDH"

    struct CommandState
    {
        uint32_t          magic;       // CommandStateMagic: a buffer we made
        bool              valid;       // only while the command runs
        Activity         *activity;    // the activity that runs the command
        RexxActivation   *activation;  // the code that issued the command
        CommandIOContext *io;          // its ADDRESS ... WITH, or OREF_NULL
    };
}


/**
 * Call a command handler written in Rexx.
 *
 * @param activity   The current activity.
 * @param activation The activation that issued the command.
 * @param address    The address name.
 * @param command    The command string.
 * @param result     The returned RC value.
 * @param condition  A potential returned condition object.
 * @param ioContext  The ADDRESS ... WITH redirection, or OREF_NULL.
 */
void CommandHandler::callRexxObject(Activity *activity, RexxActivation *activation, RexxString *address, RexxString *command, ProtectedObject &result, ProtectedObject &condition, CommandIOContext *ioContext)
{
    Protected<BufferClass> buffer = new_buffer(sizeof(CommandState));
    CommandState *state = (CommandState *)buffer->getData();
    state->magic = CommandStateMagic;
    state->valid = true;
    state->activity = activity;
    state->activation = activation;
    state->io = ioContext;

    // the context object, a .CommandContext from the REXX package
    RexxClass *contextClass = Interpreter::findClass(new_string("COMMANDCONTEXT"));
    ProtectedObject contextObject;
    contextClass->sendMessage(GlobalNames::NEW, buffer, address, contextObject);

    if (ioContext != OREF_NULL)
    {
        ioContext->init();
    }

    ProtectedObject commandResult;
    try
    {
        handlerObject->sendMessage(GlobalNames::COMMAND, command, (RexxObject *)contextObject, commandResult);
        // write what the output streams' charout left, while still valid
        ProtectedObject dummy;
        ((RexxObject *)contextObject)->sendMessage(new_string("FINISH"), dummy);
    }
    catch (...)
    {
        // an error in the handler (SYNTAX) reaches the caller as it is
        state->valid = false;
        throw;
    }
    state->valid = false;               // the context is dead from now on

    if (ioContext != OREF_NULL)
    {
        ioContext->cleanup();
    }

    // did the handler ask for a condition (caller~raise)?
    ProtectedObject pendingObject;
    ((RexxObject *)contextObject)->sendMessage(new_string("PENDING"), pendingObject);
    RexxObject *pending = pendingObject;
    if (pending != OREF_NULL && pending != TheNilObject && isArray(pending))
    {
        ArrayClass *request = (ArrayClass *)pending;
        RexxString *name = ((RexxObject *)request->get(1))->requestString();
        RexxObject *rc = (RexxObject *)request->get(2);
        RexxObject *description = (RexxObject *)request->get(3);
        RexxObject *additional = (RexxObject *)request->get(4);
        if (name->strCompare("SYNTAX"))
        {
            // raised here, at the caller's ADDRESS clause, without the
            // handler's frames; rc is the error number (major * 1000 + minor)
            wholenumber_t code = 0;
            rc->numberValue(code);
            reportException((RexxErrorCodes)code, (ArrayClass *)additional);
        }
        RexxString *text = (description == OREF_NULL || description == TheNilObject) ? command : description->requestString();
        if (additional == TheNilObject)
        {
            additional = OREF_NULL;
        }
        result = rc;
        if (name->strCompare("HALT"))
        {
            // a HALT is given back as a HALT: as a Ctrl-C, it is seen at the
            // end of the clause (SIGNAL ON HALT, CALL ON HALT), and untrapped
            // it is SYNTAX 4.1, as in plain Rexx
            activation->halt(text);
            return;
        }
        condition = activity->createConditionObject(name, rc, text, additional, OREF_NULL);
        return;
    }
    result = (RexxObject *)commandResult;
}


/**
 * The CommandState of a .CommandContext, or a SYNTAX error once its command
 * ended or when it is used from another activity.
 */
static CommandState *liveState(RexxMethodContext *context, void *self, Activity *activity)
{
    CommandState *state = (CommandState *)self;
    if (state == NULL || state->magic != CommandStateMagic || !state->valid)
    {
        reportException(Error_Execution_user_defined, "this CommandContext belongs to a command that has ended");
    }
    if (state->activity != activity)
    {
        reportException(Error_Execution_user_defined, "this CommandContext can only be used on the thread of its command");
    }
    return state;
}


/**
 * A variable retriever for a name, or a SYNTAX error if it is not a variable.
 */
static RexxVariableBase *variableRetriever(const char *name)
{
    RexxVariableBase *retriever = VariableDictionary::getVariableRetriever(new_string(name));
    if (retriever == OREF_NULL || isString(retriever))
    {
        reportException(Error_Incorrect_method_user_defined, new_string("not a Rexx variable name: ")->concatWithCstring(name));
    }
    return retriever;
}


/**
 * .CommandHandler~register's native part: makes handler the environment name.
 */
RexxMethod2(RexxObjectPtr, cmdhandler_register, RexxStringObject, name, RexxObjectPtr, handler)
{
    ApiContext ctx(context);
    try
    {
        RexxObject *covered = ctx.activity->getInstance()->addCommandHandler((RexxString *)name, (RexxObject *)handler);
        return covered == OREF_NULL ? context->Nil() : ctx.ret(covered);
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}


/**
 * .CommandHandler~deregister's native part: whether there was a handler
 * written in Rexx under this name (and it is gone now).
 */
RexxMethod1(RexxObjectPtr, cmdhandler_deregister, RexxStringObject, name)
{
    ApiContext ctx(context);
    try
    {
        RexxObject *removed = ctx.activity->getInstance()->removeCommandHandler((RexxString *)name);
        return removed == OREF_NULL ? context->Nil() : ctx.ret(removed);
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}


/**
 * .CommandHandler~handlers(name): the handlers of name, the one in use
 * first and then the ones it covers (an empty array if there is none).
 */
RexxMethod1(RexxObjectPtr, cmdhandler_handlers, RexxStringObject, name)
{
    ApiContext ctx(context);
    try
    {
        return ctx.ret(ctx.activity->getInstance()->listCommandHandlerStack((RexxString *)name));
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}


/**
 * .CommandHandler~environments: a StringTable of the command environments
 * known to this interpreter instance, the handler object for those written
 * in Rexx and "NATIVE" for the others.
 */
RexxMethod0(RexxObjectPtr, cmdhandler_environments)
{
    ApiContext ctx(context);
    try
    {
        return ctx.ret(ctx.activity->getInstance()->listCommandHandlers());
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}


/**
 * The next line of the redirected input, .nil at its end.
 */
RexxMethod1(RexxObjectPtr, cmdcontext_lineIn, CSELF, self)
{
    ApiContext ctx(context);
    try
    {
        CommandState *state = liveState(context, self, ctx.activity);
        if (state->io == OREF_NULL)
        {
            return context->Nil();
        }
        RexxString *line = state->io->readInput(ctx.context);
        if (line == OREF_NULL)
        {
            return context->Nil();
        }
        return ctx.ret(line);
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}


/**
 * write(stream, line): one line to the redirected output ("O") or error ("E").
 */
RexxMethod3(RexxObjectPtr, cmdcontext_write, CSTRING, stream, RexxStringObject, line, CSELF, self)
{
    ApiContext ctx(context);
    try
    {
        CommandState *state = liveState(context, self, ctx.activity);
        if (state->io != OREF_NULL)
        {
            RexxString *text = (RexxString *)line;
            if (stream[0] == 'E' || stream[0] == 'e')
            {
                state->io->writeError(ctx.context, text->getStringData(), text->getLength());
            }
            else
            {
                state->io->writeOutput(ctx.context, text->getStringData(), text->getLength());
            }
        }
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}


/**
 * ~isRedirected(which): "Input", "Output", "Error" (first letter), or "Same".
 */
RexxMethod2(logical_t, cmdcontext_isRedirected, CSTRING, which, CSELF, self)
{
    ApiContext ctx(context);
    try
    {
        CommandState *state = liveState(context, self, ctx.activity);
        CommandIOContext *io = state->io;
        switch (which[0])
        {
            case 'I': case 'i': return io != OREF_NULL && io->isInputRedirected();
            case 'O': case 'o': return io != OREF_NULL && io->isOutputRedirected();
            case 'E': case 'e': return io != OREF_NULL && io->isErrorRedirected();
            case 'S': case 's': return io != OREF_NULL && io->areOutputAndErrorSameTarget();
        }
        reportException(Error_Incorrect_method_user_defined, "isRedirected: the argument must be Input, Output, Error or Same");
    }
    catch (NativeActivation *)
    {
    }
    return false;
}


/**
 * caller[name]: the caller's variable, or .nil when it has no value.
 */
RexxMethod2(RexxObjectPtr, cmdcontext_getVar, CSTRING, name, CSELF, self)
{
    ApiContext ctx(context);
    try
    {
        CommandState *state = liveState(context, self, ctx.activity);
        RexxObject *value = variableRetriever(name)->getRealValue(state->activation);
        return value == OREF_NULL ? context->Nil() : ctx.ret(value);
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}


/**
 * ~hasVar(name): whether the caller's variable has a value.
 */
RexxMethod2(logical_t, cmdcontext_hasVar, CSTRING, name, CSELF, self)
{
    ApiContext ctx(context);
    try
    {
        CommandState *state = liveState(context, self, ctx.activity);
        return variableRetriever(name)->getRealValue(state->activation) != OREF_NULL;
    }
    catch (NativeActivation *)
    {
    }
    return false;
}


/**
 * caller[name] = value: assigns the caller's variable (simple, stem or
 * compound; a compound's tail is resolved in the caller, as Rexx does).
 */
RexxMethod3(RexxObjectPtr, cmdcontext_setVar, CSTRING, name, RexxObjectPtr, value, CSELF, self)
{
    ApiContext ctx(context);
    try
    {
        CommandState *state = liveState(context, self, ctx.activity);
        variableRetriever(name)->set(state->activation, (RexxObject *)value);
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}


/**
 * ~dropVar(name): drops the caller's variable.
 */
RexxMethod2(RexxObjectPtr, cmdcontext_dropVar, CSTRING, name, CSELF, self)
{
    ApiContext ctx(context);
    try
    {
        CommandState *state = liveState(context, self, ctx.activity);
        variableRetriever(name)->drop(state->activation);
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}


/**
 * ~variables: a Directory with all the caller's variables.
 */
RexxMethod1(RexxObjectPtr, cmdcontext_variables, CSELF, self)
{
    ApiContext ctx(context);
    try
    {
        CommandState *state = liveState(context, self, ctx.activity);
        return ctx.ret(state->activation->getAllLocalVariables());
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}


/**
 * Nothing, or the SYNTAX error of liveState (for methods written in Rexx).
 */
RexxMethod1(int, cmdcontext_check, CSELF, self)
{
    ApiContext ctx(context);
    try
    {
        liveState(context, self, ctx.activity);
    }
    catch (NativeActivation *)
    {
    }
    return 0;
}


/**
 * ~isValid: whether the command is still running.
 */
RexxMethod1(logical_t, cmdcontext_isValid, CSELF, self)
{
    CommandState *state = (CommandState *)self;
    return state != NULL && state->magic == CommandStateMagic && state->valid;
}


/******************************************************************************/
/* Every environment as an object (.CommandHandler~handler, ~handlers,        */
/* ~environments): native handlers are .NativeCommandHandler objects, whose   */
/* COMMAND forwards to them, so that a handler written in Rexx can forward a  */
/* command to the handler it covers.                                          */
/******************************************************************************/

/**
 * The Rexx object of this handler: the handler object itself, or for a
 * native handler a .NativeCommandHandler made once and kept (so the same
 * native handler is always the same object).
 */
RexxObject *CommandHandler::getRexxObject(RexxString *name)
{
    if (type == HandlerType::REXX_OBJECT)
    {
        return handlerObject;
    }
    if (wrapper == OREF_NULL)
    {
        RexxClass *nativeClass = Interpreter::findClass(new_string("NATIVECOMMANDHANDLER"));
        // NEW is private (only the interpreter makes them): run it directly
        ProtectedObject made;
        MethodClass *newMethod = nativeClass->methodLookup(GlobalNames::NEW);
        newMethod->run(ActivityManager::currentActivity, nativeClass, GlobalNames::NEW, OREF_NULL, 0, made);
        RexxObject *object = made;
        object->getObjectVariables(nativeClass)->set(new_string("HANDLER"), (RexxObject *)this);
        // the name it was registered under: a native handler may depend on it
        // (the Unix one runs the shell of that name)
        object->getObjectVariables(nativeClass)->set(new_string("NAME"), name->upper());
        setField(wrapper, object);
    }
    return wrapper;
}


/**
 * Call a native handler for a command forwarded by a handler written in
 * Rexx: as call(), but the redirection is already set up (and will be
 * cleaned up) by the handler written in Rexx that got the command first.
 */
void CommandHandler::callForwarded(Activity *activity, RexxActivation *activation, RexxString *address, RexxString *command, ProtectedObject &result, ProtectedObject &condition, CommandIOContext *ioContext)
{
    if (type == HandlerType::REDIRECTING)
    {
        RedirectingCommandHandlerDispatcher dispatcher(entryPoint, address, command, result, condition, ioContext);
        activity->run(dispatcher);
        return;
    }
    call(activity, activation, address, command, result, condition, ioContext);
}


const char *CommandHandler::kindName()
{
    switch (type)
    {
        case HandlerType::REGISTERED_NAME: return "SUBCOM";
        case HandlerType::DIRECT:          return "DIRECT";
        case HandlerType::REDIRECTING:     return "REDIRECTING";
        case HandlerType::REXX_OBJECT:     return "REXX";
        default:                           return "UNRESOLVED";
    }
}


/**
 * The CommandHandler kept in a .NativeCommandHandler, or a SYNTAX error.
 */
static CommandHandler *nativeHandlerOf(RexxObject *object)
{
    RexxClass *nativeClass = Interpreter::findClass(new_string("NATIVECOMMANDHANDLER"));
    RexxInternalObject *handler = object->isInstanceOf(nativeClass) ? object->getObjectVariable(new_string("HANDLER"), nativeClass) : OREF_NULL;
    if (handler == OREF_NULL || !isOfClass(CommandHandler, handler))
    {
        reportException(Error_Incorrect_method_user_defined, new_string("not a native command handler made by the interpreter"));
    }
    return (CommandHandler *)handler;
}


/**
 * .CommandHandler~handler(name): the object of the environment name, or
 * .nil if there is none.
 */
RexxMethod1(RexxObjectPtr, cmdhandler_handler, RexxStringObject, name)
{
    ApiContext ctx(context);
    try
    {
        CommandHandler *handler = ctx.activity->getInstance()->resolveCommandHandler((RexxString *)name);
        if (handler == OREF_NULL)
        {
            return context->Nil();
        }
        return ctx.ret(handler->getRexxObject((RexxString *)name));
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}


/**
 * .NativeCommandHandler~kind: SUBCOM, DIRECT or REDIRECTING.
 */
RexxMethod1(RexxObjectPtr, cmdnative_kind, OSELF, self)
{
    ApiContext ctx(context);
    try
    {
        return ctx.ret(new_string(nativeHandlerOf((RexxObject *)self)->kindName()));
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}


/**
 * caller~!forward(nativeHandler, command): sends the command to a native
 * handler as if the code that issued it had (its variables, its ADDRESS ...
 * WITH redirection); its condition, if any, becomes this command's.
 * Returns the RC.  The native handler gets the code that issued the command
 * as its caller (callbackParent): otherwise it would see the variables of
 * the COMMAND method on top of the stack.  It also gets the name it was
 * registered under, not the one the command came through.
 */
RexxMethod3(RexxObjectPtr, cmdcontext_forward, RexxObjectPtr, target, RexxStringObject, command, OSELF, self)
{
    ApiContext ctx(context);
    try
    {
        RexxObject *contextObject = (RexxObject *)self;
        RexxClass *contextClass = Interpreter::findClass(new_string("COMMANDCONTEXT"));
        RexxObject *cself = contextObject->getObjectVariable(new_string("CSELF"), contextClass);
        CommandState *state = liveState(context, cself == OREF_NULL ? NULL : ((BufferClass *)cself)->getData(), ctx.activity);
        CommandHandler *handler = nativeHandlerOf((RexxObject *)target);
        RexxClass *nativeClass = Interpreter::findClass(new_string("NATIVECOMMANDHANDLER"));
        RexxString *address = (RexxString *)((RexxObject *)target)->getObjectVariable(new_string("NAME"), nativeClass);

        ProtectedObject result;
        ProtectedObject condition;
        ctx.activity->setCallbackParent(state->activation);
        handler->callForwarded(ctx.activity, state->activation, address, (RexxString *)command, result, condition, state->io);
        ctx.activity->setCallbackParent(OREF_NULL);

        if ((RexxObject *)condition != OREF_NULL)
        {
            DirectoryClass *c = (DirectoryClass *)(RexxObject *)condition;
            // a handler that raises its condition gives its RC there
            // (as RexxActivation::command: RC, or else RESULT)
            RexxObject *conditionRC = (RexxObject *)c->get(GlobalNames::RC);
            if (conditionRC == OREF_NULL)
            {
                conditionRC = (RexxObject *)c->get(GlobalNames::RESULT);
            }
            if (conditionRC != OREF_NULL)
            {
                result = conditionRC;
            }
            ArrayClass *pending = new_array((RexxObject *)c->get(GlobalNames::CONDITION), (RexxObject *)result,
                (RexxObject *)c->get(GlobalNames::DESCRIPTION), TheNilObject);
            contextObject->getObjectVariables(contextClass)->set(new_string("PENDING"), pending);
        }
        RexxObject *rc = result;
        return rc == OREF_NULL ? context->Nil() : ctx.ret(rc);
    }
    catch (NativeActivation *)
    {
    }
    return NULLOBJECT;
}
