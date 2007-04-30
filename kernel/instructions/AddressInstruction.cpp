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
/* REXX Translator                                              AddressInstruction.c     */
/*                                                                            */
/* Primitive Address Parse Class                                              */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "RexxActivation.hpp"
#include "AddressInstruction.hpp"

RexxInstructionAddress::RexxInstructionAddress(
  RexxObject *expression,              /* variable address expression       */
  RexxString *environment,             /* address environment name          */
  RexxObject *command)                 /* command to issue                  */
/******************************************************************************/
/* Function : Complete address instruction initialization                     */
/******************************************************************************/
{
                                       /* store the instruction state       */
  OrefSet(this, this->expression, expression);
  OrefSet(this, this->environment, environment);
  OrefSet(this, this->command, command);
}

void RexxInstructionAddress::live()
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  setUpMemoryMark
  memory_mark(this->nextInstruction);  /* must be first one marked          */
  memory_mark(this->expression);
  memory_mark(this->environment);
  memory_mark(this->command);
  cleanUpMemoryMark
}

void RexxInstructionAddress::liveGeneral()
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  setUpMemoryMarkGeneral
                                       /* must be first one marked          */
  memory_mark_general(this->nextInstruction);
  memory_mark_general(this->expression);
  memory_mark_general(this->environment);
  memory_mark_general(this->command);
  cleanUpMemoryMarkGeneral
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
  RexxObject *result;                  /* expression evaluation result      */
  RexxString *command;                 /* command to be issued              */

  context->traceInstruction(this);     /* trace if necessary                */
                                       /* is this an address toggle?        */
  if (this->environment == OREF_NULL && this->expression == OREF_NULL) {
    context->toggleAddress();          /* toggle the address settings       */
    context->pauseInstruction();       /* do debug pause if necessary       */
  }
                                       /* have a constant address name?     */
  else if (this->environment != OREF_NULL) {
    if (this->command != OREF_NULL) {  /* actually the command form?        */
                                       /* get the expression value          */
      result = this->command->evaluate(context, stack);
      command = REQUEST_STRING(result);/* force to string form              */
      context->traceResult(command);   /* trace if necessary                */
                                       /* validate the address name         */
      SysValidateAddressName(this->environment);
                                       /* go process the command            */
      context->command(command, this->environment);
    }
    else {                             /* just change the address           */
                                       /* validate the address name         */
      SysValidateAddressName(this->environment);
                                       /* now perform the switch            */
      context->setAddress(this->environment);
      context->pauseInstruction();     /* do debug pause if necessary       */
    }
  }
  else {                               /* we have an ADDRESS VALUE form     */
                                       /* get the expression value          */
    result = this->expression->evaluate(context, stack);
    command = REQUEST_STRING(result);  /* force to string form              */
    context->traceResult(command);     /* trace if necessary                */
    SysValidateAddressName(command);   /* validate the address name         */
    context->setAddress(command);      /* just change the address           */
    context->pauseInstruction();       /* do debug pause if necessary       */
  }
}

