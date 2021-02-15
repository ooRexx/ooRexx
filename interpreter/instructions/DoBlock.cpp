/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2019 Rexx Language Association. All rights reserved.    */
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
/* Primitive DO/SELECT block class                                            */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "DoInstruction.hpp"
#include "DoBlock.hpp"
#include "RexxActivation.hpp"
#include "Numerics.hpp"

/**
 * Allocate a new DoBlock object.
 *
 * @param size   The object size
 *
 * @return Storage for this object allocated from the Rexx object heap.
 */
void * DoBlock::operator new(size_t size)
{
    return new_object(size, T_DoBlock);
}


/**
 * Construct a runtime DO block instance.
 *
 * @param _parent The instruction this represents.
 * @param _indent The current trace indentation level.
 */
DoBlock::DoBlock(RexxActivation *context, RexxBlockInstruction *_parent)
{
    parent = _parent;
    indent = context->getIndent();
    countVariable = parent->getCountVariable();
    // if we have a count variable, then we need to set the initial value
    // to zero before we do the loop termination tests.
    if (countVariable != OREF_NULL)
    {
        countVariable->assign(context, IntegerZero);
        context->traceKeywordResult(GlobalNames::COUNTER, IntegerZero);
    }
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void DoBlock::live(size_t liveMark)
{
    memory_mark(previous);
    memory_mark(parent);
    memory_mark(control);
    memory_mark(to);
    memory_mark(by);
    memory_mark(countVariable);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void DoBlock::liveGeneral(MarkReason reason)
{
    memory_mark_general(previous);
    memory_mark_general(parent);
    memory_mark_general(control);
    memory_mark_general(to);
    memory_mark_general(by);
    memory_mark_general(countVariable);
}


/**
 * Handle a new iteration of a loop, with setting of the counter variable, if required.
 *
 * @param context The current execution context,
 * @param v       The counter variable (if any) to set.
 */
void DoBlock::setCounter(RexxActivation *context)
{
    if (countVariable != OREF_NULL)
    {
        // assign the control variable and trace this result
        Protected<RexxObject> c =  Numerics::uint64ToObject(counter);
        countVariable->assign(context, c);
        context->traceKeywordResult(GlobalNames::COUNTER, c);
    }
}



/**
 * Process an interation of a DO OVER loop.
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 *
 * @return true if we should perform this iteration of the loop,
 *         false otherwise.
 */
bool DoBlock::checkOver(RexxActivation *context, ExpressionStack *stack)
{
    // the array was stored in the too field
    ArrayClass *overArray = (ArrayClass *)to;
    // are we past the end of the array?
    if (overArray->lastIndex() < overIndex)
    {
        return false;                    // time to get out of here.
    }

    // get the next element  from the array. This should be a
    // non-sparse array, but we need to double check anyway.
    RexxObject *result = (RexxObject *)overArray->get(overIndex);
    // use .nil for any empty slots
    if (result == OREF_NULL)
    {
        result = TheNilObject;           /* use .nil instead                  */
    }

    // assign the control variable and trace this result
    control->assign(context, result);
    overIndex++;
    return true;
}


/**
 * Perform control variable checks on a DO/LOOP iteration.
 *
 * @param context   The current execution context.
 * @param stack     The current evaluation stack.
 * @param increment a flag indicating whether to increment the value or
 *                  not.  This will be true on the first loop iteration and
 *                  false on all further iterations.
 *
 * @return True if the loop should continue, false if we've hit
 *         a termination condition.
 */
bool DoBlock::checkControl(RexxActivation *context, ExpressionStack *stack, bool increment)
{
    RexxObject *result = OREF_NULL;

    // if this is time to increment the value, perform the plus operation
    // to add in the BY increment.
    if (increment)
    {
        // get the control variable value and trace
        result = control->evaluate(context, stack);
        // increment using the plus operator
        result = result->callOperatorMethod(OPERATOR_PLUS, by);

        // the control variable gets set immediately, and and the assignment will also get traced
        // increment result
        control->assign(context, result);
    }
    else
    {
        // get the control variable value without tracing. We've
        // already traced the initial assignment as part of the setup. This
        // prevents getting an extra add looking item traced.
        result = control->getValue(context);
    }


    // if we have a termination condition, do the compare now by calling the operator method.
    if (to != OREF_NULL)
    {
        if (result->callOperatorMethod(compare, to) == TheTrueObject)
        {
            return false;                  // time to stop if this is true
        }
    }

    // do we have a forCount?  perform that test now
    if (forCount != SIZE_MAX)
    {
        return checkFor();
    }

    return true;                         // still looping
}
