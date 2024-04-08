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
/* REXX Kernel                                        ConstantDirective.cpp   */
/*                                                                            */
/* Primitive Translator Abstract Directive Code                               */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "ConstantDirective.hpp"
#include "Clause.hpp"
#include "RexxActivation.hpp"
#include "CPPCode.hpp"

/**
 * Construct a ConstantDirective.
 *
 * @param n      The name of the requires target.
 * @param p      The public name of the requires target.
 * @param clause The source file clause containing the directive.
 */
ConstantDirective::ConstantDirective(ConstantGetterCode *c, RexxInternalObject *e, RexxClause *clause) : RexxDirective(clause, KEYWORD_CONSTANT)
{
    constantCode = c;
    expression = e;
}


/**
 * Normal garbage collecting live mark.
 *
 * @param liveMark The current live object mark.
 */
void ConstantDirective::live(size_t liveMark)
{
    // must be first object marked
    memory_mark(nextInstruction);
    memory_mark(constantCode);
    memory_mark(expression);
}


/**
 * The generalized object marking routine.
 *
 * @param reason The processing faze we're running the mark on.
 */
void ConstantDirective::liveGeneral(MarkReason reason)
{
    // must be first one marked (though normally null)
    memory_mark_general(nextInstruction);
    memory_mark_general(constantCode);
    memory_mark_general(expression);
}


/**
 * Flatten the directive instance.
 *
 * @param envelope The envelope we're flattening into.
 */
void ConstantDirective::flatten(Envelope *envelope)
{
    setUpFlatten(ConstantDirective)

    flattenRef(nextInstruction);
    flattenRef(constantCode);
    flattenRef(expression);

    cleanUpFlatten
}


/**
 * Allocate a new constant directive.
 *
 * @param size   The size of the object.
 *
 * @return The memory for the new object.
 */
void *ConstantDirective::operator new(size_t size)
{
    return new_object(size, T_ConstantDirective);
}



/**
 * Execute a dynamic ::CONSTANT directive as if it is a real
 * instruction.
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 */
void ConstantDirective::execute(RexxActivation *context, ExpressionStack *stack )
{
    context->traceInstruction(this);
    // get the expression value
    RexxObject *result = expression->evaluate(context, stack);
    // trace the result
    context->traceResult(result);
    // complete the method for the constant with the resolved value
    constantCode->setValue(result);
    // do debug pause
    context->pauseInstruction();
}
