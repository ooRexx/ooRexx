/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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
/* REXX Translator                                                            */
/*                                                                            */
/* Primitive Call Parse Class                                                 */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "StringTableClass.hpp"
#include "ArrayClass.hpp"
#include "VariableDictionary.hpp"
#include "RexxActivation.hpp"
#include "Activity.hpp"
#include "CallInstruction.hpp"
#include "ProtectedObject.hpp"
#include "MethodArguments.hpp"


/**
 * Construct a Call instruction object.
 *
 * @param name       The name of the call target.
 * @param argCount   The count of arguments.
 * @param argList    A queue of the arguments (stored in reverse evaluation order)
 * @param noInternal Indicates if the internal calls are disable for this
 *                   call.  This generally means the name was specified
 *                   as a quoted string.
 * @param builtin_index
 *                   An index for a potential builtin function call.
 */
RexxInstructionCall::RexxInstructionCall(RexxString *name, size_t argCount,
    QueueClass  *argList, BuiltinCode builtin_index)
{
    targetName = name;
    builtinIndex = builtin_index;
    argumentCount = argCount;

    // now copy any arguments from the sub term stack
    // NOTE:  The arguments are in last-to-first order on the stack.
    initializeObjectArray(argCount, arguments, RexxInternalObject, argList);
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxInstructionCall::live(size_t liveMark)
{
    memory_mark(nextInstruction);  // must be first one marked
    memory_mark(targetInstruction);
    memory_mark(targetName);
    memory_mark_array(argumentCount, arguments);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxInstructionCall::liveGeneral(MarkReason reason)
{
    // must be first one marked
    memory_mark_general(nextInstruction);
    memory_mark_general(targetInstruction);
    memory_mark_general(targetName);
    memory_mark_general_array(argumentCount, arguments);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxInstructionCall::flatten(Envelope *envelope)
{
    setUpFlatten(RexxInstructionCall)

    flattenRef(nextInstruction);
    flattenRef(targetInstruction);
    flattenRef(targetName);
    flattenArrayRefs(argumentCount, arguments);

    cleanUpFlatten
}

/**
 * Resolve a call target at the end of block processing.
 *
 * @param labels The table of label instructions in the current context.
 */
void RexxInstructionCall::resolve(StringTable *labels)
{
    // Note, if we are not allowed to have internal calls, we never get added to the
    // resolution list.  If we get a resolve call, then we need to check for this.
    // if there is a labels table, see if we can find a label object from the context.
    if (labels != OREF_NULL)
    {
        // see if there is a matching label.  If we get something,
        // we're finished.
        targetInstruction = (RexxInstruction *)labels->get((RexxString *)targetName);
    }

    // really nothing else required here.  If we did not resolve a label location, then
    // the next step depends on whether we have a valid builtin index or not.
}


/**
 * Runtime execution of a static Call instruction.
 *
 * @param context The current program execution context.
 * @param stack   The current context expression stack.
 */
void RexxInstructionCall::execute(RexxActivation *context, ExpressionStack *stack)
{
    // perform a stack space check here.
    ActivityManager::currentActivity->checkStackSpace();
    context->traceInstruction(this);

    // before we do anything, we need to evaluate all of the arguments.
    for (size_t i = 0; i < argumentCount; i++)
    {
        // arguments can be omitted, so don't try to evaluate any of
        // those.
        if (arguments[i] != OREF_NULL)
        {
            // evaluate what ever this argument expression is.  The
            // argument value is also pushed on to the evaluation stack
            RexxObject *argResult = arguments[i]->evaluate(context, stack);

            // trace if the settings require it.
            context->traceArgument(argResult);
        }
        else
        {
            // push an empty value on to the stack and trace this as a null string
            // value.
            stack->push(OREF_NULL);
            context->traceArgument(GlobalNames::NULLSTRING);
        }
    }

    ProtectedObject   result;            // returned result

    // if this has not resolved to an internal call, this is set to NULL
    if (targetInstruction != OREF_NULL)
    {
        context->internalCall(targetName, targetInstruction, stack->arguments(argumentCount), argumentCount, result);
    }
    // if this was resolved to a builtin, call directly
    else if (builtinIndex != NO_BUILTIN)
    {
        result = (*(LanguageParser::builtinTable[builtinIndex]))(context, argumentCount, stack);

    }
    // an external call...this is handled elsewhere.
    else
    {
        context->externalCall(targetName, stack->arguments(argumentCount), argumentCount, GlobalNames::ROUTINE, result);
    }

    // did we get a result returned?  We need to either set or drop
    // the result variable and potentially trace this.
    if ((RexxObject *)result != OREF_NULL)   /* result returned?                  */
    {
        context->setLocalVariable(GlobalNames::RESULT, VARIABLE_RESULT, (RexxObject *)result);
        context->traceResult((RexxObject *)result);
    }
    else
    {
        context->dropLocalVariable(GlobalNames::RESULT, VARIABLE_RESULT);
    }
    // and finally the debut pause.
    context->pauseInstruction();
}


/**
 * Construct a Call instruction object.
 *
 * @param expr     The expression that must resolve to the dynamic target name.
 * @param argCount The count of arguments.
 * @param argList  A queue of the arguments (stored in reverse evaluation order)
 */
RexxInstructionDynamicCall::RexxInstructionDynamicCall(RexxInternalObject *expr, size_t argCount,
    QueueClass  *argList)
{
    dynamicName = expr;
    argumentCount = argCount;

    // now copy any arguments from the sub term stack
    // NOTE:  The arguments are in last-to-first order on the stack.
    initializeObjectArray(argCount, arguments, RexxObject, argList);
}

/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxInstructionDynamicCall::live(size_t liveMark)
{
    memory_mark(nextInstruction);  // must be first one marked
    memory_mark(dynamicName);
    memory_mark_array(argumentCount, arguments);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxInstructionDynamicCall::liveGeneral(MarkReason reason)
{
    // must be first one marked
    memory_mark_general(nextInstruction);
    memory_mark_general(dynamicName);
    memory_mark_general_array(argumentCount, arguments);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxInstructionDynamicCall::flatten(Envelope *envelope)
{
    setUpFlatten(RexxInstructionDynamicCall)

    flattenRef(nextInstruction);
    flattenRef(dynamicName);
    flattenArrayRefs(argumentCount, arguments);

    cleanUpFlatten
}


/**
 * Execute a call to a dynamic call target.
 *
 * @param context The current program execution context.
 * @param stack   The current context evaluation stack.
 */
void RexxInstructionDynamicCall::execute(RexxActivation *context, ExpressionStack *stack)
{
    // perform a stack space check here.
    ActivityManager::currentActivity->checkStackSpace();
    context->traceInstruction(this);

    // NB:  This leaves this on the stack...that's fine, because
    // it protects the expression
    RexxObject *evaluatedTarget = dynamicName->evaluate(context, stack);
    // this needs to be in string form, and protected
    Protected<RexxString> targetName = evaluatedTarget->requestString();
    context->traceResult(targetName);


    // before we do anything, we need to evaluate all of the arguments.
    for (size_t i = 0; i < argumentCount; i++)
    {
        // arguments can be omitted, so don't try to evaluate any of
        // those.
        if (arguments[i] != OREF_NULL)
        {
            // evaluate what ever this argument expression is.  The
            // argument value is also pushed on to the evaluation stack
            RexxObject *argResult = arguments[i]->evaluate(context, stack);

            // trace if the settings require it.
            context->traceArgument(argResult);
        }
        else
        {
            // push an empty value on to the stack and trace this as a null string
            // value.
            stack->push(OREF_NULL);
            context->traceArgument(GlobalNames::NULLSTRING);
        }
    }

    // see if we can find an internal label target
    RexxInstruction *targetInstruction = OREF_NULL;
    // see if the context has a matching label (case sensitive lookup)
    StringTable *labels = context->getLabels();
    if (labels != OREF_NULL)
    {
        targetInstruction = (RexxInstruction *)(labels->get(targetName));
    }

    ProtectedObject   result;            // returned result

    // if this has not resolved to an internal call, this is still NULL
    if (targetInstruction != OREF_NULL)
    {
        context->internalCall(targetName, targetInstruction, stack->arguments(argumentCount), argumentCount, result);
    }
    // builtin checks come next.
    else
    {
        // map the name to a builtin index code.  If we get a hit, call this now.
        BuiltinCode builtinIndex = RexxToken::resolveBuiltin(targetName);
        if (builtinIndex != NO_BUILTIN)
        {
            result = (*(LanguageParser::builtinTable[builtinIndex]))(context, argumentCount, stack);
        }
        // an external call...this is handled elsewhere.
        else
        {
            context->externalCall(targetName, stack->arguments(argumentCount), argumentCount, GlobalNames::ROUTINE, result);
        }
    }

    // did we get a result returned?  We need to either set or drop
    // the result variable and potentially trace this.
    if ((RexxObject *)result != OREF_NULL)
    {
        context->setLocalVariable(GlobalNames::RESULT, VARIABLE_RESULT, (RexxObject *)result);
        context->traceResult((RexxObject *)result);
    }
    else
    {
        context->dropLocalVariable(GlobalNames::RESULT, VARIABLE_RESULT);
    }
    // and finally the debug pause.
    context->pauseInstruction();
}


/**
 * Construct a Call ON instruction object.
 *
 * @param condition The name of the condition trap
 * @param name      The name of the call target (NULL if this is CALL OFF)
 * @param builtin_index
 *                  An index for a potential builtin function call.
 */
RexxInstructionCallOn::RexxInstructionCallOn(RexxString *condition, RexxString *name,
    BuiltinCode builtin_index)
{
    conditionName = condition;
    targetName = name;
    builtinIndex = builtin_index;
    targetInstruction = OREF_NULL;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxInstructionCallOn::live(size_t liveMark)
{
    memory_mark(nextInstruction);  // must be first one marked
    memory_mark(targetInstruction);
    memory_mark(conditionName);
    memory_mark(targetName);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxInstructionCallOn::liveGeneral(MarkReason reason)
{
    // must be first one marked
    memory_mark_general(nextInstruction);
    memory_mark_general(targetInstruction);
    memory_mark_general(targetName);
    memory_mark_general(conditionName);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxInstructionCallOn::flatten(Envelope *envelope)
{
    setUpFlatten(RexxInstructionCallOn)

    flattenRef(nextInstruction);
    flattenRef(targetInstruction);
    flattenRef(targetName);
    flattenRef(conditionName);

    cleanUpFlatten
}

/**
 * Resolve a call target at the end of block processing.
 *
 * @param labels The table of label instructions in the current context.
 */
void RexxInstructionCallOn::resolve(StringTable *labels)
{
    // if there is a labels table, see if we can find a label object from the context.
    if (labels != OREF_NULL)
    {
        // see if there is a matching label.  We've already resolved any
        // potential builtin, so if there is no matching label we can figure out from
        // there what sort of call we have.
        targetInstruction = (RexxInstruction *)labels->get((RexxString *)targetName);
    }
}


/**
 * Execute a CALL ON/OFF instruction.  This either
 * activates or deactivates the call trap.  Calling of
 * a target only happens when a trap is activated.
 *
 * @param context The current program context.
 * @param stack   The current context evaluation stack.
 */
void RexxInstructionCallOn::execute(RexxActivation *context, ExpressionStack *stack)
{
    // do trace stuff.
    context->traceInstruction(this);

    // if we do not have a target name set, this is a CALL OFF instruction.  Just
    // disable the trap.
    if (targetName != OREF_NULL)
    {
        // wax on...
        context->trapOn(conditionName, this);
    }
    else
    {
        // wax off...
        context->trapOff(conditionName);
    }
}


/**
 * Process a trapped condition.
 *
 * @param context The trapping context.
 * @param conditionObj
 *                The condition object associated with the condition.
 */
void RexxInstructionCallOn::trap(RexxActivation *context, DirectoryClass  *conditionObj)
{
    ProtectedObject result;

    // Call ONs do not completely disable a trap, but they do put it into a
    // delayed state while the trap is executing.
    context->trapDelay(conditionName);

    // have a resolved target label?
    if (targetInstruction != OREF_NULL)
    {
        // this is handled by the activation context
        context->internalCallTrap(targetName, targetInstruction, conditionObj, result);
    }
    // match for a builtin function?  A little strange, but allowed.  This will most
    // likely give an error, since we call with no arguments
    else if (builtinIndex != NO_BUILTIN)
    {
        (*(LanguageParser::builtinTable[builtinIndex]))(context, 0, context->getStack());
    }
    // this is an external call.
    else
    {
        context->externalCall(targetName, NULL, 0, GlobalNames::ROUTINE, result);
    }

    // NOTE:  Any result object is ignored for a CALL ON trap

    // finally, restore the trap condition
    context->trapUndelay(conditionName);
}

