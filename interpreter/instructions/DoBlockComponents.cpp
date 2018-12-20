/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2018 Rexx Language Association. All rights reserved.    */
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
/* Primitive classes for implementing various DO loop types in instructions.  */
/*                                                                            */
/******************************************************************************/
#include "DoBlockComponents.hpp"
#include "DoInstruction.hpp"
#include "DoBlock.hpp"
#include "RexxActivation.hpp"
#include "MethodArguments.hpp"
#include "SupplierClass.hpp"
#include "NumberStringClass.hpp"

/**
 * Set up for execution of a FOR loop.
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 * @param doblock The context doblock useds to store loop state data.
 */
void ForLoop::setup(RexxActivation *context,
        ExpressionStack *stack, DoBlock *doblock, bool forKeyword)
{
    // we might not have anything here, but we need to set
    // a marker in the doblock so we know not to use this
    if (forCount == OREF_NULL)
    {
        // set this to a negative value to indicate not to use this.
        doblock->setFor(SIZE_MAX);
        return;
    }

    // get the expression value.  This must be a whole number, so we need to
    // validate and convert now.
    wholenumber_t count = 0;
    RexxObject *result = forCount->evaluate(context, stack);

    context->traceKeywordResult(GlobalNames::FOR, result);

    // if this is an integer value already and we're at the default digits setting,
    // we should be able to use this directly.
    if (isInteger(result) && context->digits() >= Numerics::DEFAULT_DIGITS)
    {
        // get the value directly and trace
        count = ((RexxInteger *)result)->getValue();
    }
    else
    {
        // first get the string version and and request a string version
        Protected<NumberString> strResult = result->requestString()->numberString();
        // non-numeric value, this is an error
        if (strResult == OREF_NULL)
        {
            reportException(forKeyword ? Error_Invalid_whole_number_for : Error_Invalid_whole_number_repeat, result);
        }
        // force rounding
        Protected<RexxObject> rounded = strResult->callOperatorMethod(OPERATOR_PLUS, OREF_NULL);
        // now convert the rounded value to an integer, if possible
        if (!rounded->requestNumber(count, number_digits()))
        {
            // use original object in the error report.
            reportException(forKeyword ? Error_Invalid_whole_number_for : Error_Invalid_whole_number_repeat, result);
        }
    }

    // This must be a non-negative value.
    if (count < 0)
    {
        reportException(forKeyword ? Error_Invalid_whole_number_for : Error_Invalid_whole_number_repeat, result);
    }

    // set this value in the doblock
    doblock->setFor((size_t)count);
}


/**
 * Set up for execution of a controlled loop.
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 * @param doblock The context doblock useds to store loop state data.
 */
void ControlledLoop::setup( RexxActivation *context,
     ExpressionStack *stack, DoBlock *doblock)
{
    // evaluate the initial expression
    RexxObject *_initial = initial->evaluate(context, stack);

    // force rounding by adding zero to this
    _initial = _initial->callOperatorMethod(OPERATOR_PLUS, OREF_NULL);
    // now process each of the expressions.  the expressions
    // array allows us to process these in the order they were specified on
    // the instruction
    for (size_t i = 0; i < 3 && expressions[i] != 0; i++)
    {
        switch (expressions[i])
        {
            // The TO expression;
            case EXP_TO:
            {
                // get the too value and round...which has the side effect
                // of also validating that this is a valid numeric.
                RexxObject *result = to->evaluate(context, stack);
                context->traceKeywordResult(GlobalNames::TO, result);

                // prefix + is like adding zero
                result = result->callOperatorMethod(OPERATOR_PLUS, OREF_NULL);

                // if the result is a string, see if we can convert this to
                // an integer value.  This is very common in loops, and can
                // save us a lot of processing on each loop iteration.
                RexxObject *temp = result->integerValue(number_digits());
                if (temp != TheNilObject)
                {
                    result = temp;
                }
                // this value gets saved in the doblock as state data we can reuse.
                doblock->setTo(result);
                break;
            }

            // BY expression
            case EXP_BY:
            {
                // get the expression value and round
                RexxObject *result = by->evaluate(context, stack);
                context->traceKeywordResult(GlobalNames::BY, result);
                result = result->callOperatorMethod(OPERATOR_PLUS, OREF_NULL);
                // this gets saved in the doblock
                doblock->setBy(result);

                // now we need to check if this is a negative value so set know how to
                // compare.
                if (result->callOperatorMethod(OPERATOR_LESSTHAN, IntegerZero) == TheTrueObject)
                {
                    // we're counting down, so check less than for termination
                    doblock->setCompare(OPERATOR_LESSTHAN);
                }
                else
                {
                    // counting up...compare greater than
                    doblock->setCompare(OPERATOR_GREATERTHAN);
                }
                break;
            }

            // FOR expression...does a binary count
            // our superclass can handle that one.
            case EXP_FOR:
                ForLoop::setup(context, stack, doblock, true);
                break;
        }
    }
    // if the loop did not specify a BY expression, we use a default increment of 1.
    // and we are counting upward, so use a greater than comparison
    if (by == OREF_NULL)
    {
        doblock->setBy(IntegerOne);
        doblock->setCompare(OPERATOR_GREATERTHAN);
    }
    // also disable the FOR count if that was not specified.
    if (forCount == OREF_NULL)
    {
        // set this to a negative value to indicate not to use this.
        doblock->setFor(SIZE_MAX);
    }

    // set the control variable in the doblock (it will use it on subsequent passes).
    doblock->setControl(control);
    // do the initial assignment
    control->assign(context, _initial);
}


/**
 * Set up for execution of a DO OVER loop.
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 * @param doblock The context doblock useds to store loop state data.
 */
void OverLoop::setup( RexxActivation *context,
     ExpressionStack *stack, DoBlock *doblock)
{
    // evaluate the array target
    RexxObject* result = target->evaluate(context, stack);
    // anchor immediately to protect from GC
    doblock->setTo(result);

    context->traceKeywordResult(GlobalNames::OVER, result);
    // if this is already an array item, request the non-sparse version

    ArrayClass *array;
    if (isArray(result))
    {
        array = ((ArrayClass *)result)->makeArray();
    }
    else
    {
        // some other type of collection, use the less direct means
        // of requesting an array
        array = result->requestArray();
        // raise an error if this did not convert ok, or we got
        // back something other than a real Rexx array.
        if (!isArray(array))
        {
            reportException(Error_Execution_noarray, result);
        }
    }

    // we use the TO field to store the array, and the for
    // counter is our index position.
    doblock->setTo(array);
    doblock->setOverIndex(1);
    // and don't forget the variable (which of course, I DID forget!)
    doblock->setControl(control);
}


/**
 * Evaluate a WHILE condition, return true if the condition
 * should continue and false if the WHILE condition failed.
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 *
 * @return true if the WHILE condition passes, false if it fails.
 */
bool WhileUntilLoop::checkWhile(RexxActivation *context, ExpressionStack *stack )
{
    // evaluate the condition and trace
    RexxObject *result = conditional->evaluate(context, stack);
    context->traceKeywordResult(GlobalNames::WHILE, result);

    // most comparisons return either true or false directly, so we
    // can optimize this check.  WHILE conditions are more likely to
    // evaluate to true, so we'll check that first
    if (result == TheTrueObject)
    {
        return true;
    }
    else if (result == TheFalseObject)
    {
        return false;
    }
    // This is some sort of computed boolean, so we need to do a real
    // validation on this
    return result->truthValue(Error_Logical_value_while);
}


/**
 * Evaluate a UNTIL condition, return true if the condition is
 * true or false if it evaluates false.  The meaning of these is
 * the reverse for the UNTIL.  This is optimized for the UNTIL
 * cases.
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 *
 * @return true if the WHILE condition passes, false if it fails.
 */
bool WhileUntilLoop::checkUntil(RexxActivation *context, ExpressionStack *stack )
{
    // evaluate the condition and trace
    RexxObject *result = conditional->evaluate(context, stack);
    context->traceKeywordResult(GlobalNames::UNTIL, result);

    // most comparisons return either true or false directly, so we
    // can optimize this check.  UNTIL conditions are more likely to
    // evaluate to false, so we'll check that first
    if (result == TheFalseObject)
    {
        return false;
    }
    else if (result == TheTrueObject)
    {
        return true;
    }
    // This is some sort of computed boolean, so we need to do a real
    // validation on this
    return result->truthValue(Error_Logical_value_until);
}


/**
 * Set up for execution of a DO WITH loop.
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 * @param doblock The context doblock useds to store loop state data.
 */
void WithLoop::setup( RexxActivation *context,
     ExpressionStack *stack, DoBlock *doblock)
{
    // evaluate the supplier provider
    RexxObject* result = supplierSource->evaluate(context, stack);

    context->traceKeywordResult(GlobalNames::WITH, result);
    // Now send this expression result the supplier message
    // to get a supplier instance.
    ProtectedObject p;

    SupplierClass *supplier = (SupplierClass *)result->sendMessage(GlobalNames::SUPPLIER, p);

    if (supplier == OREF_NULL || !isOfClassType(Supplier, supplier))
    {
        reportException(Error_Execution_no_supplier, result);
    }

    // Store this instance in the doblock
    doblock->setSupplier(supplier);
}


/**
 * Handle the loop iteration for a WITH loop.
 *
 * @param context The current execution context.
 * @param stack   The evaluation stack.
 * @param doblock The do block context.
 * @param first   The first iteration indicator.
 *
 * @return true if the loop should iterate, false otherwise.
 */
bool WithLoop::checkIteration(RexxActivation *context, ExpressionStack *stack, DoBlock *doblock, bool first)
{
    // get the supplier object from the doblock
    SupplierClass *supplier = (SupplierClass *)doblock->getSupplier();

    // make sure we step to next one on each iteration...this is
    // why the loop variation exists!
    if (!first)
    {
        supplier->loopNext();
    }

    // have we reached the end?  Then stop the loop
    if (!supplier->loopAvailable())
    {
        return false;
    }

    // we must have one of the variable items, but need not have both.
    if (indexVar != OREF_NULL)
    {
        RexxObject *index = supplier->loopIndex();
        // the control variable gets set immediately, and we trace this
        // increment result
        indexVar->assign(context, index);
    }

    // we must have one of the variable items, but need not have both.
    if (itemVar != OREF_NULL)
    {
        RexxObject *item = supplier->loopItem();
        // the control variable gets set immediately, and we trace this
        // increment result
        itemVar->assign(context, item);
    }

    // this is a good iteration
    return true;
}



