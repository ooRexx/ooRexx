/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2022 Rexx Language Association. All rights reserved.    */
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
/* REXX Translator                                                            */
/*                                                                            */
/* Primitive Call Parse Class                                                 */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "StringClass.hpp"
#include "DirectoryClass.hpp"
#include "StringTableClass.hpp"
#include "ArrayClass.hpp"
#include "VariableDictionary.hpp"
#include "RexxActivation.hpp"
#include "Activity.hpp"
#include "CallInstruction.hpp"
#include "ProtectedObject.hpp"
#include "MethodArguments.hpp"
#include "RoutineClass.hpp"


/**
 * Construct a Call instruction object.
 *
 * @param name       The name of the call target.
 * @param argCount   The count of arguments.
 * @param argList    A queue of the arguments (stored in reverse evaluation order)
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
    memory_mark(externalTarget);
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
    memory_mark_general(externalTarget);
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
    flattenRef(externalTarget);
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

    // evaluate the arguments first
    RexxInstruction::evaluateArguments(context, stack, arguments, argumentCount);

    ProtectedObject   result;            // returned result

    // do we have a resolved external routine to call?
    if (externalTarget != OREF_NULL)
    {
        context->externalCall(targetName, externalTarget, stack->arguments(argumentCount), argumentCount, GlobalNames::SUBROUTINE, result);
    }

    // if this has not resolved to an internal call, this is set to NULL
    else if (targetInstruction != OREF_NULL)
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
        // this is a potentially resolved external target
        RoutineClass *resolvedTarget = OREF_NULL;

        context->externalCall(resolvedTarget, targetName, stack->arguments(argumentCount), argumentCount, GlobalNames::SUBROUTINE, result);

        // this potentially resolved a target that will allow us
        // to fast path the next call
        setField(externalTarget, resolvedTarget);
    }

    // did we get a result returned?  We need to either set or drop
    // the result variable and potentially trace this.
    if (!result.isNull())
    {
        context->setLocalVariable(GlobalNames::RESULT, VARIABLE_RESULT, result);
        context->traceResult(result);
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

    // evaluate the arguments first
    RexxInstruction::evaluateArguments(context, stack, arguments, argumentCount);

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
            // we need to provide the variable, but we don't cache the result in this case
            RoutineClass *resolvedRoutine = OREF_NULL;

            context->externalCall(resolvedRoutine, targetName, stack->arguments(argumentCount), argumentCount, GlobalNames::SUBROUTINE, result);
        }
    }

    // did we get a result returned?  We need to either set or drop
    // the result variable and potentially trace this.
    if (!result.isNull())
    {
        context->setLocalVariable(GlobalNames::RESULT, VARIABLE_RESULT, result);
        context->traceResult(result);
    }
    else
    {
        context->dropLocalVariable(GlobalNames::RESULT, VARIABLE_RESULT);
    }
    // and finally the debug pause.
    context->pauseInstruction();
}


/**
 * Construct a Call instruction object.
 *
 * @param n        The target namespace
 * @param r        the name of the routine.
 * @param argCount The count of arguments.
 * @param argList  A queue of the arguments (stored in reverse evaluation order)
 */
RexxInstructionQualifiedCall::RexxInstructionQualifiedCall(RexxString *n, RexxString *r,
    size_t argCount, QueueClass *argList)
{
    namespaceName = n;
    routineName = r;

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
void RexxInstructionQualifiedCall::live(size_t liveMark)
{
    memory_mark(nextInstruction);  // must be first one marked
    memory_mark(namespaceName);
    memory_mark(routineName);
    memory_mark_array(argumentCount, arguments);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxInstructionQualifiedCall::liveGeneral(MarkReason reason)
{
    // must be first one marked
    memory_mark_general(nextInstruction);
    memory_mark_general(namespaceName);
    memory_mark_general(routineName);
    memory_mark_general_array(argumentCount, arguments);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxInstructionQualifiedCall::flatten(Envelope *envelope)
{
    setUpFlatten(RexxInstructionQualifiedCall)

    flattenRef(nextInstruction);
    flattenRef(namespaceName);
    flattenRef(routineName);
    flattenArrayRefs(argumentCount, arguments);

    cleanUpFlatten
}


/**
 * Execute a call to a dynamic call target.
 *
 * @param context The current program execution context.
 * @param stack   The current context evaluation stack.
 */
void RexxInstructionQualifiedCall::execute(RexxActivation *context, ExpressionStack *stack)
{
    // perform a stack space check here.
    ActivityManager::currentActivity->checkStackSpace();
    context->traceInstruction(this);

    // evaluate the arguments first
    RexxInstruction::evaluateArguments(context, stack, arguments, argumentCount);

    ProtectedObject   result;            // returned result

    // get the current package from the context
    PackageClass *package = context->getPackage();

    // we must be able to find the named namespace
    PackageClass *namespacePackage = package->findNamespace(namespaceName);
    if (namespacePackage == OREF_NULL)
    {
        reportException(Error_Execution_no_namespace, namespaceName, package->getProgramName());
    }

    // we only look for public routines in the namespaces
    RoutineClass *resolvedRoutine = namespacePackage->findPublicRoutine(routineName);
    // we give a specific error for this one
    if (resolvedRoutine == OREF_NULL)
    {
        reportException(Error_Routine_not_found_namespace, routineName, namespaceName);
    }

    // call the resolved function
    resolvedRoutine->call(context->getActivity(), routineName, stack->arguments(argumentCount), argumentCount, GlobalNames::SUBROUTINE, OREF_NULL, EXTERNALCALL, result);


    // did we get a result returned?  We need to either set or drop
    // the result variable and potentially trace this.
    if (!result.isNull())
    {
        context->setLocalVariable(GlobalNames::RESULT, VARIABLE_RESULT, result);
        context->traceResult(result);
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
        context->trapOn(conditionName, this, false);
    }
    else
    {
        // wax off...
        context->trapOff(conditionName, false);
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
        // arguments to builtins are passed on the stack
        context->getStack()->push(conditionObj);

        (*(LanguageParser::builtinTable[builtinIndex]))(context, 1, context->getStack());
    }
    // this is an external call.
    else
    {
        // we need to provide the variable, but we don't cache the result in this case
        RoutineClass *resolvedRoutine = OREF_NULL;

        context->externalCall(resolvedRoutine, targetName, (RexxObject **)&conditionObj, 1, GlobalNames::SUBROUTINE, result);
    }

    // NOTE:  Any result object is ignored for a CALL ON trap

    // finally, restore the trap condition
    context->trapUndelay(conditionName);
}

