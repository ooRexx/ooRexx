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
/* Primitive Operator Parse Class                                             */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "RexxActivation.hpp"
#include "ExpressionOperator.hpp"

// Table for transforming an operator back into its
// string name.  These strings must match the operator subclass
// enum defined in RexxToken (TokenSubclass)
const char *RexxExpressionOperator::operatorNames[] =
{
    "",   // dummy value because the operators start at 1.
    "+",
    "-",
    "*",
    "/",
    "%",
    "//",
    "**",
    "",
    "||",
    " ",
    "=",
    "\\=",
    ">",
    "\\>",
    "<",
    "\\<",
    ">=",
    "<=",
    "==",
    "\\==",
    ">>",
    "\\>>",
    "<<",
    "\\<<",
    ">>=",
    "<<=",
    "<>",
    "><",
    "&",
    "|",
    "&&",
    "\\",
};




/**
 * Create a new Unary operator object.
 *
 * @param size   The size of the C++ object.
 *
 * @return Storage for the object instance.
 */
void *RexxUnaryOperator::operator new(size_t size)
{
    return new_object(size, T_UnaryOperatorTerm);
}

/**
 * Create a new Binary operator object.
 *
 * @param size   The size of the C++ object.
 *
 * @return Storage for the object instance.
 */
void *RexxBinaryOperator::operator new(size_t size)
{
    return new_object(size, T_BinaryOperatorTerm);
}

/**
 * Initialize an expression operator.
 *
 * @param op     The operator subcode.
 * @param left   The left side of the expression.
 * @param right  The right side of the expression (null if this is
 *               not a binarry operator).
 */
RexxExpressionOperator::RexxExpressionOperator(TokenSubclass op, RexxInternalObject *left, RexxInternalObject *right)
{
    oper = op;
    left_term = left;
    right_term = right;
}

/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxExpressionOperator::live(size_t liveMark)
{
    memory_mark(left_term);
    memory_mark(right_term);
}

/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxExpressionOperator::liveGeneral(MarkReason reason)
{
  memory_mark_general(left_term);
  memory_mark_general(right_term);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxExpressionOperator::flatten(Envelope *envelope)
{
   setUpFlatten(RexxExpressionOperator)

   flattenRef(left_term);
   flattenRef(right_term);

   cleanUpFlatten
}


/**
 * Evaluate an operator expression term
 *
 * @param context The current execution context.
 * @param stack   The expression stack from the context.
 *
 * @return The operation result object.
 */
RexxObject *RexxBinaryOperator::evaluate(RexxActivation *context, ExpressionStack *stack )
{
    // evaluate both expression terms
    RexxObject *left = left_term->evaluate(context, stack);
    RexxObject *right = right_term->evaluate(context, stack);
    // and finally evaluate the operation itself.  The left term is
    // the target of the message and determines how this is invoked.
    RexxObject *result = left->callOperatorMethod(oper, right);
    // replace top two stack elements with our result
    stack->operatorResult(result);
    // trace the result if necessary
    context->traceOperator(operatorName(), result);
    return result;
}


/**
 * Evaluate a Unary operation.
 *
 * @param context The current execution context.
 * @param stack   The context evaluation stack.
 *
 * @return The operation result.
 */
RexxObject *RexxUnaryOperator::evaluate(RexxActivation *context, ExpressionStack *stack )
{
    // we only have a single term to evaluate
    RexxObject *term = left_term->evaluate(context, stack);
    // and forward to the operator type
    RexxObject *result = term->callOperatorMethod(oper, OREF_NULL);
    // we only replace one term on the stack.
    stack->prefixResult(result);
    context->tracePrefix(operatorName(), result);
    return result;
}



