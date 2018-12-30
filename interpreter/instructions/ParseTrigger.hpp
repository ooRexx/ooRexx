/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                               ParseTrigger.hpp */
/*                                                                            */
/* Primitive PARSE instruction parsing trigger Class Definitions              */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxInstructionTrigger
#define Included_RexxInstructionTrigger

typedef enum
{
    TRIGGER_NONE,
    TRIGGER_END,
    TRIGGER_PLUS,
    TRIGGER_MINUS,
    TRIGGER_ABSOLUTE,
    TRIGGER_STRING,
    TRIGGER_MIXED,
    TRIGGER_PLUS_LENGTH,
    TRIGGER_MINUS_LENGTH,
} ParseTriggerType;

class RexxTarget;
class RexxVariableBase;

class ParseTrigger : public RexxInternalObject
{
 public:
    void        *operator new(size_t, size_t);
    inline void  operator delete(void *) { }

    ParseTrigger(ParseTriggerType, RexxInternalObject *, size_t, QueueClass *);
    inline ParseTrigger(RESTORETYPE restoreType) { ; };

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope *) override;

    size_t integerTrigger(RexxActivation *context, ExpressionStack *stack);
    RexxString *stringTrigger(RexxActivation *context, ExpressionStack *stack);
    void        parse(RexxActivation *, ExpressionStack *, RexxTarget *);

protected:

    ParseTriggerType  triggerType;       // type of trigger
    RexxInternalObject *value;           // value associated with trigger (can be an expression)
    size_t      variableCount;           // count of variables to assign after applying trigger
    RexxVariableBase *variables[1];      // after applying trigger
};
#endif
