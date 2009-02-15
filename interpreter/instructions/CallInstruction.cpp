/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
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
/* Primitive Call Parse Class                                                 */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "DirectoryClass.hpp"
#include "ArrayClass.hpp"
#include "RexxVariableDictionary.hpp"
#include "RexxActivation.hpp"
#include "RexxActivity.hpp"
#include "CallInstruction.hpp"
#include "SourceFile.hpp"
#include "ProtectedObject.hpp"

RexxInstructionCall::RexxInstructionCall(
    RexxObject *_name,                 /* CALL name                         */
    RexxString *_condition,            /* CALL ON/OFF condition             */
    size_t      argCount,              /* count of arguments                */
    RexxQueue  *argList,               /* call arguments                    */
    size_t      flags,                 /* CALL flags                        */
    size_t      builtin_index)         /* builtin routine index             */
/******************************************************************************/
/* Function:  Complete CALL instruction object                                */
/******************************************************************************/
{
                                       /* set the name                      */
  OrefSet(this, this->name, (RexxString *)_name);
                                       /* and the condition                 */
  OrefSet(this, this->condition, _condition);
  instructionFlags = (uint16_t)flags;  /* copy the flags                    */
  builtinIndex = (uint16_t)builtin_index; /* and the builtin function index    */
                                       /* no arguments                      */
  argumentCount = (uint16_t)argCount;
  while (argCount > 0) {               /* now copy the argument pointers    */
                                       /* in reverse order                  */
    OrefSet(this, this->arguments[--argCount], argList->pop());
  }
}

void RexxInstructionCall::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  size_t i;                            /* loop counter                      */
  size_t count;                        /* argument count                    */

  memory_mark(this->nextInstruction);  /* must be first one marked          */
  memory_mark(this->name);
  memory_mark(this->target);
  memory_mark(this->condition);
  for (i = 0, count = argumentCount; i < count; i++)
  {
      memory_mark(this->arguments[i]);
  }
}

void RexxInstructionCall::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  size_t i;                            /* loop counter                      */
  size_t count;                        /* argument count                    */

                                       /* must be first one marked          */
  memory_mark_general(this->nextInstruction);
  memory_mark_general(this->name);
  memory_mark_general(this->target);
  memory_mark_general(this->condition);
  for (i = 0, count = argumentCount; i < count; i++)
  {
      memory_mark_general(this->arguments[i]);
  }
}

void RexxInstructionCall::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  size_t i;                            /* loop counter                      */
  size_t count;                        /* argument count                    */

  setUpFlatten(RexxInstructionCall)

  flatten_reference(newThis->nextInstruction, envelope);
  flatten_reference(newThis->name, envelope);
  flatten_reference(newThis->target, envelope);
  flatten_reference(newThis->condition, envelope);
  for (i = 0, count = argumentCount; i < count; i++)
    flatten_reference(newThis->arguments[i], envelope);

  cleanUpFlatten
}

void RexxInstructionCall::resolve(
    RexxDirectory *labels)             /* table of program labels           */
/******************************************************************************/
/* Function:  Resolve a CALL instruction target                               */
/******************************************************************************/
{
  if (this->name == OREF_NULL)         /* not a name target form?           */
    return;                            /* just return                       */
  if (instructionFlags&call_dynamic)   {        // can't resolve now
      return;                          //
  }
  if (!(instructionFlags&call_nointernal)) {    /* internal routines allowed?        */
    if (labels != OREF_NULL)           /* have a labels table?              */
                                       /* check the label table             */
      OrefSet(this, this->target, (RexxInstruction *)labels->at((RexxString *)this->name));
    instructionFlags |= call_internal;          /* this is an internal call          */
  }
  if (this->target == OREF_NULL) {     /* not found yet?                    */
                                       /* have a builtin function?          */
    if (builtinIndex != NO_BUILTIN) {
      instructionFlags |= call_builtin;         /* this is a builtin function        */
                                       /* cast off the routine name         */
      OrefSet(this, this->name, OREF_NULL);
    }
    else
      instructionFlags |= call_external;        /* have an external routine          */
  }
}

void RexxInstructionCall::execute(
    RexxActivation      *context,      /* current activation context        */
    RexxExpressionStack *stack)        /* evaluation stack                  */
/******************************************************************************/
/* Function:  Execute a REXX CALL instruction                                 */
/******************************************************************************/
{
    size_t  argcount;                    /* count of arguments                */
    size_t  i;                           /* loop counter                      */
    int     type;                        /* type of call                      */
    size_t  builtin_index;               /* builtin function index            */
    ProtectedObject   result;            /* returned result                   */
    RexxInstruction  *_target;            /* resolved call target              */
    RexxString       *_name;              /* resolved function name            */
    RexxDirectory    *labels;            /* labels table                      */

    ActivityManager::currentActivity->checkStackSpace();       /* have enough stack space?          */
    context->traceInstruction(this);     /* trace if necessary                */
    if (this->condition != OREF_NULL)  /* is this the ON/OFF form?          */
    {
        if (instructionFlags&call_on_off)           /* ON form?                          */
        {
            /* turn on the trap                  */
            context->trapOn(this->condition, (RexxInstructionCallBase *)this);
        }
        else
        {
            /* turn off the trap                 */
            context->trapOff(this->condition);
        }
    }
    else                               /* normal form of CALL               */
    {
        if (instructionFlags&call_dynamic)        /* dynamic form of call?             */
        {
            /* evaluate the variable             */
            result = this->name->evaluate(context, stack);
            stack->toss();                   /* toss the top item                 */
            _name = REQUEST_STRING(result);   /* force to string form              */
            context->traceResult(name);      /* trace if necessary                */
                                             /* resolve potential builtins        */
            builtin_index = RexxSource::resolveBuiltin(_name);
            _target = OREF_NULL;              /* clear out the target              */
            labels = context->getLabels();   /* get the labels table              */
            if (labels != OREF_NULL)         /* have labels in the program?       */
            {
                                             /* look up label and go to normal    */
                                             /* signal processing                 */
                _target = (RexxInstruction *)(labels->at(_name));
            }
            if (_target != OREF_NULL)        /* found one?                        */
            {
                type = call_internal;          /* have an internal call             */
            }
                                               /* have a builtin by this name?      */
            else if (builtin_index != NO_BUILTIN)
            {
                type = call_builtin;           /* set for a builtin                 */
            }
            else                             /* must be external                  */
            {
                type = call_external;          /* set as so                         */
            }
        }
        else                             /* set up for a normal call          */
        {
            _target = this->target;           /* copy the target                   */
            _name = (RexxString *)this->name; /* the name value                    */
            /* and the builtin index             */
            builtin_index = builtinIndex;
            type = instructionFlags&call_type_mask;   /* just copy the type info           */
        }

        argcount = argumentCount;          /* get the argument count            */
        for (i = 0; i < argcount; i++)   /* loop through the argument list    */
        {
            /* real argument?                    */
            if (this->arguments[i] != OREF_NULL)
            {
                /* evaluate the expression           */
                RexxObject *argResult = this->arguments[i]->evaluate(context, stack);

                /* trace if necessary                */
                context->traceIntermediate(argResult, TRACE_PREFIX_ARGUMENT);
            }
            else
            {
                stack->push(OREF_NULL);        /* push an non-existent argument     */
                                               /* trace if necessary                */
                context->traceIntermediate(OREF_NULLSTRING, TRACE_PREFIX_ARGUMENT);
            }
        }
        switch (type)                    /* process various call types        */
        {

            case call_internal:              /* need to process internal routine  */
                /* go process the internal call      */
                context->internalCall(_target, argcount, stack, result);
                break;

            case call_builtin:               /* builtin function call             */
                /* call the function                 */
                result = (*(RexxSource::builtinTable[builtin_index]))(context, argcount, stack);
                break;

            case call_external:              /* need to call externally           */
                /* go process the external call      */
                context->externalCall(_name, argcount, stack, OREF_ROUTINENAME, result);
                break;
        }
        if ((RexxObject *)result != OREF_NULL)   /* result returned?                  */
        {
            /* set the RESULT variable to the    */
            /* message return value              */
            context->setLocalVariable(OREF_RESULT, VARIABLE_RESULT, (RexxObject *)result);
            context->traceResult((RexxObject *)result);  /* trace if necessary                */
        }
        else                               /* drop the variable RESULT          */
        {
            context->dropLocalVariable(OREF_RESULT, VARIABLE_RESULT);
        }
    }
    context->pauseInstruction();         /* do debug pause if necessary       */
}

void RexxInstructionCall::trap(
    RexxActivation *context,           /* current execution context         */
    RexxDirectory  *conditionObj)      /* associated condition object       */
/******************************************************************************/
/* Function:  Process a CALL ON trap                                          */
/******************************************************************************/
{
    ProtectedObject result;
    context->trapDelay(this->condition); /* put trap into delay state         */

    switch (instructionFlags&call_type_mask)    /* process various call types        */
    {

        case call_internal:                /* need to process internal routine  */
            /* go process the internal call      */
            context->internalCallTrap(this->target, conditionObj, result);
            break;

        case call_builtin:                 /* builtin function call             */
            /* call the function                 */
            (*(RexxSource::builtinTable[builtinIndex]))(context, 0, context->getStack());
            break;

        case call_external:                /* need to call externally           */
            /* go process the externnl call      */
            context->externalCall((RexxString *)this->name, 0, context->getStack(), OREF_ROUTINENAME, result);
            break;
    }
    /* restore the trap state            */
    context->trapUndelay(this->condition);
}

