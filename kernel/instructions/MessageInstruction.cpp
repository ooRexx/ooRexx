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
/* REXX Translator                                              MessageInstruction.c      */
/*                                                                            */
/* Primitive Message Instruction Parse Class                                  */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "RexxActivation.hpp"
#include "RexxVariableDictionary.hpp"
#include "MessageInstruction.hpp"
#include "ExpressionMessage.hpp"

RexxInstructionMessage::RexxInstructionMessage(
    RexxExpressionMessage *message)    /* templace message to process       */
/******************************************************************************/
/* Function:  Initialize a message instruction                                */
/******************************************************************************/
{
  RexxObject **argument_pointer;       /* pointer to message args           */
  size_t       i;                      /* loop counter                      */

                                       /* copy the message info             */
  OrefSet(this, this->target, message->target);
  OrefSet(this, this->super, message->super);
                                       /* get the name                      */
  OrefSet(this, this->name, message->u_name);
                                       /* get the argument count            */
  message_argument_count = message->argumentCount;
                                       /* and pointer to arguments          */
  argument_pointer = (RexxObject **)message->arguments;
                                       /* copy each argument                */
  for (i = 0; i < message_argument_count; i++)
                                       /* into the message instruction      */
    OrefSet(this, this->arguments[i], argument_pointer[i]);
  if (message->doubleTilde)            /* double twiddle form?              */
    i_flags |= message_i_double;       /* turn this on                      */
}

RexxInstructionMessage::RexxInstructionMessage(
    RexxExpressionMessage *message,    /* templace message to process       */
    RexxObject *expression)            /* associated expression             */
/******************************************************************************/
/* Function:  Initialize an assignment message instruction                    */
/******************************************************************************/
{
  RexxObject  **argument_pointer;      /* pointer to message args           */
  INT    i;                            /* loop counter                      */

                                       /* copy the message info             */
  OrefSet(this, this->target, message->target);
  OrefSet(this, this->super, message->super);
  OrefSet(this, this->name, message->u_name);     /* get the name                      */
                                       /* get the argument count            */
  message_argument_count = message->argumentCount + 1;
                                       /* and the argument pointer          */
  argument_pointer = (RexxObject **)message->arguments;
                                       /* make the expression the first     */
  OrefSet(this, this->arguments[0], expression);
                                       /* copy each argument                */
  for (i = 1; i < message_argument_count; i++)
                                       /* into the message instruction      */
    OrefSet(this, this->arguments[i], argument_pointer[i - 1]);
  if (message->doubleTilde)            /* double twiddle form?              */
    i_flags |= message_i_double;       /* turn this on                      */
}

void RexxInstructionMessage::live()
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  INT  i;                              /* loop counter                      */
  INT  count;                          /* argument count                    */

  setUpMemoryMark
  memory_mark(this->nextInstruction);  /* must be first one marked          */
  memory_mark(this->name);
  memory_mark(this->target);
  memory_mark(this->super);
  for (i = 0, count = message_argument_count; i < count; i++)
    memory_mark(this->arguments[i]);
  cleanUpMemoryMark
}

void RexxInstructionMessage::liveGeneral()
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  INT  i;                              /* loop counter                      */
  INT  count;                          /* argument count                    */

  setUpMemoryMarkGeneral
                                       /* must be first one marked          */
  memory_mark_general(this->nextInstruction);
  memory_mark_general(this->name);
  memory_mark_general(this->target);
  memory_mark_general(this->super);
  for (i = 0, count = message_argument_count; i < count; i++)
    memory_mark_general(this->arguments[i]);
  cleanUpMemoryMarkGeneral
}

void RexxInstructionMessage::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  INT  i;                              /* loop counter                      */
  INT  count;                          /* argument count                    */

  setUpFlatten(RexxInstructionMessage)

  flatten_reference(newThis->nextInstruction, envelope);
  flatten_reference(newThis->name, envelope);
  flatten_reference(newThis->target, envelope);
  flatten_reference(newThis->super, envelope);
  for (i = 0, count = message_argument_count; i < count; i++)
    flatten_reference(newThis->arguments[i], envelope);

  cleanUpFlatten
}

void RexxInstructionMessage::execute (
    RexxActivation      *context,      /* current activation context        */
    RexxExpressionStack *stack )       /* evaluation stack                  */
/****************************************************************************/
/* Function:  Execute a REXX THEN instruction                               */
/****************************************************************************/
{
  RexxObject *result;                  /* message expression result         */
  RexxObject *_super;                  /* target super class                */
  LONG      argcount;                  /* count of arguments                */
  RexxObject *_target;                 /* message target                    */
  LONG      i;                         /* loop counter                      */

  context->traceInstruction(this);     /* trace if necessary                */
                                       /* evaluate the target               */
  _target = this->target->evaluate(context, stack);
  if (this->super != OREF_NULL) {      /* have a message lookup override?   */
    if (_target != context->receiver)   /* sender and receiver different?    */
                                       /* this is an error                  */
      report_exception(Error_Execution_super);
                                       /* get the variable value            */
    _super = this->super->evaluate(context, stack);
    stack->toss();                     /* pop the top item                  */
  }
  else
    _super = OREF_NULL;                 /* use the default lookup            */

  argcount = message_argument_count;   /* get the argument count            */
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
  if (super == OREF_NULL)              /* no super class override?          */
                                       /* issue the fast message            */
    result = stack->send(this->name, argcount);
  else
                                       /* evaluate the message w/override   */
    result = stack->send(this->name, _super, argcount);
  stack->popn(argcount);               /* remove any arguments              */
  if (i_flags&message_i_double)        /* double twiddle form?              */
    result = _target;                  /* get the target element            */
  if (result != OREF_NULL) {           /* result returned?                  */
    context->traceResult(result);      /* trace if necessary                */
                                       /* set the RESULT variable to the    */
                                       /* message return value              */
    context->setLocalVariable(OREF_RESULT, VARIABLE_RESULT, result);
  }
  else                                 /* drop the variable RESULT          */
    context->dropLocalVariable(OREF_RESULT, VARIABLE_RESULT);
  context->pauseInstruction();         /* do debug pause if necessary       */
}

