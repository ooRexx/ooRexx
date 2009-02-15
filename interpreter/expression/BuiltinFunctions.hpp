/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
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
/* REXX Translator                                    BuiltinFunctions.h      */
/*                                                                            */
/* Builtin Function Execution Stub macros                                     */
/*                                                                            */
/******************************************************************************/

#ifndef OTPBIF_INCLUDED
#define OTPBIF_INCLUDED

#define fix_args(x) stack->expandArgs(argcount, x##_MIN, x##_MAX, CHAR_##x)
#define check_args(x) stack->expandArgs(argcount, x##_MIN, x##_MAX, CHAR_##x)

#define get_arg(x,n) stack->peek(argcount - x##_##n)

#define required_string(x,n) stack->requiredStringArg(argcount - x##_##n)
#define optional_string(x,n) ((argcount >= x##_##n) ? stack->optionalStringArg(argcount - x##_##n) : OREF_NULL)

#define required_integer(x,n) stack->requiredIntegerArg(argcount - x##_##n, argcount, CHAR_##x)
#define optional_integer(x,n) ((argcount >= x##_##n) ? stack->optionalIntegerArg(argcount - x##_##n, argcount, CHAR_##x) : OREF_NULL)

#define optional_argument(x,n) ((argcount >= x##_##n) ? stack->peek(argcount - x##_##n) : OREF_NULL )
#define arg_exists(x,n) ((argcount >= x##_##n) ? false : stack->peek(argcount - x##_##n) != OREF_NULL )
#define arg_omitted(x,n) ((argcount < x##_##n) ? true : stack->peek(argcount - x##_##n) == OREF_NULL )

#define BUILTIN(x) RexxObject *builtin_function_##x ( RexxActivation * context, size_t argcount, RexxExpressionStack *stack )

#define positive_integer(n,f,p) if (n <= 0) reportException(Error_Incorrect_call_positive, CHAR_##f, p, n)
#define nonnegative_integer(n,f,p) if (n < 0) reportException(Error_Incorrect_call_nonnegative, CHAR_##f, p, n)

#define  ALPHANUM "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
#endif
