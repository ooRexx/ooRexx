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
/* REXX Translator                                   AddressInstruction.cpp   */
/*                                                                            */
/* Primitive Address Parse Class                                              */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "RexxActivation.hpp"
#include "AddressInstruction.hpp"
#include "SystemInterpreter.hpp"

RexxInstructionAddress::RexxInstructionAddress(
  RexxObject *_expression,              /* variable address expression       */
  RexxString *_environment,             /* address environment name          */
  RexxObject *_command)                 /* command to issue                  */
/******************************************************************************/
/* Function : Complete address instruction initialization                     */
/******************************************************************************/
{
                                       /* store the instruction state       */
  OrefSet(this, this->expression, _expression);
  OrefSet(this, this->environment, _environment);
  OrefSet(this, this->command, _command);
}

void RexxInstructionAddress::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  memory_mark(this->nextInstruction);  /* must be first one marked          */
  memory_mark(this->expression);
  memory_mark(this->environment);
  memory_mark(this->command);
}

void RexxInstructionAddress::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
                                       /* must be first one marked          */
  memory_mark_general(this->nextInstruction);
  memory_mark_general(this->expression);
  memory_mark_general(this->environment);
  memory_mark_general(this->command);
}

void RexxInstructionAddress::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxInstructionAddress)

  flatten_reference(newThis->nextInstruction, envelope);
  flatten_reference(newThis->expression, envelope);
  flatten_reference(newThis->environment, envelope);
  flatten_reference(newThis->command, envelope);

  cleanUpFlatten
}

void RexxInstructionAddress::execute(
    RexxActivation      *context,      /* current activation context        */
    RexxExpressionStack *stack )       /* evaluation stack                  */
/****************************************************************************/
/* Function:  Execute a REXX LEAVE instruction                              */
/****************************************************************************/
{
    context->traceInstruction(this);     /* trace if necessary                */
                                         /* is this an address toggle?        */
    if (this->environment == OREF_NULL && this->expression == OREF_NULL)
    {
        context->toggleAddress();          /* toggle the address settings       */
        context->pauseInstruction();       /* do debug pause if necessary       */
    }
    /* have a constant address name?     */
    else if (this->environment != OREF_NULL)
    {
        if (this->command != OREF_NULL)
        {  /* actually the command form?        */
           /* get the expression value          */
            RexxObject *result = this->command->evaluate(context, stack);
            RexxString *_command = REQUEST_STRING(result);/* force to string form              */
            context->traceResult(_command);  /* trace if necessary                */
                                             /* validate the address name         */
            SystemInterpreter::validateAddressName(this->environment);
            /* go process the command            */
            context->command(this->environment, _command);
        }
        else
        {                             /* just change the address           */
                                      /* validate the address name         */
            SystemInterpreter::validateAddressName(this->environment);
            /* now perform the switch            */
            context->setAddress(this->environment);
            context->pauseInstruction();     /* do debug pause if necessary       */
        }
    }
    else
    {                               /* we have an ADDRESS VALUE form     */
                                    /* get the expression value          */
        RexxObject *result = this->expression->evaluate(context, stack);
        RexxString *_address = REQUEST_STRING(result); /* force to string form              */
        context->traceResult(_address);    /* trace if necessary                */
        SystemInterpreter::validateAddressName(_address);  /* validate the address name         */
        context->setAddress(_address);     /* just change the address           */
        context->pauseInstruction();       /* do debug pause if necessary       */
    }
}

