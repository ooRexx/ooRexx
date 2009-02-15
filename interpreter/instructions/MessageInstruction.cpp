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
/* REXX Translator                                                            */
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
#include "ProtectedObject.hpp"

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
    OrefSet(this, this->name, message->messageName);
    /* get the argument count            */
    argumentCount = message->argumentCount;
    /* and pointer to arguments          */
    argument_pointer = (RexxObject **)message->arguments;
    /* copy each argument                */
    for (i = 0; i < argumentCount; i++)
    {
        /* into the message instruction      */
        OrefSet(this, this->arguments[i], argument_pointer[i]);
    }
    if (message->doubleTilde)            /* double twiddle form?              */
    {
        instructionFlags |= message_i_double;   /* turn this on                      */
    }
}

RexxInstructionMessage::RexxInstructionMessage(
    RexxExpressionMessage *message,    /* templace message to process       */
    RexxObject *expression)            /* associated expression             */
/******************************************************************************/
/* Function:  Initialize an assignment message instruction                    */
/******************************************************************************/
{
    RexxObject  **argument_pointer;      /* pointer to message args           */
    size_t i;                            /* loop counter                      */

                                         /* copy the message info             */
    OrefSet(this, this->target, message->target);
    OrefSet(this, this->super, message->super);
    OrefSet(this, this->name, message->messageName);  /* get the name                      */
    /* get the argument count            */
    argumentCount = message->argumentCount + 1;
    /* and the argument pointer          */
    argument_pointer = (RexxObject **)message->arguments;
    /* make the expression the first     */
    OrefSet(this, this->arguments[0], expression);
    /* copy each argument                */
    for (i = 1; i < argumentCount; i++)
    {
        /* into the message instruction      */
        OrefSet(this, this->arguments[i], argument_pointer[i - 1]);
    }
    if (message->doubleTilde)            /* double twiddle form?              */
    {
        instructionFlags |= message_i_double; /* turn this on                      */
    }
}

void RexxInstructionMessage::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
    size_t  i;                           /* loop counter                      */
    size_t  count;                       /* argument count                    */

    memory_mark(this->nextInstruction);  /* must be first one marked          */
    memory_mark(this->name);
    memory_mark(this->target);
    memory_mark(this->super);
    for (i = 0, count = argumentCount; i < count; i++)
    {
        memory_mark(this->arguments[i]);
    }
}

void RexxInstructionMessage::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
    size_t  i;                           /* loop counter                      */
    size_t  count;                       /* argument count                    */

                                         /* must be first one marked          */
    memory_mark_general(this->nextInstruction);
    memory_mark_general(this->name);
    memory_mark_general(this->target);
    memory_mark_general(this->super);
    for (i = 0, count = argumentCount; i < count; i++)
    {
        memory_mark_general(this->arguments[i]);
    }
}

void RexxInstructionMessage::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
    size_t  i;                           /* loop counter                      */
    size_t  count;                       /* argument count                    */

    setUpFlatten(RexxInstructionMessage)

    flatten_reference(newThis->nextInstruction, envelope);
    flatten_reference(newThis->name, envelope);
    flatten_reference(newThis->target, envelope);
    flatten_reference(newThis->super, envelope);
    for (i = 0, count = argumentCount; i < count; i++)
    {
        flatten_reference(newThis->arguments[i], envelope);
    }

    cleanUpFlatten
}

void RexxInstructionMessage::execute (
    RexxActivation      *context,      /* current activation context        */
    RexxExpressionStack *stack )       /* evaluation stack                  */
/****************************************************************************/
/* Function:  Execute a REXX THEN instruction                               */
/****************************************************************************/
{
    ProtectedObject result;              /* message expression result         */
    RexxObject *_super;                  /* target super class                */
    size_t      argcount;                /* count of arguments                */
    RexxObject *_target;                 /* message target                    */
    size_t      i;                       /* loop counter                      */

    context->traceInstruction(this);     /* trace if necessary                */
                                         /* evaluate the target               */
    _target = this->target->evaluate(context, stack);
    if (this->super != OREF_NULL)      /* have a message lookup override?   */
    {
        if (_target != context->getReceiver())  /* sender and receiver different?    */
        {
            /* this is an error                  */
            reportException(Error_Execution_super);
        }
        /* get the variable value            */
        _super = this->super->evaluate(context, stack);
        stack->toss();                     /* pop the top item                  */
    }
    else
    {
        _super = OREF_NULL;                 /* use the default lookup            */
    }

    argcount = argumentCount;            /* get the argument count            */
    for (i = 0; i < argcount; i++)     /* loop through the argument list    */
    {
        /* real argument?                    */
        if (this->arguments[i] != OREF_NULL)
        {
            /* evaluate the expression           */
            result = this->arguments[i]->evaluate(context, stack);
            /* trace if necessary                */
            context->traceIntermediate(result, TRACE_PREFIX_ARGUMENT);
        }
        else
        {
            stack->push(OREF_NULL);          /* push an non-existent argument     */
                                             /* trace if necessary                */
            context->traceIntermediate(OREF_NULLSTRING, TRACE_PREFIX_ARGUMENT);
        }
    }
    if (super == OREF_NULL)              /* no super class override?          */
    {
                                         /* issue the fast message            */
        stack->send(this->name, argcount, result);
    }
    else
    {
        /* evaluate the message w/override   */
        stack->send(this->name, _super, argcount, result);
    }
    stack->popn(argcount);               /* remove any arguments              */
    if (instructionFlags&message_i_double) /* double twiddle form?              */
    {
        result = _target;                  /* get the target element            */
    }
    if ((RexxObject *)result != OREF_NULL)   /* result returned?                  */
    {
        context->traceResult((RexxObject *)result);  /* trace if necessary                */
        /* set the RESULT variable to the    */
        /* message return value              */
        context->setLocalVariable(OREF_RESULT, VARIABLE_RESULT, (RexxObject *)result);
    }
    else                                 /* drop the variable RESULT          */
    {
        context->dropLocalVariable(OREF_RESULT, VARIABLE_RESULT);
    }
    context->pauseInstruction();         /* do debug pause if necessary       */
}

