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
/* REXX Translator                                    BuiltinFunctions.h      */
/*                                                                            */
/* Builtin Function Execution Stub macros                                     */
/*                                                                            */
/******************************************************************************/

#ifndef BuiltinFunctions_INCLUDED
#define BuiltinFunctions_INCLUDED

// Builtin functions are called with all arguments on the expression stack.
// Individual arguments are retrieved relative to the starting position using
// the defines set up for each function

/**
 * Marks a running built-in function, so that the argument checks it
 * delegates to the methods that implement it report the function's errors
 * (40.xxx) and not the methods' (93.xxx).  See Activity::inBuiltinFunction().
 */
class BuiltinFunctionScope
{
 public:
    inline BuiltinFunctionScope(Activity *a, const char *name) : activity(a),
        savedName(a->builtinName), savedFrame(a->builtinFrame), savedTarget(a->builtinTarget)
    {
        activity->builtinName = name;
        activity->builtinFrame = activity->activationFrames;
        activity->builtinTarget = 1;
    }

    inline ~BuiltinFunctionScope()
    {
        activity->builtinName = savedName;
        activity->builtinFrame = savedFrame;
        activity->builtinTarget = savedTarget;
    }

    // the argument that becomes the target of the method (POS(needle, haystack)
    // calls haystack~pos(needle)); the others are the method's arguments, in order
    inline void targetIs(size_t position) { activity->builtinTarget = position; }

 protected:
    Activity *activity;
    const char *savedName;
    ActivationFrame *savedFrame;
    size_t savedTarget;
};


// makes sure that we have our minimum or maximum number of arguments.
// (Every built-in function starts with one of these, which also marks it
// as running.)
#define fix_args(x) BuiltinFunctionScope builtinScope(context->getActivity(), #x); stack->expandArgs(argcount, x##_Min, x##_Max, #x)
#define check_args(x) BuiltinFunctionScope builtinScope(context->getActivity(), #x); stack->expandArgs(argcount, x##_Min, x##_Max, #x)

// get an individual argument from the stack, by name.
#define get_arg(x,n) (RexxObject *)stack->peek(argcount - x##_##n)

// get an argument from the stack that is required to be a string value.
#define required_string(x,n) stack->requiredStringArg(argcount - x##_##n)
#define optional_string(x,n) ((argcount >= x##_##n) ? stack->optionalStringArg(argcount - x##_##n) : OREF_NULL)

// get an argument from the stack that is required to be an integer value
#define required_integer(x,n) stack->requiredIntegerArg(argcount - x##_##n, argcount, #x)
#define optional_integer(x,n) ((argcount >= x##_##n) ? stack->optionalIntegerArg(argcount - x##_##n, argcount, #x) : OREF_NULL)

// get an argument from the stack that is required to be a "big" integer argument,
// such as a file position.
#define required_big_integer(x,n) stack->requiredBigIntegerArg(argcount - x##_##n, argcount, #x)
#define optional_big_integer(x,n) ((argcount >= x##_##n) ? stack->optionalBigIntegerArg(argcount - x##_##n, argcount, #x) : OREF_NULL)

// tests for optional arguments
#define optional_argument(x,n) ((argcount >= x##_##n) ? (RexxObject *)stack->peek(argcount - x##_##n) : OREF_NULL )
#define arg_exists(x,n) ((argcount >= x##_##n) ? false : stack->peek(argcount - x##_##n) != OREF_NULL )
#define arg_omitted(x,n) ((argcount < x##_##n) ? true : stack->peek(argcount - x##_##n) == OREF_NULL )

// prototype declaration for a builtin function
#define BUILTIN(x) RexxObject *builtin_function_##x ( RexxActivation * context, size_t argcount, ExpressionStack *stack )

// error reporting tests for integer type arguments
#define positive_integer(n,f,p) if (n <= 0) reportException(Error_Incorrect_call_positive, #f, p, n)
#define nonnegative_integer(n,f,p) if (n < 0) reportException(Error_Incorrect_call_nonnegative, #f, p, n)

#define  ALPHANUM "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"

#define optional_pad(x,n) ((argcount >= x##_##n) ? checkPadArgument(#x, x##_##n, stack->optionalStringArg(argcount - x##_##n)) : OREF_NULL)

// checks if pad is a single character string
inline RexxString *checkPadArgument(const char *pFuncName, size_t position, RexxString *pad)
{
    // pads are typically optional, so accept if not there.
    if (pad == OREF_NULL)
    {
        return OREF_NULL;
    }

    if (pad->getLength() != 1)
    {
        reportException(Error_Incorrect_call_pad, pFuncName, new_integer(position), pad);
    }

    return pad;
}

#endif
