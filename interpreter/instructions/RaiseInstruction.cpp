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
/* Primitive Raise Parse Class                                                */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "DirectoryClass.hpp"
#include "ArrayClass.hpp"
#include "QueueClass.hpp"
#include "RexxActivation.hpp"
#include "RaiseInstruction.hpp"
#include "Interpreter.hpp"

RexxInstructionRaise::RexxInstructionRaise(
  RexxString *_condition,               /* condition to raise                */
  RexxObject *_expression,              /* expressionial value               */
  RexxObject *_description,             /* description expression            */
  RexxObject *_additional,              /* additional expression             */
  RexxObject *_result,                  /* returned result                   */
  size_t      _arrayCount,             /* size of the array items           */
  RexxQueue  *array,                   /* array argument information        */
  bool        raiseReturn )            /* return/exit flag                  */
/******************************************************************************/
/* Function:  Initialize a RAISE instruction item                             */
/******************************************************************************/
{
    /* save the static information       */
    OrefSet(this, this->condition, _condition);
    OrefSet(this, this->expression, _expression);
    OrefSet(this, this->description, _description);
    OrefSet(this, this->result, _result);
    if (_arrayCount != (size_t)-1)     /* array form?                       */
    {
        instructionFlags |= raise_array;   /* set the array form                */
        /* get the array size                */
        arrayCount = _arrayCount;
        while (_arrayCount > 0)            /* loop through the expression list  */
        {
                                           /* copying each expression           */
            OrefSet(this, this->additional[--_arrayCount], array->pop());
        }
    }
    else                               /* just the one item                 */
    {
        OrefSet(this, this->additional[0], _additional);
        arrayCount = 1;                    /* just the one item                 */
    }
    if (raiseReturn)                     /* return form?                      */
    {
        instructionFlags |= raise_return;  /* turn on the return flag           */
    }
}

void RexxInstructionRaise::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
    size_t  count;                       /* count of array expressions        */
    size_t    i;                         /* loop counter                      */

    memory_mark(this->nextInstruction);  /* must be first one marked          */
    memory_mark(this->condition);
    memory_mark(this->expression);
    memory_mark(this->description);
    memory_mark(this->result);
    for (i = 0, count = arrayCount; i < count; i++)
    {
        memory_mark(this->additional[i]);
    }
}

void RexxInstructionRaise::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
    size_t  count;                       /* count of array expressions        */
    size_t    i;                         /* loop counter                      */

                                         /* must be first one marked          */
    memory_mark_general(this->nextInstruction);
    memory_mark_general(this->condition);
    memory_mark_general(this->expression);
    memory_mark_general(this->description);
    memory_mark_general(this->result);
    for (i = 0, count = arrayCount; i < count; i++)
    {
        memory_mark_general(this->additional[i]);
    }
}

void RexxInstructionRaise::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
    size_t  count;                       /* count of array expressions        */
    size_t    i;                         /* loop counter                      */

    setUpFlatten(RexxInstructionRaise)

    flatten_reference(newThis->nextInstruction, envelope);
    flatten_reference(newThis->condition, envelope);
    flatten_reference(newThis->expression, envelope);
    flatten_reference(newThis->description, envelope);
    flatten_reference(newThis->result, envelope);
    for (i = 0, count = arrayCount; i < count; i++)
    {
        flatten_reference(this->additional[i], envelope);
    }

    cleanUpFlatten
}

void RexxInstructionRaise::execute(
    RexxActivation      *context,      /* current activation context        */
    RexxExpressionStack *stack)        /* evaluation stack                  */
/******************************************************************************/
/* Function:  Execute a REXX RAISE instruction                                */
/******************************************************************************/
{
    RexxObject    *_result;              /* evaluated expression              */
    RexxObject    *rc;                   /* RC variable information           */
    RexxString    *errorcode;            /* converted error code              */
    RexxString    *_description;         /* condition description             */
    RexxObject    *_additional;          /* additional state information      */
    RexxDirectory *conditionobj;         /* propagated condition object       */
    size_t  count;                       /* count of array expressions        */
    size_t  i;                           /* loop counter                      */
    wholenumber_t  msgNum;               /* message number                    */

    context->traceInstruction(this);     /* trace if necessary                */
    _additional = OREF_NULL;             /* no object yet                     */
    _description = OREF_NULL;            /* no description                    */
    rc = OREF_NULL;                      /* no extra information              */
    _result = OREF_NULL;                 /* no result information             */

    if (this->expression != OREF_NULL)   /* extra information for RC?         */
    {
                                         /* get the expression value          */
        rc = this->expression->evaluate(context, stack);
    }
    /* need to validate the RC value?    */
    if (this->condition->strCompare(CHAR_SYNTAX))
    {
        _additional = TheNullArray->copy(); /* change default additional info    */
        /* and the default description       */
        _description = OREF_NULLSTRING;
        errorcode = REQUEST_STRING(rc);    /* get the string version            */
        if (errorcode == TheNilObject)     /* didn't convert?                   */
        {
                                           /* raise an error                    */
            reportException(Error_Conversion_raise, rc);
        }
        /* convert to a decimal              */
        /* and get integer object            */
        msgNum = Interpreter::messageNumber(errorcode);
        rc = (RexxObject *)new_integer(msgNum);
    }
    if (this->description != OREF_NULL)  /* given a description?              */
    {
                                         /* get the expression value          */
        _description = (RexxString *)this->description->evaluate(context, stack);
    }
    if (instructionFlags&raise_array)  /* array form of additional?         */
    {
        count = arrayCount;                /* get the array size                */
        _additional = new_array(count);    /* get a result array                */
        stack->push(_additional);          /* and protect it from collection    */
        for (i = 0; i < count; i++)      /* loop through the expression list  */
        {
            /* real argument?                    */
            if (this->additional[i] != OREF_NULL)
            {
                /* evaluate the expression           */
                ((RexxArray *)_additional)->put((this->additional[i])->evaluate(context, stack), i + 1);
            }
        }
    }
    /* extra information with ?          */
    else if (this->additional[0] != OREF_NULL)
    {
        /* get the expression value          */
        _additional = this->additional[0]->evaluate(context, stack);
    }
    if (this->result != OREF_NULL)       /* given a result value?             */
    {
                                         /* get the expression value          */
        _result = this->result->evaluate(context, stack);
    }
    /* set default condition object      */
    conditionobj = (RexxDirectory *)TheNilObject;
    /* propagating an existing condition?*/
    if (this->condition->strCompare(CHAR_PROPAGATE))
    {
        /* get current trapped condition     */
        conditionobj = context->getConditionObj();
        if (conditionobj == OREF_NULL)     /* no current active condition?      */
        {
            reportException(Error_Execution_propagate);
        }
    }
    if (_additional != OREF_NULL)      /* have additional information?      */
    {
        /* propagate condition maybe?        */
        if (this->condition->strCompare(CHAR_PROPAGATE))
        {
            /* get the original condition name   */
            errorcode = (RexxString *)conditionobj->at(OREF_CONDITION);
        }
        else
        {
            errorcode = this->condition;     /* just use the condition name       */
        }
                                             /* description a single item?        */
        if (errorcode->strCompare(CHAR_SYNTAX))
        {
            /* get the array version             */
            _additional = REQUEST_ARRAY(_additional);
            /* not an array item or a multiple   */
            /* dimension one?                    */
            if (_additional == TheNilObject || ((RexxArray *)_additional)->getDimension() != 1)
            {
                /* this is an error                  */
                reportException(Error_Execution_syntax_additional);
            }
        }
    }
    if (instructionFlags&raise_return)   /* is this the exit form?            */
    {
                                         /* let activation handle as return   */
        context->raise(this->condition, rc, _description, _additional, _result, conditionobj);
    }
    else
    {
        /* activation needs to exit          */
        context->raiseExit(this->condition, rc, _description, _additional, _result, conditionobj);
    }
}

