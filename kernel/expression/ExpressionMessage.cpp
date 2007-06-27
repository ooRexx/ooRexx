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
/* REXX Translator                                              ExpressionMessage.c      */
/*                                                                            */
/* Primitive Message Instruction Parse Class                                  */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "QueueClass.hpp"
#include "RexxActivation.hpp"
#include "ExpressionMessage.hpp"
#include "Token.hpp"
#include "SourceFile.hpp"

RexxExpressionMessage::RexxExpressionMessage(
    RexxObject *target,                /* message send target               */
    RexxString *name,                  /* message name                      */
    RexxObject *super,                 /* message super class               */
    size_t      argCount,              /* count of arguments                */
    RexxQueue  *arglist,               /* message argument list             */
    int         classId)               /* type of message send              */
/******************************************************************************/
/*  Function:  Create a new message expression object                         */
/******************************************************************************/
{
  ClearObject(this);                   /* start completely clean            */
                                       /* also make sure name is cleared    */
                                       /* name doubles as hash so ClearObjec*/
  this->u_name = OREF_NULL;            /* doesn't clear hash field.         */

  OrefSet(this, this->target, target); /* fill in the target                */
                                       /* the message name                  */
  OrefSet(this, this->u_name, name->upper());
  OrefSet(this, this->super, super);   /* the super class target            */
  if (classId == TOKEN_TILDE)          /* single twiddle form?              */
    this->doubleTilde = FALSE;         /* not a double twiddle form         */
  else
    this->doubleTilde = TRUE;          /* target is return value            */
  /* get the count of arguments        */
  this->argumentCount = (SHORT)argCount;
  while (argCount > 0) {               /* now copy the argument pointers    */
                                       /* in reverse order                  */
    OrefSet(this, this->arguments[--argCount], arglist->pop());
  }
}

RexxObject *RexxExpressionMessage::evaluate(
    RexxActivation      *context,      /* current activation context        */
    RexxExpressionStack *stack )       /* evaluation stack                  */
/******************************************************************************/
/* Function:  Evaluate a message send in an expression                        */
/******************************************************************************/
{
  RexxObject *result;                  /* message expression result         */
  RexxObject *super;                   /* target super class                */
  LONG        argcount;                /* count of arguments                */
  RexxObject *target;                  /* message target                    */
  size_t      i;                       /* loop counter                      */

                                       /* evaluate the target               */
  target = this->target->evaluate(context, stack);
  if (this->super != OREF_NULL) {      /* have a message lookup override?   */

    if (target != context->receiver)   /* sender and receiver different?    */
                                       /* this is an error                  */
      report_exception(Error_Execution_super);
                                       /* get the variable value            */
    super = this->super->evaluate(context, stack);
    stack->toss();                     /* pop the top item                  */
  }
  else
    super = OREF_NULL;                 /* use the default lookup            */

  argcount = this->argumentCount;      /* get the argument count            */
  /* loop through the argument list    */
  for (i = 0; i < (size_t)argcount; i++) {
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
  if (super == OREF_NULL)              /* no super class override?          */
                                       /* issue the fast message            */
    result = stack->send(this->u_name, argcount);
  else
                                       /* evaluate the message w/override   */
    result = stack->send(this->u_name, super, argcount);
  stack->popn(argcount);               /* remove any arguments              */
  if (this->doubleTilde)               /* double twiddle form?              */
    result = target;                   /* get the target element            */
  else
    stack->prefixResult(result);       /* replace top element on stack      */

  if (result == OREF_NULL)             /* in an expression and need a result*/
                                       /* need to raise an exception        */
    report_exception1(Error_No_result_object_message, this->u_name);
                                       /* trace if necessary                */
  context->traceMessage(u_name, result);
  return result;                       /* return the result                 */
}

void RexxExpressionMessage::live()
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  INT  i;                              /* loop counter                      */
  INT  count;                          /* argument count                    */

  setUpMemoryMark
  memory_mark(this->u_name);
  memory_mark(this->target);
  memory_mark(this->super);
  for (i = 0, count = this->argumentCount; i < count; i++)
    memory_mark(this->arguments[i]);
  cleanUpMemoryMark
}

void RexxExpressionMessage::liveGeneral()
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  INT  i;                              /* loop counter                      */
  INT  count;                          /* argument count                    */

  setUpMemoryMarkGeneral
  memory_mark_general(this->u_name);
  memory_mark_general(this->target);
  memory_mark_general(this->super);
  for (i = 0, count = this->argumentCount; i < count; i++)
    memory_mark_general(this->arguments[i]);
  cleanUpMemoryMarkGeneral
}

void RexxExpressionMessage::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  INT  i;                              /* loop counter                      */
  INT  count;                          /* argument count                    */

  setUpFlatten(RexxExpressionMessage)

  flatten_reference(newThis->u_name, envelope);
  flatten_reference(newThis->target, envelope);
  flatten_reference(newThis->super, envelope);
  for (i = 0, count = this->argumentCount; i < count; i++)
    flatten_reference(newThis->arguments[i], envelope);

  cleanUpFlatten
}

void *RexxExpressionMessage::operator new(size_t size,
    LONG  argCount)                    /* count of arguments                */
/******************************************************************************/
/* Function:  Create a new translator object                                  */
/******************************************************************************/
{
  RexxObject *newObject;               /* newly create object               */

                                       /* Get new object                    */
  newObject = new_object(size + (argCount - 1) * sizeof(RexxObject *));
                                       /* Give new object its behaviour     */
  BehaviourSet(newObject, TheMessageSendBehaviour);
  return newObject;
}

void RexxExpressionMessage::assign(
    RexxActivation *context,           /* current activation context        */
    RexxExpressionStack *stack,        /* current evaluation stack          */
    RexxObject     *value )            /* new value to assign               */
/******************************************************************************/
/* Function:  Emulate a variable assignment using a method                    */
/******************************************************************************/
{
    // evaluate the target
    RexxObject *target = this->target->evaluate(context, stack);
    RexxObject *super = OREF_NULL;
    // message override?
    if (this->super != OREF_NULL)
    {
        // in this context, the value needs to be SELF
        if (target != context->receiver)
        {
            report_exception(Error_Execution_super);
        }
        // evaluate the superclass override
        super = this->super->evaluate(context, stack);
        stack->toss();
    }
    // push the assignment value on to the stack as the argument
    stack->push(value);
    // now push the rest of the arguments.  This might be something like a[1,2,3,4] as
    // an assignment term.  The assignment value is the first argument, followed by
    // any other arguments that are part of the encoded message term.
    size_t argcount = (size_t)this->argumentCount;

    RexxObject *result;
    for (size_t i = 0; i < argcount; i++)
    {
        // non-omitted argument?
        if (this->arguments[i] != OREF_NULL)
        {
            // evaluate and potentiall trace
            result = this->arguments[i]->evaluate(context, stack);
            context->traceResult(result);
        }
        else
        {
            // non existant arg....we may still need to trace that
            stack->push(OREF_NULL);
            context->traceResult(OREF_NULLSTRING);
        }
    }

    // now send the message the appropriate way
    if (super == OREF_NULL)
    {
        // normal message send
        result = stack->send(this->u_name, argcount + 1);
    }
    else
    {
        // send with an override
        result = stack->send(this->u_name, super, argcount + 1);
    }
    // remove all arguments
    stack->popn(argcount + 1);
}



/**
 * Convert a message into an assignment message by adding "="
 * to the end of the message name.
 *
 * @param source The current source context.
 */
void RexxExpressionMessage::makeAssignment(RexxSource *source)
{
    // add an equal sign to the name
    u_name = source->commonString(u_name->concat(OREF_EQUAL));
}

