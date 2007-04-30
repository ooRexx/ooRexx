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
/* REXX Translator                                              ParseInstruction.c    */
/*                                                                            */
/* Primitive Parse Parse Class                                                */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "QueueClass.hpp"
#include "DirectoryClass.hpp"
#include "RexxActivation.hpp"
#include "RexxActivity.hpp"
#include "ParseInstruction.hpp"
#include "ParseTrigger.hpp"
#include "ParseTarget.hpp"
#include "Token.hpp"

extern RexxActivity *CurrentActivity;  /* current activity                  */

RexxInstructionParse::RexxInstructionParse(
  RexxObject *expression,              /* string expression source          */
  USHORT      string_source,           /* source of the parsed string       */
  UCHAR       flags,                   /* option flags                      */
  LONG        templateCount,           /* count of template items           */
  RexxQueue  *parse_template )         /* parsing template array            */
/******************************************************************************/
/* Function:  Complete parse instruction initialization                       */
/******************************************************************************/
{
                                       /* save the expression               */
  OrefSet(this, this->expression, expression);
  i_flags = flags;                     /* save the expression               */
  parse_string_source = string_source; /* save the source information       */
  this->trigger_count = templateCount; /* save the size                     */
  while (templateCount > 0)            /* loop through the template list    */
                                       /* copying each trigger              */
    OrefSet(this, this->triggers[--templateCount], (RexxTrigger *)parse_template->pop());
}

void RexxInstructionParse::execute(
    RexxActivation      *context,      /* current activation context        */
    RexxExpressionStack *stack )       /* evaluation stack                  */
/****************************************************************************/
/* Function:  Execute a REXX PARSE instruction                              */
/****************************************************************************/
{
  RexxObject       *value;             /* parsed value                      */
  RexxObject      **argList;           /* current argument list             */
  size_t            argCount;          /* the argument list size            */
  RexxTarget        target;            /* created target value              */
  RexxTrigger      *trigger;           /* current trigger                   */
  LONG              size;              /* size of template array            */
  BOOL              multiple;          /* processing an argument list       */
  LONG              i;                 /* loop counter                      */

  context->traceInstruction(this);     /* trace if necessary                */
  multiple = FALSE;                    /* default to no argument list       */
  value = OREF_NULLSTRING;             /* no value yet                      */
  argList = OREF_NULL;                 /* neither is there an argument list */
  argCount = 0;

  switch (parse_string_source) {       /* get data from variaous sources    */

    case SUBKEY_PULL:                  /* PARSE PULL instruction            */
                                       /* read a line from the queue        */
      value = CurrentActivity->pullInput(context);
      stack->push(value);              /* add the value to the stack        */
      break;

    case SUBKEY_LINEIN:                /* PARSE LINEIN instruction          */
                                       /* read a line from the stream       */
      value = CurrentActivity->lineIn(context);
      stack->push(value);              /* add the value to the stack        */
      break;

    case SUBKEY_ARG:                   /* PARSE ARG instruction             */
      multiple = TRUE;                 /* have an argument list             */
      /* get the current argument list     */
      argList = context->getMethodArgumentList();
      argCount = context->getMethodArgumentCount();
      break;

    case SUBKEY_SOURCE:                /* PARSE SOURCE instruction          */
      value = context->sourceString(); /* retrieve the source string        */
      stack->push(value);              /* add the value to the stack        */
      break;

    case SUBKEY_VERSION:               /* PARSE VERSION instruction         */
                                       /* retrieve the version string       */
      value = TheKernel->fastAt(OREF_VERSION);
      if (value == OREF_NULL) {        /* missing for some reason?          */
        value = version_number();      /* reconstruct this                  */
                                       /* save this again                   */
        TheKernel->put(value, OREF_VERSION);
      }
      break;

    case SUBKEY_VAR:                   /* PARSE VAR name instruction        */
                                       /* get the variable value            */
      value = this->expression->evaluate(context, stack);
      stack->push(value);              /* add the value to the stack        */
      break;

    case SUBKEY_VALUE:                 /* PARSE VALUE expr WITH instruction */
                                       /* have an expression?               */
      if (this->expression != OREF_NULL)
                                       /* get the expression value          */
        value = this->expression->evaluate(context, stack);
      else
        value = OREF_NULLSTRING;       /* must have been "parse value with" */
      stack->push(value);              /* add the value to the stack        */
      context->traceResult(value);     /* trace if necessary                */
      break;
  }
                                       /* create the parse target           */
  target.init(value, argList, argCount, i_flags&parse_translate, multiple, context, stack);

  size = this->trigger_count;          /* get the template size             */
  for (i = 0; i < size; i++) {         /* loop through the template list    */
    trigger = this->triggers[i];       /* get the next trigger value        */
    if (trigger == OREF_NULL)          /* end of this template portion?     */
      target.next(context);            /* reset for the next string         */
    else                               /* process this trigger              */
      trigger->parse(context, stack, &target);
  }
  context->pauseInstruction();         /* do debug pause if necessary       */
}

void RexxInstructionParse::live()
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  INT  i;                              /* loop counter                      */
  INT  count;                          /* argument count                    */

  setUpMemoryMark
  memory_mark(this->nextInstruction);  /* must be first one marked          */
  for (i = 0, count = this->trigger_count; i < count; i++)
    memory_mark(this->triggers[i]);
  memory_mark(this->expression);
  cleanUpMemoryMark
}

void RexxInstructionParse::liveGeneral()
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  INT  i;                              /* loop counter                      */
  INT  count;                          /* argument count                    */

  setUpMemoryMarkGeneral
                                       /* must be first one marked          */
  memory_mark_general(this->nextInstruction);
  for (i = 0, count = this->trigger_count; i < count; i++)
    memory_mark_general(this->triggers[i]);
  memory_mark_general(this->expression);
  cleanUpMemoryMarkGeneral
}

void RexxInstructionParse::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  INT  i;                              /* loop counter                      */
  INT  count;                          /* argument count                    */

  setUpFlatten(RexxInstructionParse)

  flatten_reference(newThis->nextInstruction, envelope);
  for (i = 0, count = this->trigger_count; i < count; i++)
    flatten_reference(newThis->triggers[i], envelope);
  flatten_reference(newThis->expression, envelope);
  cleanUpFlatten
}

