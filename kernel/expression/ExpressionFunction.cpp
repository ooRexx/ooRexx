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
/* REXX Translator                                              ExpressionFunction.c     */
/*                                                                            */
/* Primitive Function Invocation Class                                        */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "QueueClass.hpp"
#include "DirectoryClass.hpp"
#include "RexxActivation.hpp"
#include "RexxInstruction.hpp"
#include "ExpressionFunction.hpp"
#include "Token.hpp"
#include "StackClass.hpp"
#include "RexxActivity.hpp"

extern pbuiltin builtin_table[];       /* table of builtin function stubs   */

RexxExpressionFunction::RexxExpressionFunction(
    RexxString *function_name,         /* name of the function              */
    size_t      argCount,              /* count of arguments                */
    RexxQueue  *arglist,               /* function arguments                */
    LONG        builtin_index,         /* index of possible built-in func   */
    BOOL        string )               /* string or symbol invocation       */
/******************************************************************************/
/* Function:  Create a function expression object                             */
/******************************************************************************/
{
  ClearObject(this);                   /* initialize the object             */
                                       /* NOTE: the name oref needs to      */
                                       /* be filled in prior to doing any   */
                                       /* thing that might cause a gc       */
                                       /* set the default target            */
  OrefSet(this, this->u_name, function_name);
  /* save the argument count           */
  this->argument_count = (UCHAR)argCount;
  while (argCount > 0) {               /* now copy the argument pointers    */
                                       /* in reverse order                  */
    OrefSet(this, this->arguments[--argCount], arglist->pop());
  }
                                       /* set the builtin index for later   */
  /* resolution step                   */
  this->builtin_index = (SHORT)builtin_index;

  if (string)                          /* have a string lookup?             */
    this->flags |= function_nointernal;/* do not check for internal routines*/
}

void RexxExpressionFunction::resolve(
    RexxDirectory *labels)             /* current table of labels           */
/******************************************************************************/
/* Function:  Resolve a function target location                              */
/******************************************************************************/
{
                                       /* internal routines allowed?        */
  if (!(this->flags&function_nointernal)) {
    if (labels != OREF_NULL)           /* have a labels table?              */
                                       /* check the label table             */
      OrefSet(this, this->target, (RexxInstruction *)labels->at(this->u_name));
    this->flags |= function_internal;  /* this is an internal call          */
  }
  if (this->target == OREF_NULL) {     /* not found yet?                    */
                                       /* have a builtin function?          */
    if (this->builtin_index != NO_BUILTIN) {
      this->flags |= function_builtin; /* this is a builtin function        */
    }
    else
      this->flags |= function_external;/* have an external routine          */
  }
}

void RexxExpressionFunction::live()
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  INT  i;                              /* loop counter                      */
  INT  count;                          /* argument count                    */

  setUpMemoryMark
  memory_mark(this->u_name);
  memory_mark(this->target);
  for (i = 0, count = this->argument_count; i < count; i++)
    memory_mark(this->arguments[i]);
  cleanUpMemoryMark
}

void RexxExpressionFunction::liveGeneral()
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  INT  i;                              /* loop counter                      */
  INT  count;                          /* argument count                    */

  setUpMemoryMarkGeneral
  memory_mark_general(this->u_name);
  memory_mark_general(this->target);
  for (i = 0, count = this->argument_count; i < count; i++)
    memory_mark_general(this->arguments[i]);
  cleanUpMemoryMarkGeneral
}

void RexxExpressionFunction::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  INT  i;                              /* loop counter                      */
  INT  count;                          /* argument count                    */

  setUpFlatten(RexxExpressionFunction)

  flatten_reference(newThis->u_name, envelope);
  flatten_reference(newThis->target, envelope);
  for (i = 0, count = this->argument_count; i < count; i++)
    flatten_reference(newThis->arguments[i], envelope);

  cleanUpFlatten
}

RexxObject *RexxExpressionFunction::evaluate(
    RexxActivation      *context,      /* current activation context        */
    RexxExpressionStack *stack )       /* evaluation stack                  */
/******************************************************************************/
/* Function:  Execute a REXX function                                         */
/******************************************************************************/
{
  RexxObject *result;                  /* returned result                   */
  INT         argcount;                /* count of arguments                */
  INT         i;                       /* loop counter                      */
  LONG        stacktop;                /* top location on the stack         */

  context->activity->stackSpace();     /* check if enough stack is there    */

  stacktop = stack->location();        /* save the stack top                */

  argcount = this->argument_count;     /* get the argument count            */
  for (i = 0; i < argcount; i++) {     /* loop through the argument list    */
                                       /* real argument?                    */
    if (this->arguments[i] != OREF_NULL) {
                                       /* evaluate the expression           */
      result = this->arguments[i]->evaluate(context, stack);
                                       /* trace if necessary                */
      context->traceIntermediate(result, TRACE_PREFIX_ARGUMENT);
    }
    else {
      stack->push(OREF_NULL);          /* push an non-existent argument     */
                                       /* trace if necessary                */
      context->traceIntermediate(OREF_NULLSTRING, TRACE_PREFIX_ARGUMENT);
    }
  }

                                       /* process various call types        */
  switch (this->flags&function_type_mask) {

    case function_internal:            /* need to process internal routine  */
                                       /* go process the internal call      */
      result = context->internalCall(this->target, argcount, stack);
      break;

    case function_builtin:             /* builtin function call             */
                                       /* call the function                 */
      result = (RexxObject *) (*(builtin_table[this->builtin_index]))(context, argcount, stack);
      break;

    case function_external:            /* need to call externally           */
                                       /* go process the internal call      */
      result = context->externalCall(this->u_name, argcount, stack, OREF_FUNCTIONNAME);
      break;
  }
  if (result == OREF_NULL)             /* result returned?                  */
                                       /* raise an error                    */
    if (this->u_name)
      report_exception1(Error_Function_no_data_function, this->u_name);
    else
      report_exception(Error_Function_no_data);  // no name => don't try to print one out...!
  stack->setTop(stacktop);             /* remove arguments from the stack   */
  stack->push(result);                 /* push onto the stack               */
  if ((this->flags&function_type_mask) != function_builtin) discard(result);
                                       /* trace if necessary                */
  context->traceFunction(u_name, result);
  return result;                       /* and return this to the caller     */
}

void *RexxExpressionFunction::operator new(size_t size,
    LONG  argCount)                    /* count of arguments                */
/******************************************************************************/
/* Function:  Create a new translator object                                  */
/******************************************************************************/
{
  RexxObject *newObject;               /* newly create object               */

                                       /* Get new object                    */
  newObject = new_object(size + (argCount - 1) * sizeof(RexxObject *));
                                       /* Give new object its behaviour     */
  BehaviourSet(newObject, TheFunctionBehaviour);
  return newObject;                    /* and return the function           */
}
