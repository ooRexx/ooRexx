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
/* Primitive Translator Expression Parsing Dot Variable Reference Class       */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "RexxActivation.hpp"
#include "ExpressionDotVariable.hpp"
#include "SourceFile.hpp"

RexxDotVariable::RexxDotVariable(
    RexxString * variable_name )       /* dExpressionBaseVariableiable name to access       */
/******************************************************************************/
/* Function:  Initialize a DOTVARIABLE retriever item                         */
/******************************************************************************/
{
                                       /* store the name                    */
  OrefSet(this, this->variableName, variable_name);
}

void RexxDotVariable::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  memory_mark(this->variableName);
}

void RexxDotVariable::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  memory_mark_general(this->variableName);
}

void RexxDotVariable::flatten(RexxEnvelope * envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxDotVariable)

  flatten_reference(newThis->variableName, envelope);

  cleanUpFlatten
}

RexxObject * RexxDotVariable::evaluate(
    RexxActivation      *context,      /* current activation context        */
    RexxExpressionStack *stack )       /* evaluation stack                  */
/****************************************************************************/
/* Function:  Evaluate a REXX dot variable                                  */
/****************************************************************************/
{
    /* get this from the source          */
    RexxObject *result = context->resolveDotVariable(this->variableName);
    if (result == OREF_NULL)             /* not there?                        */
    {
        /* try for a REXX defined name       */
        result = context->rexxVariable(this->variableName);
    }
    if (result == OREF_NULL)             /* not there?                        */
    {
        /* add a period to the name          */
        result = this->variableName->concatToCstring(CHAR_PERIOD);
    }
    stack->push(result);                 /* place on the evaluation stack     */
                                         /* trace if necessary                */
    context->traceDotVariable(variableName, result);
    return result;                       /* also return the result            */
}


RexxObject * RexxDotVariable::getValue(
    RexxActivation      *context)
/****************************************************************************/
/* Function:  Evaluate a REXX dot variable                                  */
/****************************************************************************/
{
    /* get this from the source          */
    RexxObject *result = context->resolveDotVariable(this->variableName);
    if (result == OREF_NULL)             /* not there?                        */
    {
        /* try for a REXX defined name       */
        result = context->rexxVariable(this->variableName);
    }
    if (result == OREF_NULL)             /* not there?                        */
    {
        /* add a period to the name          */
        result = this->variableName->concatToCstring(CHAR_PERIOD);
    }
    return result;                       /* also return the result            */
}

void * RexxDotVariable::operator new(size_t size)
/******************************************************************************/
/* Function:  Create a new translator object                                  */
/******************************************************************************/
{
                                       /* Get new object                    */
    return new_object(size, T_DotVariableTerm);
}

